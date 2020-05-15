
#include "stdafx.h"
#include "RhRdkWavesTexture.h"
#include "RhRdkTextureUtilities.h"
#include "RhRcm.h"

using namespace std;

class CRhRdkWavesTexture::Evaluator : public CRhRdkTwoColorEvaluator
{
public:
	Evaluator(const CRhRdkWavesTexture& texture, CEvalFlags ef);

	virtual ~Evaluator()
	{
		SAFE_DELETE_THIS(m_pWaveWidthTE);
	}

	virtual void DeleteThis(void) { delete this; }

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const;

	virtual bool RegisterChildCallback(IChildCallback* pChildCallback, const CRhRdkTexture& texture, void* pvData);

private:
	IRhRdkTextureEvaluator* m_pWaveWidthTE;
	RhRdkWaveType m_waveType;
	double m_dContrast1;
	double m_dContrast2;
	double m_dWaveWidth;
	int m_childIndexWaveWidth;
	bool m_bWaveWidthTextureOn;
};

CRhRdkWavesTexture::Evaluator::Evaluator(const CRhRdkWavesTexture& texture, CEvalFlags ef)
	:
	CRhRdkTwoColorEvaluator(texture, ef)
{
	m_dContrast1 = texture.Contrast1();
	m_dContrast2 = texture.Contrast2();
	m_waveType   = texture.WaveType();
	m_dWaveWidth = texture.WaveWidth();
	m_bWaveWidthTextureOn = texture.WaveWidthTextureOn();

	m_pWaveWidthTE = nullptr;

	const CRhRdkContent* pSource = texture.FindChild(CS_WAVES_WAVE_WIDTH);
	const CRhRdkTexture* pTexture = dynamic_cast<const CRhRdkTexture*>(pSource);
	if (nullptr != pTexture)
	{
		m_pWaveWidthTE = pTexture->NewTextureEvaluator(ef);
	}

	m_childIndexWaveWidth = -1;
}

CRhRdkColor CRhRdkWavesTexture::Evaluator::GetColorSample(const ON_3dPoint& uvwOriginal,
                                                        const ON_3dVector& duvwdx,
                                                        const ON_3dVector& duvwdy, void* pvData) const
{
	ON_3dVector uvw = LocalMappingTransform() * uvwOriginal;

	const ON_3dVector d1 = LocalMappingTransform() * duvwdx;
	const ON_3dVector d2 = LocalMappingTransform() * duvwdx;

	CRhRdkColor colOut1 = OutputColor(1, uvwOriginal, duvwdx, duvwdy, pvData);
	CRhRdkColor colOut2 = OutputColor(2, uvwOriginal, duvwdx, duvwdy, pvData);

	const double parameter = ((m_waveType == RhRdkWaveType::Linear) ? uvw.y : sqrt(uvw.x * uvw.x + uvw.y * uvw.y));

	const double phase = parameter - floor(parameter);

	const double hillParameter = min(phase, 1.0 - phase);

	double waveWidth = m_dWaveWidth;

	if (m_bWaveWidthTextureOn)
	{
		ON_3dPoint ptWaveWidthSource(uvw.x, 0.5 + floor(uvw.y), 0.0);

		if (m_waveType == RhRdkWaveType::Radial)
		{
			const double length = uvw.Length();
			if (length == 0.0)
				ptWaveWidthSource = ON_origin;
			else
				ptWaveWidthSource = uvw / uvw.Length() * (0.5 + floor(length));
		}

		const IChildCallback* pChildCallback = ChildCallback();

		if(nullptr != pChildCallback && -1 != m_childIndexWaveWidth)
		{
			CRhRdkColor colWaveWidth;

			if (pChildCallback->GetChildColor(m_childIndexWaveWidth, pvData, ptWaveWidthSource, duvwdx, duvwdy, colWaveWidth))
				waveWidth = colWaveWidth.Luminance();
		}
		else if (nullptr != m_pWaveWidthTE)
		{
			CRhRdkColor colWaveWidth;
			if (m_pWaveWidthTE->GetColor(ptWaveWidthSource, ON_origin, ON_origin, colWaveWidth))
				waveWidth = colWaveWidth.Luminance();
		}
	}

	const double hillLength = min(1.0 - waveWidth, waveWidth);
	const double hillMiddle = 0.5 * (1.0 - waveWidth);

	double value = 0.0;
	if (hillParameter > hillMiddle - 0.5 * hillLength)
	{
		if (hillParameter < hillMiddle + 0.5 * hillLength)
		{
			value = 0.5 + 0.5 * sin((hillParameter - hillMiddle) / (hillLength) * ON_PI);
		}
		else
			value = 1.0;
	}

	double contrast = m_dContrast1;
	if (phase > 0.5)
		contrast = m_dContrast2;

	if (contrast < 1.0)
	{
		value = 0.5 + 0.5 * (2.0 * value - 1.0) / (1.0 - contrast);
		if (value < 0.0)
			value = 0.0;
		if (value > 1.0)
			value = 1.0;
	}
	else
	{
		if (value >= 0.5)
			value = 1.0;
		if (value < 0.5)
			value = 0.0;
	}

	if (value <= 0.0)
		return colOut1;

	if (value >= 1.0)
		return colOut2;

	colOut1.BlendTo(float(value), colOut2, true);

	return colOut1;
}

bool CRhRdkWavesTexture::Evaluator::RegisterChildCallback(IRhRdkTextureEvaluator::IChildCallback* pChildCallback, const CRhRdkTexture& texture, void* pvData)
{
	if (!CRhRdkTwoColorEvaluator::RegisterChildCallback(pChildCallback, texture, pvData))
		return false;

	const CRhRdkContent* pChild = texture.FindChild(CS_WAVES_WAVE_WIDTH);
	if (nullptr != pChild)
	{
		m_childIndexWaveWidth = pChildCallback->InitializeChild(pChild, pvData);
	}

	return true;
}

CRhRdkWavesTexture::CRhRdkWavesTexture()
	:
	m_dContrast1         (*this, FS_TEX_WAVES_CONTRAST1         , RhLocalizeString( L"Contrast 1", 33807)      , L"Contrast 1"      ),
	m_dContrast2         (*this, FS_TEX_WAVES_CONTRAST2         , RhLocalizeString( L"Contrast 2", 33808)      , L"Contrast 2"      ),
	m_iWaveType          (*this, FS_TEX_WAVES_WAVE_TYPE         , RhLocalizeString( L"Wave Type", 33809)       , L"Wave Type"       ),
	m_dWaveWidth         (*this, FS_TEX_WAVES_WAVE_WIDTH        , RhLocalizeString( L"Width", 33810)           , L"Width"           ),
	m_bWaveWidthTextureOn(*this, FS_TEX_WAVES_WAVE_WIDTH_TEX_ON , RhLocalizeString( L"Width Texture On", 33811), L"Width Texture On")
{
	m_dContrast1 = 1.0;
	m_dContrast2 = 0.5;
	m_dWaveWidth = 0.5;
	m_iWaveType  = (int)RhRdkWaveType::Linear;
	m_bWaveWidthTextureOn = false;

	m_dContrast1.SetLimits(0.0, 1.0);
	m_dContrast2.SetLimits(0.0, 1.0);
	m_dWaveWidth.SetLimits(0.0, 1.0);
}

UUID CRhRdkWavesTexture::TypeId(void) const
{
	return uuidWavesTextureType;
}

ON_wString CRhRdkWavesTexture::TypeName(void) const
{
	return RhLocalizeString( L"Waves Texture", 23580);
}

ON_wString CRhRdkWavesTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Waves Texture", 23581);
}

ON_wString CRhRdkWavesTexture::InternalName(void) const
{
	return L"rdk-waves-texture";
}

bool CRhRdkWavesTexture::IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const
{
	return f.IsKind(CRhRdkContent::Kinds::Texture);
}

double CRhRdkWavesTexture::Contrast1(void) const
{
	return m_dContrast1;
}

double CRhRdkWavesTexture::Contrast2(void) const
{
	return m_dContrast2;
}

double CRhRdkWavesTexture::WaveWidth(void) const
{
	return m_dWaveWidth;
}

bool CRhRdkWavesTexture::WaveWidthTextureOn() const
{
	return m_bWaveWidthTextureOn;
}

void CRhRdkWavesTexture::SetContrast1(double d)
{
	m_dContrast1.SetValue(d);
}

void CRhRdkWavesTexture::SetContrast2(double d)
{
	m_dContrast2.SetValue(d);
}

RhRdkWaveType CRhRdkWavesTexture::WaveType(void) const
{
	return (RhRdkWaveType)(int)m_iWaveType;
}

void CRhRdkWavesTexture::SetWaveType(RhRdkWaveType type)
{
	m_iWaveType.SetValue((int)type);
}

void CRhRdkWavesTexture::SetWaveWidth(double d)
{
	m_dWaveWidth.SetValue(d);
}

void CRhRdkWavesTexture::SetWaveWidthTextureOn(bool b)
{
	m_bWaveWidthTextureOn.SetValue(b);
}

IRhRdkTextureEvaluator* CRhRdkWavesTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new Evaluator(*this, ef);
}

class CRhRdkWavesTextureCSI : public CRhRdkTwoColorTextureBaseCSI
{
public:
	CRhRdkWavesTextureCSI(const CRhRdkContent* pContent, CRhRdkContent::CSIContext context)
		: CRhRdkTwoColorTextureBaseCSI(pContent, context) { }

	virtual bool NextChildSlot(ON_wString& sParamNameOut, ON_wString& sChildSlotNameOut, ON_wString& sDisplayNameOut);
};

CRhRdkContent::CChildSlotIterator* CRhRdkWavesTexture::NewChildSlotIterator(CSIContext context) const
{
	return new CRhRdkWavesTextureCSI(this, context);
}

bool CRhRdkWavesTextureCSI::NextChildSlot(ON_wString& sParamNameOut, ON_wString& sChildSlotNameOut, ON_wString& sDisplayNameOut)
{
	if (CRhRdkTwoColorTextureBaseCSI::NextChildSlot(sParamNameOut, sChildSlotNameOut, sDisplayNameOut))
		return true;

	if (2 == Index())
	{
		sChildSlotNameOut = CS_WAVES_WAVE_WIDTH;
		sDisplayNameOut = RhLocalizeString( L"Spacing", 23583);
		sParamNameOut = m_pContent->ParamNameFromChildSlotName(sChildSlotNameOut.Array());

		IncIndex();

		return true;
	}

	return false;
}
