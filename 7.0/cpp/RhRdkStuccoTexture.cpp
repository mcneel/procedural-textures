
#include "stdafx.h"
#include "RhRdkStuccoTexture.h"
#include "RhRdkNoiseTexture.h"
#include "RhRdkBlendTexture.h"
#include "RhRcmContentFactories.h"
#include "RhRcm.h"

// CRhRdkStuccoTextureEvaluator

class CRhRdkStuccoTextureEvaluator : public CRhRdkTwoColorEvaluator
{
public:
	CRhRdkStuccoTextureEvaluator(const CRhRdkStuccoTexture& texture, CEvalFlags ef);
	virtual ~CRhRdkStuccoTextureEvaluator();

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const;

protected:
	IRhRdkTextureEvaluator* m_pTextureEvaluator;
};

CRhRdkStuccoTextureEvaluator::CRhRdkStuccoTextureEvaluator(const CRhRdkStuccoTexture& texture, CEvalFlags ef)
	:
	CRhRdkTwoColorEvaluator(texture, ef, false)
{
	m_pTextureEvaluator = nullptr;

	CRhRdkContent* pNoiseContent = rdk.ContentFactories().NewContentFromTypeEx(uuidNoiseTextureType, texture.DocumentAssoc());
	CRhRdkNoiseTexture* pNoiseTexture = dynamic_cast<CRhRdkNoiseTexture*>(pNoiseContent);
	if (nullptr == pNoiseTexture)
		return;

	pNoiseTexture->SetNoiseType(CRhRdkNoiseTexture::NoiseType::kPerlin);
	pNoiseTexture->SetOctaveCount(2);
	pNoiseTexture->SetFrequencyMultiplier(1.0 + texture.Thickness());
	pNoiseTexture->SetAmplitudeMultiplier(1.0);
	pNoiseTexture->SetClampMin(texture.Threshold() * 0.6);
	pNoiseTexture->SetClampMax(0.6);
	pNoiseTexture->SetScaleToClamp(true);

	const double dSize = 8.0 / texture.Size();
	const ON_3dVector vecRepeat(dSize, dSize, dSize);
	pNoiseTexture->SetRepeat(vecRepeat);

	CRhRdkContent* pBlendContent = rdk.ContentFactories().NewContentFromTypeEx(uuidBlendTextureType, texture.DocumentAssoc());
	CRhRdkBlendTexture* pBlendTexture = dynamic_cast<CRhRdkBlendTexture*>(pBlendContent);
	if (nullptr == pBlendTexture)
	{
		pNoiseTexture->Uninitialize();
		delete pNoiseTexture;
		return;
	}

	pBlendTexture->SetTextureOn(true);
	pBlendTexture->SetColor1(Color1());
	pBlendTexture->SetTextureOn1(TextureOn1());
	pBlendTexture->SetTextureAmount1(TextureAmount1());
	pBlendTexture->SetColor2(Color2());
	pBlendTexture->SetTextureOn2(TextureOn2());
	pBlendTexture->SetTextureAmount2(TextureAmount2());
	pBlendTexture->SetRepeat(texture.Repeat());
	pBlendTexture->SetOffset(texture.Offset());
	pBlendTexture->SetRotation(texture.Rotation());

	const CRhRdkContent* pColor1 = texture.FindChild(texture.SwapColors() ? CS_TEX_TWO_COLOR_COLOR_2 : CS_TEX_TWO_COLOR_COLOR_1);
	if (nullptr != pColor1)
	{
		const CRhRdkContent* pColor1Copy = pColor1->MakeCopy();
		if (pColor1Copy != nullptr)
		{
			pBlendTexture->SetChild(pColor1Copy, CS_TEX_TWO_COLOR_COLOR_1);
		}
	}

	const CRhRdkContent* pColor2 = texture.FindChild(texture.SwapColors() ? CS_TEX_TWO_COLOR_COLOR_1 : CS_TEX_TWO_COLOR_COLOR_2);
	if (nullptr != pColor2)
	{
		const CRhRdkContent* pColor2Copy = pColor2->MakeCopy();
		if (pColor2Copy != nullptr)
		{
			pBlendTexture->SetChild(pColor2Copy, CS_TEX_TWO_COLOR_COLOR_2);
		}
	}

	pBlendTexture->SetChild(pNoiseTexture, CS_TEX_BLEND_TEXTURE);

	m_pTextureEvaluator = pBlendTexture->NewTextureEvaluator(ef);

	pBlendContent->Uninitialize();
	delete pBlendContent;
}

CRhRdkStuccoTextureEvaluator::~CRhRdkStuccoTextureEvaluator()
{
	SAFE_DELETE_THIS(m_pTextureEvaluator);
}

CRhRdkColor CRhRdkStuccoTextureEvaluator::GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx,
                                                                                const ON_3dVector& duvwdy, void* pvData) const
{
	CRhRdkColor color;

	if (nullptr != m_pTextureEvaluator)
	{
		m_pTextureEvaluator->GetColor(uvw, duvwdx, duvwdy, color);
	}

	return color;
}

// CRhRdkStuccoTexture

CRhRdkStuccoTexture::CRhRdkStuccoTexture()
	:
	m_dThickness(*this, FS_TEX_STUCCO_THICKNESS, RhLocalizeString( L"Thickness", 33766), L"Thickness"),
	m_dThreshold(*this, FS_TEX_STUCCO_THRESHOLD, RhLocalizeString( L"Threshold", 33767), L"Threshold")
{
	m_dThickness = 1.0;
	m_dThreshold = 0.2;

	SetColor1(CRhRdkColor(210, 200, 200, 255));
	SetColor2(CRhRdkColor(170, 160, 160, 255));

	m_dThickness.SetLimits(0.0, 1.0);
	m_dThreshold.SetLimits(0.0, 1.0);
}

bool CRhRdkStuccoTexture::Initialize(void)
{
	if (!__super::Initialize())
		return false;

	new CRhRdkDynamicContentField(*this, FS_TEX_STUCCO_SIZE, RhLocalizeString( L"Size", 33765), L"Size", 1.0, 0.0, CRhRdkVariant());

	return true;
}

double CRhRdkStuccoTexture::Size(void) const
{
	const auto pField = Fields().FindField(FS_TEX_STUCCO_SIZE);
	RHRDK_ASSERT(nullptr != pField);
	if (nullptr == pField)
		return 1.0;

	return double(*pField);
}

void CRhRdkStuccoTexture::SetSize(double d)
{
	const auto pField = Fields().FindField(FS_TEX_STUCCO_SIZE);
	RHRDK_ASSERT(nullptr != pField);
	if (nullptr != pField)
	{
		*pField = d;
	}
}

double CRhRdkStuccoTexture::Thickness(void) const
{
	return m_dThickness;
}

void CRhRdkStuccoTexture::SetThickness(double d)
{
	m_dThickness.SetValue(d);
}

double CRhRdkStuccoTexture::Threshold(void) const
{
	return m_dThreshold;
}

void CRhRdkStuccoTexture::SetThreshold(double d)
{
	m_dThreshold.SetValue(d);
}

UUID CRhRdkStuccoTexture::TypeId(void) const
{
	return uuidStuccoTextureType;
}

ON_wString CRhRdkStuccoTexture::InternalName(void) const
{
	return L"rdk-stucco-texture";
}

ON_wString CRhRdkStuccoTexture::TypeName(void) const
{
	return RhLocalizeString( L"Stucco Texture", 23506);
}

ON_wString CRhRdkStuccoTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Stucco texture", 23507);
}

const wchar_t* CRhRdkStuccoTexture::Category(void) const
{
	return RDK_CAT_PROCEDURAL_3D;
}

IRhRdkTextureEvaluator* CRhRdkStuccoTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new CRhRdkStuccoTextureEvaluator(*this, ef);
}
