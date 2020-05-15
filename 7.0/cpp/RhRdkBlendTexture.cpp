
#include "stdafx.h"
#include "RhRdkBlendTexture.h"
#include "RhRdkTextureUtilities.h"
#include "RhRcm.h"


class CRhRdkBlendTexture::Evaluator : public CRhRdkTwoColorEvaluator
{
public:
	Evaluator(const CRhRdkBlendTexture& texture, CEvalFlags ef);
	virtual ~Evaluator();

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const;

private:
	double m_dBlendFactor;
	IRhRdkTextureEvaluator* m_pBlendTE;
};

CRhRdkBlendTexture::Evaluator::Evaluator(const CRhRdkBlendTexture& texture, CEvalFlags ef)
	:
	CRhRdkTwoColorEvaluator(texture, ef)
{
	m_pBlendTE = nullptr;

	if (texture.TextureOn())
	{
		const CRhRdkTexture* pTexture = dynamic_cast<const CRhRdkTexture*>(texture.FindChild(CS_TEX_BLEND_TEXTURE));
		if (nullptr != pTexture)
		{
			m_pBlendTE = pTexture->NewTextureEvaluator(ef);
		}
	}

	m_dBlendFactor = texture.BlendFactor();
}

CRhRdkBlendTexture::Evaluator::~Evaluator()
{
	SAFE_DELETE_THIS(m_pBlendTE);
}

CRhRdkColor CRhRdkBlendTexture::Evaluator::GetColorSample(const ON_3dPoint& uvwOriginal, const ON_3dVector& duvwdx,
                                                          const ON_3dVector& duvwdy, void* pvData) const
{
	ON_3dVector uvw = LocalMappingTransform() * uvwOriginal;

	const ON_3dVector d1 = LocalMappingTransform() * duvwdx;
	const ON_3dVector d2 = LocalMappingTransform() * duvwdx;

	CRhRdkColor colOut1 = OutputColor(1, uvw, d1, d2, pvData);
	CRhRdkColor colOut2 = OutputColor(2, uvw, d1, d2, pvData);

	float fBlendFactor = (float)m_dBlendFactor;

	if (m_pBlendTE != nullptr)
	{
		CRhRdkColor blendColor;
		m_pBlendTE->GetColor(uvw, duvwdx, duvwdy, blendColor);
		fBlendFactor = blendColor.Luminance();
	}

	colOut1.BlendTo(fBlendFactor, colOut2, true);

	return colOut1;
}

IRhRdkTextureEvaluator* CRhRdkBlendTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new Evaluator(*this, ef);
}

// CRhRdkBlendTexture

CRhRdkBlendTexture::CRhRdkBlendTexture()
	:
	m_blendFactor(*this, FS_TEX_BLEND_FACTOR    , RhLocalizeString( L"Blend Factor", 33715), L"Blend Factor"),
	m_bTextureOn (*this, FS_TEX_BLEND_TEXTURE_ON, RhLocalizeString( L"Texture On", 33716)  , L"Texture On"  )
{
	m_blendFactor = 0.5;
	m_bTextureOn = false;
}

double CRhRdkBlendTexture::BlendFactor(void) const
{
	return m_blendFactor;
}

void CRhRdkBlendTexture::SetBlendFactor(double d)
{
	m_blendFactor.SetValue(d);
}

bool CRhRdkBlendTexture::TextureOn(void) const
{
	return m_bTextureOn;
}

void CRhRdkBlendTexture::SetTextureOn(bool b)
{
	m_bTextureOn.SetValue(b);
}

UUID CRhRdkBlendTexture::RenderEngineId(void) const
{
	return uuidUniversalRenderEngine;
}

UUID CRhRdkBlendTexture::PlugInId(void) const
{
	return rdk.RhinoPlugInUuid();
}

UUID CRhRdkBlendTexture::TypeId(void) const
{
	return uuidBlendTextureType;
}

ON_wString CRhRdkBlendTexture::InternalName(void) const
{
	return L"BlendTexture";
}

ON_wString CRhRdkBlendTexture::TypeName(void) const
{
	return RhLocalizeString( L"Blend Texture", 32474);
}

ON_wString CRhRdkBlendTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Blends between any two textures.", 32475);
}

const wchar_t* CRhRdkBlendTexture::Category(void) const
{
	return RDK_CAT_PROCEDURAL_3D;
}

bool CRhRdkBlendTexture::IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const
{
	return f.IsKind(CRhRdkContent::Kinds::Texture);
}

CRhRdkContent* CRhRdkBlendTexture::MakeCopy(CopyMethods m) const
{
	CRhRdkContent* pCopy = FastMakeCopy(m);

#ifdef _DEBUG
	VerifyCopy(pCopy);
#endif

	return pCopy;
}
