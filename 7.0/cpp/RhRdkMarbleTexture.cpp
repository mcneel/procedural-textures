
#include "stdafx.h"
#include "RhRdkMarbleTexture.h"
#include "RhRdkWavesTexture.h"
#include "RhRdkNoiseTexture.h"
#include "RhRdkPerturbingTexture.h"
#include "RhRcmContentFactories.h"
#include "RhRcm.h"

class CRhRdkMarbleTextureEvaluator : public CRhRdkTwoColorEvaluator
{
public:
	CRhRdkMarbleTextureEvaluator(const CRhRdkMarbleTexture& texture, CEvalFlags ef);
	~CRhRdkMarbleTextureEvaluator();
	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw,
									const ON_3dVector& duvwdx,
									const ON_3dVector& duvwdy, void* pvData) const;
protected:
	IRhRdkTextureEvaluator* m_pTextureEvaluator;
};

CRhRdkMarbleTextureEvaluator::CRhRdkMarbleTextureEvaluator(const CRhRdkMarbleTexture& texture, CEvalFlags ef)
	:
	CRhRdkTwoColorEvaluator(texture, ef, false)
{
	m_pTextureEvaluator = nullptr;

	CRhRdkContent* pNoiseContent = rdk.ContentFactories().NewContentFromTypeEx(uuidNoiseTextureType, texture.DocumentAssoc());
	CRhRdkNoiseTexture* pNoiseTexture = dynamic_cast<CRhRdkNoiseTexture*>(pNoiseContent);
	if (nullptr == pNoiseTexture)
		return;

	pNoiseTexture->SetNoiseType(CRhRdkNoiseTexture::NoiseType::kPerlin);
	pNoiseTexture->SetOctaveCount(5);
	pNoiseTexture->SetSpecSynthType(CRhRdkNoiseTexture::SpecSynthType::kFractalSum);
	pNoiseTexture->SetFrequencyMultiplier(2.17);
	pNoiseTexture->SetAmplitudeMultiplier(0.5);
	pNoiseTexture->SetClampMin(-1.0);
	pNoiseTexture->SetClampMax( 1.0);

	const double dSize4 = 4.0 / texture.Size();
	const ON_3dVector vecRepeat(dSize4, dSize4, dSize4);
	pNoiseTexture->SetRepeat(vecRepeat);

	CRhRdkContent* pWaveContent = rdk.ContentFactories().NewContentFromTypeEx(uuidWavesTextureType, texture.DocumentAssoc());
	CRhRdkWavesTexture* pWaveTexture = dynamic_cast<CRhRdkWavesTexture*>(pWaveContent);
	if (nullptr == pWaveTexture)
	{
		pNoiseContent->Uninitialize();
		delete pNoiseContent;
		return;
	}

	pWaveTexture->SetWaveType(RhRdkWaveType::Linear);
	pWaveTexture->SetWaveWidth(texture.VeinWidth());
	pWaveTexture->SetContrast1(1.0 - texture.Blur());
	pWaveTexture->SetContrast2(1.0 - texture.Blur());
	pWaveTexture->SetWaveWidthTextureOn(false);
	pWaveTexture->SetColor1(Color1());
	pWaveTexture->SetTextureOn1(TextureOn1());
	pWaveTexture->SetTextureAmount1(TextureAmount1());
	pWaveTexture->SetColor2(Color2());
	pWaveTexture->SetTextureOn2(TextureOn2());
	pWaveTexture->SetTextureAmount2(TextureAmount2());
	pWaveTexture->SetRepeat(vecRepeat);

	const CRhRdkContent* pColor1 = texture.FindChild(texture.SwapColors() ? CS_TEX_TWO_COLOR_COLOR_2 : CS_TEX_TWO_COLOR_COLOR_1);
	if (pColor1 != nullptr)
	{
		const CRhRdkContent* pColor1Copy = pColor1->MakeCopy();
		if (pColor1Copy != nullptr)
		{
			pWaveTexture->SetChild(pColor1Copy, CS_TEX_TWO_COLOR_COLOR_1);
		}
	}

	const CRhRdkContent* pColor2 = texture.FindChild(texture.SwapColors() ? CS_TEX_TWO_COLOR_COLOR_1 : CS_TEX_TWO_COLOR_COLOR_2);
	if (pColor2 != nullptr)
	{
		const CRhRdkContent* pColor2Copy = pColor2->MakeCopy();
		if (pColor2Copy != nullptr)
		{
			pWaveTexture->SetChild(pColor2Copy, CS_TEX_TWO_COLOR_COLOR_2);
		}
	}

	CRhRdkContent* pPerturbContent = rdk.ContentFactories().NewContentFromTypeEx(uuidPerturbingTextureType, texture.DocumentAssoc());
	CRhRdkPerturbingTexture* pPerturbTexture = dynamic_cast<CRhRdkPerturbingTexture*>(pPerturbContent);
	if (nullptr == pPerturbTexture)
	{
		pWaveTexture->Uninitialize();
		delete pWaveTexture;
		pNoiseContent->Uninitialize();
		delete pNoiseContent;
		return;
	}

	pPerturbTexture->SetAmount(texture.Noise() * 0.1);
	pPerturbTexture->SetRepeat(texture.Repeat());   
	pPerturbTexture->SetOffset(texture.Offset());
	pPerturbTexture->SetRotation(texture.Rotation());

	pPerturbTexture->SetChild(pWaveTexture,  CS_TEX_PERTURBING_SOURCE_TEXTURE);
	pPerturbTexture->SetChild(pNoiseTexture, CS_TEX_PERTURBING_PERTURB_TEXTURE);

	m_pTextureEvaluator = pPerturbTexture->NewTextureEvaluator(ef);

	pPerturbContent->Uninitialize();
	delete pPerturbContent;
}

CRhRdkMarbleTextureEvaluator::~CRhRdkMarbleTextureEvaluator()
{
	SAFE_DELETE_THIS(m_pTextureEvaluator);
}

CRhRdkColor CRhRdkMarbleTextureEvaluator::GetColorSample(const ON_3dPoint& uvw,
									const ON_3dVector& duvwdx,
									const ON_3dVector& duvwdy, void* pvData) const
{
	CRhRdkColor color;

	if (nullptr != m_pTextureEvaluator)
	{
		m_pTextureEvaluator->GetColor(uvw, duvwdx, duvwdy, color);
	}

	return color;
}

CRhRdkMarbleTexture::CRhRdkMarbleTexture()
	:
	m_dSize      (*this, FS_TEX_MARBLE_SIZE      , RhLocalizeString( L"Size", 33731)      , L"Size"      ),
	m_dVeinWidth (*this, FS_TEX_MARBLE_VEIN_WIDTH, RhLocalizeString( L"Vein Width", 33732), L"Vein Width"),
	m_dBlur      (*this, FS_TEX_MARBLE_BLUR      , RhLocalizeString( L"Blur", 33733)      , L"Blur"      ),
	m_dNoise     (*this, FS_TEX_MARBLE_NOISE     , RhLocalizeString( L"Noise", 33734)     , L"Noise"     )
{
	m_dSize = 1.0;
	m_dVeinWidth = 0.5;
	m_dBlur = 0.0;
	m_dNoise = 5.0;

	m_dSize.SetLimits(0.01, CRhRdkVariant::Null());
	m_dVeinWidth.SetLimits(0.0, 1.0);
	m_dBlur.SetLimits(0.0, 1.0);
	m_dNoise.SetLimits(0.0, 20.0);
}

UUID CRhRdkMarbleTexture::TypeId(void) const
{
	return uuidMarbleTextureType;
}

ON_wString CRhRdkMarbleTexture::TypeName(void) const
{
	return RhLocalizeString( L"Marble Texture", 23330);
}

ON_wString CRhRdkMarbleTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Marble texture", 23331);
}

ON_wString CRhRdkMarbleTexture::InternalName(void) const
{
	return L"rdk-marble-texture";
}

double CRhRdkMarbleTexture::Size(void) const
{
	return m_dSize;
}

double CRhRdkMarbleTexture::VeinWidth(void) const
{
	return m_dVeinWidth;
}

void CRhRdkMarbleTexture::SetSize(double d)
{
	m_dSize.SetValue(d);
}

void CRhRdkMarbleTexture::SetVeinWidth(double d)
{
	m_dVeinWidth.SetValue(d);
}

double CRhRdkMarbleTexture::Blur(void) const
{
	return m_dBlur;
}

void CRhRdkMarbleTexture::SetBlur(double d)
{
	m_dBlur.SetValue(d);
}

double CRhRdkMarbleTexture::Noise(void) const
{
	return m_dNoise;
}

void CRhRdkMarbleTexture::SetNoise(double d)
{
	m_dNoise.SetValue(d);
}

unsigned int CRhRdkMarbleTexture::BitFlags(void) const
{
	return __super::BitFlags() & ~bfTextureSummary;
}

IRhRdkTextureEvaluator* CRhRdkMarbleTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new CRhRdkMarbleTextureEvaluator(*this, ef);
}
