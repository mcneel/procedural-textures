
#include "stdafx.h"
#include "RhRdkCheckerTexture.h"
#include "RhRdkTextureUtilities.h"
#include "RhRdkUuids.h"

class CRhRdkCheckerTexture::Evaluator : public CRhRdkTwoColorEvaluator
{
public:
	Evaluator(const CRhRdkCheckerTexture& texture, CEvalFlags ef)
		: m_bRemapTextures(texture.RemapTextures()),
		CRhRdkTwoColorEvaluator(texture, ef)
	{
	}

protected:
	const bool m_bRemapTextures;
};

class CRhRdk3DCheckerTexture::Evaluator : public CRhRdkCheckerTexture::Evaluator
{
public:
	Evaluator(const CRhRdkCheckerTexture& texture, CEvalFlags ef) : CRhRdkCheckerTexture::Evaluator(texture, ef) {}

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const;
};

class CRhRdk2DCheckerTexture::Evaluator : public CRhRdkCheckerTexture::Evaluator
{
public:
	Evaluator(const CRhRdkCheckerTexture& texture, CEvalFlags ef) : CRhRdkCheckerTexture::Evaluator(texture, ef) {}

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const;
};

CRhRdkColor CRhRdk3DCheckerTexture::Evaluator::GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, 
                                                                   const ON_3dVector& duvwdy, void* pvData) const
{
	CRhRdkColor colOut = CRhRdkColor::black;

	static const ON_Xform scale2x(2);

	const ON_3dPoint uvwNew = scale2x * LocalMappingTransform() * uvw;
	const ON_3dVector duvwdxNew = scale2x * LocalMappingTransform() * duvwdx;
	const ON_3dVector duvwdyNew = scale2x * LocalMappingTransform() * duvwdy;

	const float u = (float)(uvwNew.x);
	const float v = (float)(uvwNew.y);
	const float w = (float)(uvwNew.z);

	// Compute cell indices for dots

	const int uCell = Floor2Int(u), vCell = Floor2Int(v), wCell = Floor2Int(w);

	if (LBP_IsOdd(uCell + vCell + wCell))
	{
		colOut = OutputColor(1, m_bRemapTextures ? uvwNew : uvw, duvwdxNew, duvwdyNew, pvData);
	}
	else
	{
		colOut = OutputColor(2, m_bRemapTextures ? uvwNew : uvw, duvwdxNew, duvwdyNew, pvData);
	}

	return colOut;
}

CRhRdkColor CRhRdk2DCheckerTexture::Evaluator::GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, 
                                                                   const ON_3dVector& duvwdy, void* pvData) const
{
	CRhRdkColor colOut = CRhRdkColor::black;

	static const ON_Xform scale2x(2);

	const ON_3dPoint uvwNew = scale2x * LocalMappingTransform() * uvw;
	const ON_3dVector duvwdxNew = scale2x * LocalMappingTransform() * duvwdx;
	const ON_3dVector duvwdyNew = scale2x * LocalMappingTransform() * duvwdy;

	const float u = (float)(uvwNew.x);
	const float v = (float)(uvwNew.y);

	// Compute cell indices for dots

	const int uCell = Floor2Int(u), vCell = Floor2Int(v);

	if (LBP_IsOdd(uCell + vCell))
	{
		colOut = OutputColor(1, m_bRemapTextures ? uvwNew : uvw, duvwdxNew, duvwdyNew, pvData);
	}
	else
	{
		colOut = OutputColor(2, m_bRemapTextures ? uvwNew : uvw, duvwdxNew, duvwdyNew, pvData);
	}

	return colOut;
}

//---------------------

CRhRdkCheckerTexture::CRhRdkCheckerTexture()
	:
	m_bRemapTextures(*this, FS_TEX_CHECKER_REMAP_TEXTURES, RhLocalizeString( L"Remap Textures", 33678), L"Remap Textures")
{
	m_bRemapTextures = true;
}

CRhRdkCheckerTexture::~CRhRdkCheckerTexture()
{
}

UUID CRhRdk3DCheckerTexture::TypeId(void) const { return uuid3DCheckerTextureType; }
UUID CRhRdk2DCheckerTexture::TypeId(void) const { return uuid2DCheckerTextureType; }

ON_wString CRhRdk3DCheckerTexture::TypeName(void) const	{ return RhLocalizeString( L"3D Checker Texture", 33001); }
ON_wString CRhRdk2DCheckerTexture::TypeName(void) const	{ return RhLocalizeString( L"2D Checker Texture", 33002); }

ON_wString CRhRdk3DCheckerTexture::TypeDescription(void) const { return RhLocalizeString( L"A 3-D (UVW) procedural checker pattern.", 33003); }
ON_wString CRhRdk2DCheckerTexture::TypeDescription(void) const { return RhLocalizeString( L"A 2-D (UV) procedural checker pattern.", 33004); }

ON_wString CRhRdk3DCheckerTexture::InternalName(void) const { return L"rcm-basic-checker-texture";	}
ON_wString CRhRdk2DCheckerTexture::InternalName(void) const { return L"rcm-2d-checker-texture";	}

bool CRhRdkCheckerTexture::RemapTextures(void) const
{
	return m_bRemapTextures;
}

void CRhRdkCheckerTexture::SetRemapTextures(bool b)
{
	m_bRemapTextures.SetValue(b);
}

ON_Xform CRhRdkCheckerTexture::LocalMappingTransform(void) const
{
	ON_Xform xform;

	//ALB - attempting to remove the local mapping hack in the checker textures - instead
	//I'm going to evaluate them correctly.
	ON_3dVector vecRepeat = Repeat();//* 2.0; // Never zero.

	//I know this changes behaviour, but this is just plain wrong and the
	//sooner we get rid of it the better.  Texture scaling to model scale should
	//only be done when importing a texture from one model to another.
	/*if (ProjectionMode() == wcs)
	{
		vecRepeat.x = MetersToUnits(vecRepeat.x);
		vecRepeat.y = MetersToUnits(vecRepeat.y);
		vecRepeat.z = MetersToUnits(vecRepeat.z);
	}*/

	ConstructXform(vecRepeat.x,  vecRepeat.y,  vecRepeat.z,
                   Rotation().x, Rotation().y, Rotation().z,
                   Offset().x,   Offset().y,   Offset().z,   xform);

	return xform;
}

IRhRdkTextureEvaluator* CRhRdk3DCheckerTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new CRhRdk3DCheckerTexture::Evaluator(*this, ef);
}

IRhRdkTextureEvaluator* CRhRdk2DCheckerTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new CRhRdk2DCheckerTexture::Evaluator(*this, ef);
}

