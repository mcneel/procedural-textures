
#include "stdafx.h"
#include "RhRdkWoodTexture.h"
#include "RhRdkWavesTexture.h"
#include "RhRdkNoiseTexture.h"
#include "RhRdkPerturbingTexture.h"
#include "RhRcmContentFactories.h"
#include "RhRcm.h"

class CRhRdkWoodTextureEvaluator : public CRhRdkTwoColorEvaluator
{
public:
	CRhRdkWoodTextureEvaluator(const CRhRdkWoodTexture& texture, CEvalFlags ef);
	virtual ~CRhRdkWoodTextureEvaluator();

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const;

protected:
	IRhRdkTextureEvaluator* m_pTextureEvaluator;
};

CRhRdkWoodTextureEvaluator::CRhRdkWoodTextureEvaluator(const CRhRdkWoodTexture& texture, CEvalFlags ef)
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
	pNoiseTexture->SetSpecSynthType(CRhRdkNoiseTexture::SpecSynthType::kFractalSum);
	pNoiseTexture->SetFrequencyMultiplier(2.17);
	pNoiseTexture->SetAmplitudeMultiplier(0.5);
	pNoiseTexture->SetClampMin(-1.0);
	pNoiseTexture->SetClampMax( 1.0);
	pNoiseTexture->SetRepeat(ON_3dVector(1.0, 1.0, texture.AxialNoise()));

	CRhRdkContent* pWaveContent = rdk.ContentFactories().NewContentFromTypeEx(uuidWavesTextureType, texture.DocumentAssoc());
	CRhRdkWavesTexture* pWaveTexture = dynamic_cast<CRhRdkWavesTexture*>(pWaveContent);
	if (nullptr == pWaveTexture)
	{
		pNoiseTexture->Uninitialize();
		delete pNoiseTexture;
		return;
	}

	pWaveTexture->SetWaveType(RhRdkWaveType::Radial);
	pWaveTexture->SetWaveWidth(texture.GrainThickness());
	pWaveTexture->SetContrast1(1.0 - texture.Blur1());
	pWaveTexture->SetContrast2(1.0 - texture.Blur2());
	pWaveTexture->SetWaveWidthTextureOn(false);
	pWaveTexture->SetColor1(Color1());
	pWaveTexture->SetTextureOn1(TextureOn1());
	pWaveTexture->SetTextureAmount1(TextureAmount1());
	pWaveTexture->SetColor2(Color2());
	pWaveTexture->SetTextureOn2(TextureOn2());
	pWaveTexture->SetTextureAmount2(TextureAmount2());

	const CRhRdkContent* pColor1 = texture.FindChild(texture.SwapColors() ? CS_TEX_TWO_COLOR_COLOR_2 : CS_TEX_TWO_COLOR_COLOR_1);
	if (nullptr != pColor1)
	{
		const CRhRdkContent* pColor1Copy = pColor1->MakeCopy();
		if (nullptr != pColor1Copy)
		{
			pWaveTexture->SetChild(pColor1Copy, CS_TEX_TWO_COLOR_COLOR_1);
		}
	}

	const CRhRdkContent* pColor2 = texture.FindChild(texture.SwapColors() ? CS_TEX_TWO_COLOR_COLOR_1 : CS_TEX_TWO_COLOR_COLOR_2);
	if (nullptr != pColor2)
	{
		const CRhRdkContent* pColor2Copy = pColor2->MakeCopy();
		if (nullptr != pColor2Copy)
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

	pPerturbTexture->SetAmount(texture.RadialNoise());
	pPerturbTexture->SetRepeat(texture.Repeat());
	pPerturbTexture->SetOffset(texture.Offset());
	pPerturbTexture->SetRotation(texture.Rotation());

	pPerturbTexture->SetChild(pWaveTexture,  CS_TEX_PERTURBING_SOURCE_TEXTURE);
	pPerturbTexture->SetChild(pNoiseTexture, CS_TEX_PERTURBING_PERTURB_TEXTURE);

	m_pTextureEvaluator = pPerturbTexture->NewTextureEvaluator(ef);

	pPerturbContent->Uninitialize();
	delete pPerturbContent;
}

CRhRdkWoodTextureEvaluator::~CRhRdkWoodTextureEvaluator()
{
	SAFE_DELETE_THIS(m_pTextureEvaluator);
}

CRhRdkColor CRhRdkWoodTextureEvaluator::GetColorSample(const ON_3dPoint& uvw,
                                                       const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const
{
	CRhRdkColor color;

	if (nullptr != m_pTextureEvaluator)
	{
		m_pTextureEvaluator->GetColor(uvw, duvwdx, duvwdy, color);
	}

	return color;
}

CRhRdkWoodTexture::CRhRdkWoodTexture()
	:
	m_grainThickness(*this, FS_TEX_WOOD_GRAIN_THICKNESS, RhLocalizeString( L"Grain Thickness", 33812), L"Grain Thickness"),
	m_radialNoise   (*this, FS_TEX_WOOD_RADIAL_NOISE,    RhLocalizeString( L"Radial Noise", 33813)   , L"Radial Noise"),
	m_axialNoise    (*this, FS_TEX_WOOD_AXIAL_NOISE,     RhLocalizeString( L"Axial Noise", 33814)    , L"Axial Noise"),
	m_blur1         (*this, FS_TEX_WOOD_BLUR_1,          RhLocalizeString( L"Blur 1", 33815)         , L"Blur 1"),
	m_blur2         (*this, FS_TEX_WOOD_BLUR_2,          RhLocalizeString( L"Blur 2", 33816)         , L"Blur 2")
{
	m_grainThickness = 0.3;
	m_radialNoise = 0.3;
	m_axialNoise = 0.5;
	m_blur1 = 0.9;
	m_blur2 = 0.1;

	SetColor1(CRhRdkColor(210, 180, 100, 255));
	SetColor2(CRhRdkColor(100,  50,  20, 255));
	SetRepeat(ON_3dVector(20.0, 20.0, 20.0));
	SetOffset(ON_3dVector(0.5, 0.5, 0.0));

	m_grainThickness.SetLimits(0.0, 1.0);
	m_radialNoise.SetLimits(0.0, 5.0);
	m_axialNoise.SetLimits(0.0, 5.0);
	m_blur1.SetLimits(0.0, 1.0);
	m_blur2.SetLimits(0.0, 1.0);
}

CRhRdkWoodTexture::~CRhRdkWoodTexture()
{
}

double CRhRdkWoodTexture::GrainThickness(void) const
{
	return m_grainThickness;
}

void CRhRdkWoodTexture::SetGrainThickness(double d)
{
	m_grainThickness.SetValue(d);
}

double CRhRdkWoodTexture::RadialNoise(void) const
{
	return m_radialNoise;
}

void CRhRdkWoodTexture::SetRadialNoise(double d)
{
	m_radialNoise.SetValue(d);
}

double CRhRdkWoodTexture::AxialNoise(void) const
{
	return m_axialNoise;
}

void CRhRdkWoodTexture::SetAxialNoise(double d)
{
	m_axialNoise.SetValue(d);
}

double CRhRdkWoodTexture::Blur1(void) const
{
	return m_blur1;
}

void CRhRdkWoodTexture::SetBlur1(double d)
{
	m_blur1.SetValue(d);
}

double CRhRdkWoodTexture::Blur2(void) const
{
	return m_blur2;
}

void CRhRdkWoodTexture::SetBlur2(double d)
{
	m_blur2.SetValue(d);
}

UUID CRhRdkWoodTexture::RenderEngineId(void) const
{
	return uuidUniversalRenderEngine;
}

UUID CRhRdkWoodTexture::PlugInId(void) const
{
	return rdk.RhinoPlugInUuid();
}

UUID CRhRdkWoodTexture::TypeId(void) const
{
	return uuidWoodTextureType;
}

ON_wString CRhRdkWoodTexture::InternalName(void) const
{
	return L"WoodTexture";
}

ON_wString CRhRdkWoodTexture::TypeName(void) const
{
	return RhLocalizeString( L"Wood Texture", 32492);
}

ON_wString CRhRdkWoodTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Wood texture", 32493);
}

const wchar_t* CRhRdkWoodTexture::Category(void) const
{
	return RDK_CAT_PROCEDURAL_3D;
}

bool CRhRdkWoodTexture::IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const
{
	return f.IsKind(CRhRdkContent::Kinds::Texture);
}

IRhRdkTextureEvaluator* CRhRdkWoodTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new CRhRdkWoodTextureEvaluator(*this, ef);
}
