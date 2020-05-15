
#include "stdafx.h"
#include "RhRdkTwoColorTextureBase.h"
#include "RhRdkTextureUtilities.h"
#include "RhRcm.h"

IRhRdkTextureEvaluator::CEvalFlags ClearDisableLocalMappingFlag(const IRhRdkTextureEvaluator::CEvalFlags& ef)
{
	unsigned int flags = ef.Flags();
	flags &= ~IRhRdkTextureEvaluator::CEvalFlags::efDisableLocalMapping;
	return IRhRdkTextureEvaluator::CEvalFlags(flags);
}

class CRhRdkOneColorEvaluatorImpl
{
public:
	CRhRdkOneColorEvaluatorImpl(const CRhRdkOneColorTextureBase& texture, IRhRdkTextureEvaluator::CEvalFlags ef, bool bCreateTexEvals)
		:
		m_pColorEval1(nullptr)
	{
		m_aChildIndices[0] = -1;
		m_aChildIndices[1] = -1;

		m_color1 = texture.Color1();

		m_bTextureOn1 = texture.TextureOn1();
		m_dTextureAmount1 = texture.TextureAmount1();

		const bool bLocalMappingOn = !ef.HasFlag(IRhRdkTextureEvaluator::CEvalFlags::efDisableLocalMapping);
		const bool bFilteringOn    = !ef.HasFlag(IRhRdkTextureEvaluator::CEvalFlags::efDisableFiltering);
		const bool bAdjustmentOn   = !ef.HasFlag(IRhRdkTextureEvaluator::CEvalFlags::efDisableAdjustment);

		m_xform = bLocalMappingOn ? texture.LocalMappingTransform() : ON_Xform(1);

		m_bSuperSample = bFilteringOn ? texture.SuperSample() : false;

		m_bTile = texture.Tile();

		m_bDecal = texture.WrapType() == CRhRdkTexture::WrapTypes::Clamped;

		m_pColorAdjuster = bAdjustmentOn ? texture.NewColorAdjuster() :nullptr;

		if (bCreateTexEvals && texture.TextureOn1() && (0.0 < texture.TextureAmount1()))
		{
			const auto* pTexture = dynamic_cast<const CRhRdkTexture*>(texture.FindChild(CS_TEX_TWO_COLOR_COLOR_1));
			if (nullptr != pTexture)
			{
				// Clear DisableLocalMapping flag because it doesn't make sense
				// to propagate this to child-textures.
				const auto ef2 = ClearDisableLocalMappingFlag(ef);
				m_pColorEval1 = pTexture->NewTextureEvaluator(ef2);
			}
		}
	}

	virtual ~CRhRdkOneColorEvaluatorImpl()
	{
		SAFE_DELETE_THIS(m_pColorEval1);
		SAFE_DELETE_THIS(m_pColorAdjuster);
	}

public:
	CRhRdkColor m_color1;
	bool m_bTextureOn1;
	double m_dTextureAmount1;
	IRhRdkTextureEvaluator* m_pColorEval1;
	CRhRdkTexture::IColorAdjuster* m_pColorAdjuster;
	ON_Xform m_xform;
	int m_aChildIndices[2];
	bool m_bSuperSample;
	bool m_bTile;
	bool m_bDecal;
};

class CRhRdkTwoColorEvaluatorImpl
{
public:
	CRhRdkTwoColorEvaluatorImpl(CRhRdkOneColorEvaluatorImpl* p1, const CRhRdkTwoColorTextureBase& texture, IRhRdkTextureEvaluator::CEvalFlags ef, bool bCreateTexEvals)
		:
		m_pImpl1(p1),
		m_pColorEval2(nullptr)
	{
		const bool bSwap = texture.SwapColors();

		m_pImpl1->m_color1          = bSwap ? texture.Color2() : texture.Color1();
		m_pImpl1->m_bTextureOn1     = bSwap ? texture.TextureOn2() : texture.TextureOn1();
		m_pImpl1->m_dTextureAmount1 = bSwap ? texture.TextureAmount2() : texture.TextureAmount1();

		m_color2          = bSwap ? texture.Color1() : texture.Color2();
		m_bTextureOn2     = bSwap ? texture.TextureOn1() : texture.TextureOn2();
		m_dTextureAmount2 = bSwap ? texture.TextureAmount1() : texture.TextureAmount2();

		if (bCreateTexEvals && texture.TextureOn2() && (0.0 < texture.TextureAmount2()))
		{
			const auto* pTexture = dynamic_cast<const CRhRdkTexture*>(texture.FindChild(CS_TEX_TWO_COLOR_COLOR_2));
			if (nullptr != pTexture)
			{
				// Clear DisableLocalMapping flag because it doesn't make sense to propagate this to child-textures.
				const auto ef2 = ClearDisableLocalMappingFlag(ef);
				m_pColorEval2 = pTexture->NewTextureEvaluator(ef2);
			}
		}

		if (bSwap)
		{
			auto* pTemp = m_pImpl1->m_pColorEval1;
			m_pImpl1->m_pColorEval1 = m_pColorEval2;
			m_pColorEval2 = pTemp;
		}
	}

	virtual ~CRhRdkTwoColorEvaluatorImpl()
	{
		SAFE_DELETE_THIS(m_pColorEval2);
	}

public:
	CRhRdkColor m_color2;
	bool m_bTextureOn2;
	double m_dTextureAmount2;

	IRhRdkTextureEvaluator* m_pColorEval2;
	CRhRdkOneColorEvaluatorImpl* m_pImpl1;
};

CRhRdkOneColorEvaluator::CRhRdkOneColorEvaluator(const CRhRdkOneColorTextureBase& texture, CEvalFlags ef, bool bCreateTexEvals)
	:
	CRhRdkTextureEvaluator(ef)
{
	m_pImpl1 = new CRhRdkOneColorEvaluatorImpl(texture, ef, bCreateTexEvals);
}

CRhRdkTwoColorEvaluator::CRhRdkTwoColorEvaluator(const CRhRdkTwoColorTextureBase& texture, CEvalFlags ef, bool bCreateTexEvals)
	:
	CRhRdkOneColorEvaluator(texture, ef, bCreateTexEvals)
{
	m_pImpl2 = new CRhRdkTwoColorEvaluatorImpl(m_pImpl1, texture, ef, bCreateTexEvals);
}

CRhRdkOneColorEvaluator::~CRhRdkOneColorEvaluator()
{
	SAFE_DELETE(m_pImpl1);
}

CRhRdkTwoColorEvaluator::~CRhRdkTwoColorEvaluator()
{
	SAFE_DELETE(m_pImpl2);
}

const CRhRdkColor& CRhRdkOneColorEvaluator::Color1(void) const
{
	return m_pImpl1->m_color1;
}

bool CRhRdkOneColorEvaluator::TextureOn1(void) const
{
	return m_pImpl1->m_bTextureOn1;
}

double CRhRdkOneColorEvaluator::TextureAmount1(void) const
{
	return m_pImpl1->m_dTextureAmount1;
}

const CRhRdkColor& CRhRdkTwoColorEvaluator::Color2(void) const
{
	return m_pImpl2->m_color2;
}

bool CRhRdkTwoColorEvaluator::TextureOn2(void) const
{
	return m_pImpl2->m_bTextureOn2;
}

double CRhRdkTwoColorEvaluator::TextureAmount2(void) const
{
	return m_pImpl2->m_dTextureAmount2;
}

const ON_Xform& CRhRdkOneColorEvaluator::LocalMappingTransform(void) const
{
	return m_pImpl1->m_xform;
}

static CRhRdkColor OutputColorImpl(const IRhRdkTextureEvaluator* pEval, const CRhRdkColor& colBase, double textureAmount,
                                   const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy)
{
	if (nullptr == pEval)
		return colBase;

	CRhRdkColor colOut;
	pEval->GetColor(uvw, duvwdx, duvwdy, colOut);

	// 0 equals base color and 1 equals texture color.
	const double blendFactor = colOut.FAlpha() * textureAmount;
	if (blendFactor >= 0.999f)
		return colOut;

	if (blendFactor <= 0.001f)
		return colBase;

	colOut.BlendTo(1.0f - (float)blendFactor, colBase);

	return colOut;
}

CRhRdkColor CRhRdkOneColorEvaluator::OutputColor(int colorNumber, const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const
{
	RHRDK_ASSERT(1 == colorNumber);

	const int childIndex = m_pImpl1->m_aChildIndices[0];
	if (childIndex >= 0)
	{
		const auto* pChildCallback = ChildCallback();
		if (nullptr != pChildCallback)
		{
			CRhRdkColor colOut;
			if (pChildCallback->GetChildColor(childIndex, pvData, uvw, duvwdx, duvwdy, colOut))
			{
				colOut.BlendTo((float)(1.0 - m_pImpl1->m_dTextureAmount1), m_pImpl1->m_color1);
				return colOut;
			}
		}
	}

	return OutputColorImpl(m_pImpl1->m_pColorEval1, m_pImpl1->m_color1, m_pImpl1->m_dTextureAmount1, uvw, duvwdx, duvwdy);
}

CRhRdkColor CRhRdkTwoColorEvaluator::OutputColor(int colorNumber, const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const
{
	RHRDK_ASSERT((1 == colorNumber) || (2 == colorNumber));

	const bool bColor1 = (1 == colorNumber);
	const auto& col = bColor1 ? m_pImpl1->m_color1 : m_pImpl2->m_color2;
	const double dTextureAmount = bColor1 ? m_pImpl1->m_dTextureAmount1 : m_pImpl2->m_dTextureAmount2;

	const int childIndex = m_pImpl1->m_aChildIndices[colorNumber-1];
	if (childIndex >= 0)
	{
		const auto* pChildCallback = ChildCallback();
		if (nullptr != pChildCallback)
		{
			CRhRdkColor colOut;
			if (pChildCallback->GetChildColor(childIndex, pvData, uvw, duvwdx, duvwdy, colOut))
			{
				colOut.BlendTo((float)(1.0 - dTextureAmount), col);
				return colOut;
			}
		}
	}

	return OutputColorImpl(bColor1 ? m_pImpl1->m_pColorEval1 : m_pImpl2->m_pColorEval2, col, dTextureAmount, uvw, duvwdx, duvwdy);
}

static const int iSamples = 4;
static const int iRepeats = 3;

bool CRhRdkOneColorEvaluator::GetColor(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData) const
{
	if (m_pImpl1->m_bDecal && !InsideUnitRange2d(LocalMappingTransform() * uvw))
	{
		// This is not an error condition.
		colOut.Set(0.0, 0.0, 0.0, 0.0);
		return true;
	}

	// If tiling is disabled use the old way to get color.
	if (!m_pImpl1->m_bTile)
		return GetColor2(uvw, duvwdx, duvwdy, colOut, pvData);

	// And if tiling is enabled do some magic maths

	// If texture is continuous the tiling result will be continuous as well.

	// Tile texture coordinate to [0,1]x[0,1]x[0,1]
	double dIntegerPart;
	double u = modf(uvw.x, &dIntegerPart);
	double v = modf(uvw.y, &dIntegerPart);
	double w = modf(uvw.z, &dIntegerPart);
	if (u < 0.0)
		u += 1.0;
	if (v < 0.0)
		v += 1.0;
	if (w < 0.0)
		w += 1.0;

	const double dFactorU = u * u * (3.0 - 2.0 * u);
	const double dFactorV = v * v * (3.0 - 2.0 * v);
	const double dFactorW = w * w * (3.0 - 2.0 * w);

	// Create sample points and weights
	ON_3dPoint ptUVW[8];
	double dWeight[8];
	ptUVW[0].Set(u, v, w);
	dWeight[0] = (1.0 - dFactorU) * (1.0 - dFactorV) * (1.0 - dFactorW);
	ptUVW[1].Set(u - 1.0, v, w);
	dWeight[1] = dFactorU * (1.0 - dFactorV) * (1.0 - dFactorW);
	ptUVW[2].Set(u, v - 1.0, w);
	dWeight[2] = (1.0 - dFactorU) * dFactorV * (1.0 - dFactorW);
	ptUVW[3].Set(u - 1.0, v - 1.0, w);
	dWeight[3] = dFactorU * dFactorV * (1.0 - dFactorW);
	ptUVW[4].Set(u, v, w - 1.0);
	dWeight[4] = (1.0 - dFactorU) * (1.0 - dFactorV) * dFactorW;
	ptUVW[5].Set(u - 1.0, v, w - 1.0);
	dWeight[5] = dFactorU * (1.0 - dFactorV) * dFactorW;
	ptUVW[6].Set(u, v - 1.0, w - 1.0);
	dWeight[6] = (1.0 - dFactorU) * dFactorV * dFactorW;
	ptUVW[7].Set(u - 1.0, v - 1.0, w - 1.0);
	dWeight[7] = dFactorU * dFactorV * dFactorW;

	// Sum weighted sample colors
	colOut.Set(0.0, 0.0, 0.0, 0.0);
	for (int i = 0; i < 8; i++)
	{
		CRhRdkColor col;
		if (!GetColor2(ptUVW[i], duvwdx, duvwdy, col, pvData))
			return false;

		colOut += (float)dWeight[i] * col;
	}

	return true;
}

bool CRhRdkOneColorEvaluator::GetColor2(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData) const
{
	const double du = std::max(std::abs(duvwdx.x), std::abs(duvwdy.x));
	const double dv = std::max(std::abs(duvwdx.y), std::abs(duvwdy.y));

	//No derivatives - don't sample
	if (!m_pImpl1->m_bSuperSample || ((du == 0.0) && (dv == 0.0)))
	{
		colOut = GetColorSample(uvw, duvwdx, duvwdy, pvData);

		if (m_pImpl1->m_pColorAdjuster)
		{
			m_pImpl1->m_pColorAdjuster->AdjustColor(colOut);
		}

		return true;
	}

	const double u = uvw.x, v = uvw.y, w = uvw.z;

	CRhRdkColor color(GetColorSample(uvw, duvwdx, duvwdy, pvData));
	int iSamplesTaken = 1;

	ON_3dPoint uvwSample[iSamples];

	CRhRdkColor colorCenter = color;
	bool bVaries = false;

	CRhRdkColor c;
	for (int r = 0; r < iRepeats; r++)
	{
		double n0 = Noise(u+r+1,v);
		double n1 = Noise(u,v+r+1);
		//double n2 = Noise(u+r+1,v,w);

		double du5 = du * std::abs(n0 * 2.0);
		double dv5 = dv * std::abs(n1 * 2.0);
		//double dw5 = dw * std::absf(n0*2);

		double u0 = u - du5, u1 = u + du5;
		double v0 = v - dv5, v1 = v + dv5;
		//double w0 = w - dw5, w1 = w + dw5;

		uvwSample[0].Set(u0,v0,w);
		uvwSample[1].Set(u1,v1,w);
		uvwSample[2].Set(u1,v0,w);
		uvwSample[3].Set(u0,v1,w);

		for (int i = 0; i < iSamples; i++)
		{
			c = GetColorSample(uvwSample[i], duvwdx, duvwdy, pvData);

			color.Add(c); // Use Add() so alpha doesn't get clamped.

			iSamplesTaken++;

			if (colorCenter != c)
				bVaries = true;
		}

		// Get out if there has been no color variance.
		if (!bVaries)
			break;
	}

	const float f = float(iSamplesTaken);
	CRhRdkColor colDiv(f, f, f, f);
	color /= colDiv; // Use color divide so alpha gets divided.

	colOut.Set(color.FRed(), color.FGreen(), color.FBlue(), color.FAlpha());

	if (nullptr != m_pImpl1->m_pColorAdjuster)
	{
		m_pImpl1->m_pColorAdjuster->AdjustColor(colOut);
	}

	return true;
}

bool CRhRdkOneColorEvaluator::RegisterChildCallback(IRhRdkTextureEvaluator::IChildCallback* pChildCallback, const CRhRdkTexture& texture, void* pvData)
{
	if (!CRhRdkTextureEvaluator::RegisterChildCallback(pChildCallback, texture, pvData))
		return false;

	const auto* pChild1 = texture.FindChild(CS_TEX_TWO_COLOR_COLOR_1);
	if (nullptr != pChild1)
	{
		m_pImpl1->m_aChildIndices[0] = pChildCallback->InitializeChild(pChild1, pvData);
	}

	return true;
}

void* CRhRdkOneColorEvaluator::EVF(const wchar_t*, void*)
{
	return nullptr;
}

bool CRhRdkTwoColorEvaluator::RegisterChildCallback(IRhRdkTextureEvaluator::IChildCallback* pChildCallback, const CRhRdkTexture& texture, void* pvData)
{
	if (!CRhRdkOneColorEvaluator::RegisterChildCallback(pChildCallback, texture, pvData))
		return false;

	const auto* pChild2 = texture.FindChild(CS_TEX_TWO_COLOR_COLOR_2);
	if (nullptr != pChild2)
	{
		m_pImpl1->m_aChildIndices[1] = pChildCallback->InitializeChild(pChild2, pvData);
	}

	return true;
}

//---------------------

class CRhRdkOneColorTextureBase::CImpl
{
public:
	CImpl(CRhRdkOneColorTextureBase& octb)
		:
		m_color1        (octb, FS_TEX_TWO_COLOR_COLOR1,          RhLocalizeString( L"Color 1", 22776)     , L"Color 1"     ),
		m_bSuperSample  (octb, FS_TEX_TWO_COLOR_SUPERSAMPLE,     RhLocalizeString( L"Super-sample", 22777), L"Super-sample"),
		m_bTextureOn1   (octb, FS_TEX_TWO_COLOR_TEXTURE_ON1,     RhLocalizeString( L"On 1", 22778)        , L"On 1"        ),
		m_textureAmount1(octb, FS_TEX_TWO_COLOR_TEXTURE_AMOUNT1, RhLocalizeString( L"Amount 1", 22779)    , L"Amount 1"    ),
		m_bTile			(octb, FS_TEX_TWO_COLOR_TILE,            RhLocalizeString( L"Tile", 22780)        , L"Tile"        )
	{
		m_color1 = colBlack;
		m_bSuperSample = false;
		m_bTextureOn1 = true;
		m_textureAmount1 = 1.0;
		m_bTile = false;

		m_textureAmount1.SetLimits(0.0, 1.0);
		m_textureAmount1.SetConversion(CRhRdkContentField::conv_percentile);
	}

public:
	ON_Xform m_xform;
	CRhRdkContentField m_color1;
	CRhRdkContentField m_bSuperSample;
	CRhRdkContentField m_bTextureOn1;
	CRhRdkContentField m_textureAmount1;
	CRhRdkContentField m_bTile;
};

class CRhRdkTwoColorTextureBase::CImpl
{
public:
	CImpl(CRhRdkTwoColorTextureBase& tctb)
		:
		m_color2        (tctb, FS_TEX_TWO_COLOR_COLOR2,          RhLocalizeString( L"Color 2", 22781)    , L"Color 2"    ),
		m_bSwapColors   (tctb, FS_TEX_TWO_COLOR_SWAP_COLORS,     RhLocalizeString( L"Swap Colors", 22782), L"Swap Colors"),
		m_bTextureOn2   (tctb, FS_TEX_TWO_COLOR_TEXTURE_ON2,     RhLocalizeString( L"On 2", 22783)       , L"On 2"       ),
		m_textureAmount2(tctb, FS_TEX_TWO_COLOR_TEXTURE_AMOUNT2, RhLocalizeString( L"Amount 2", 22784)   , L"Amount 2"   )
	{
		m_color2 = colWhite;
		m_bSwapColors = false;
		m_bTextureOn2 = true;
		m_textureAmount2 = 1.0;

		m_textureAmount2.SetLimits(0.0, 1.0);
		m_textureAmount2.SetConversion(CRhRdkContentField::conv_percentile);
	}

public:
	CRhRdkContentField m_color2;
	CRhRdkContentField m_bSwapColors;
	CRhRdkContentField m_bTextureOn2;
	CRhRdkContentField m_textureAmount2;
};

CRhRdkOneColorTextureBase::CRhRdkOneColorTextureBase()
{
	m_pImpl1 = new CImpl(*this);
}

CRhRdkTwoColorTextureBase::CRhRdkTwoColorTextureBase()
{
	m_pImpl2 = new CImpl(*this);
}

CRhRdkOneColorTextureBase::~CRhRdkOneColorTextureBase()
{
	SAFE_DELETE(m_pImpl1);
}

CRhRdkTwoColorTextureBase::~CRhRdkTwoColorTextureBase()
{
	SAFE_DELETE(m_pImpl2);
}

UUID CRhRdkOneColorTextureBase::RenderEngineId(void) const
{
	return uuidUniversalRenderEngine;
}

UUID CRhRdkOneColorTextureBase::PlugInId(void) const
{
	return rdk.RhinoPlugInUuid();
}

/*const wchar_t* CRhRdkOneColorTextureBase::Category(void) const
{
	return RDK_CAT_PROCEDURAL_3D;
}*/

CRhRdkColor CRhRdkOneColorTextureBase::Color1(void) const
{
	return m_pImpl1->m_color1;
}

CRhRdkColor CRhRdkTwoColorTextureBase::Color2(void) const
{
	return m_pImpl2->m_color2;
}

bool CRhRdkTwoColorTextureBase::SwapColors(void) const
{
	return m_pImpl2->m_bSwapColors;
}

bool CRhRdkOneColorTextureBase::SuperSample(void) const
{
	return m_pImpl1->m_bSuperSample;
}

bool CRhRdkOneColorTextureBase::TextureOn1(void) const
{
	return m_pImpl1->m_bTextureOn1;
}

bool CRhRdkTwoColorTextureBase::TextureOn2(void) const
{
	return m_pImpl2->m_bTextureOn2;
}

double CRhRdkOneColorTextureBase::TextureAmount1(void) const
{
	return m_pImpl1->m_textureAmount1;
}

double CRhRdkTwoColorTextureBase::TextureAmount2(void) const
{
	return m_pImpl2->m_textureAmount2;
}

bool CRhRdkOneColorTextureBase::Tile(void) const
{
	return m_pImpl1->m_bTile;
}

void CRhRdkOneColorTextureBase::SetColor1(const CRhRdkColor& col)
{
	m_pImpl1->m_color1.SetValue(col);
}

void CRhRdkTwoColorTextureBase::SetColor2(const CRhRdkColor& col)
{
	m_pImpl2->m_color2.SetValue(col);
}

void CRhRdkTwoColorTextureBase::SetSwapColors(bool b)
{
	m_pImpl2->m_bSwapColors.SetValue(b);
}

void CRhRdkOneColorTextureBase::SetSuperSample(bool b)
{
	m_pImpl1->m_bSuperSample.SetValue(b);
}

void CRhRdkOneColorTextureBase::SetTextureOn1(bool b)
{
	m_pImpl1->m_bTextureOn1.SetValue(b);
}

void CRhRdkTwoColorTextureBase::SetTextureOn2(bool b)
{
	m_pImpl2->m_bTextureOn2.SetValue(b);
}

void CRhRdkOneColorTextureBase::SetTextureAmount1(double d)
{
	m_pImpl1->m_textureAmount1.SetValue(d);
}

void CRhRdkTwoColorTextureBase::SetTextureAmount2(double d)
{
	m_pImpl2->m_textureAmount2.SetValue(d);
}

void CRhRdkOneColorTextureBase::SetTile(bool b)
{
	m_pImpl1->m_bTile.SetValue(b);
}

bool CRhRdkOneColorTextureBase::ReadParametersFromSection(const IRhRdk_XMLSection& section, ReadParamsContext context)
{
	return __super::ReadParametersFromSection(section, context);
}

bool CRhRdkTwoColorTextureBase::ReadParametersFromSection(const IRhRdk_XMLSection& section, ReadParamsContext context)
{
	return __super::ReadParametersFromSection(section, context);
}

bool CRhRdkOneColorTextureBase::WriteParametersToSection(IRhRdk_XMLSection& section, WriteParamsContext context) const
{
	return __super::WriteParametersToSection(section, context);
}

bool CRhRdkTwoColorTextureBase::WriteParametersToSection(IRhRdk_XMLSection& section, WriteParamsContext context) const
{
	return __super::WriteParametersToSection(section, context);
}

unsigned int CRhRdkOneColorTextureBase::ComputeRenderCRC(const CRenderCRCFlags& rcrcFlags) const
{
	return __super::ComputeRenderCRC(rcrcFlags) ^ GetCompileDateMash();
}

bool CRhRdkOneColorTextureBase::SetParameter(const wchar_t* wszName, const CRhRdkVariant& value)
{
	return __super::SetParameter(wszName, value);
}

bool CRhRdkTwoColorTextureBase::SetParameter(const wchar_t* wszName, const CRhRdkVariant& value)
{
	return __super::SetParameter(wszName, value);
}

bool CRhRdkOneColorTextureBase::GetExtraRequirementParameter(const wchar_t* wszParamName, const wchar_t* wszExtraReqName, CRhRdkVariant& vValueOut) const
{
	if (__super::GetExtraRequirementParameter(wszParamName, wszExtraReqName, vValueOut))
		return true;

	vValueOut.SetNull();

	const CLBPString sChildSlotName = (const wchar_t*)ChildSlotNameFromParamName(wszParamName);
	if (sChildSlotName == CS_TEX_TWO_COLOR_COLOR_1)
	{
		const CLBPString sExtraRequirementName = wszExtraReqName;

		if (sExtraRequirementName == RDK_TEXTURE_ON)
		{
			vValueOut = (bool)m_pImpl1->m_bTextureOn1;
		}
		else
		if (sExtraRequirementName == RDK_TEXTURE_AMOUNT)
		{
			vValueOut = (float)m_pImpl1->m_textureAmount1 * 100.0f + 0.01f;
		}
	}

	if (vValueOut.IsNull())
		return false;

	return true;
}

bool CRhRdkOneColorTextureBase::SetExtraRequirementParameter(const wchar_t* wszParamName,
	const wchar_t* wszExtraReqName,
	const CRhRdkVariant& vValue, SetContext sc)
{
	if (__super::SetExtraRequirementParameter(wszParamName, wszExtraReqName, vValue, sc))
		return true;

	const ON_wString sChildSlotName = ChildSlotNameFromParamName(wszParamName);
	if (!sChildSlotName.IsEmpty())
	{
		if (sChildSlotName == CS_TEX_TWO_COLOR_COLOR_1)
		{
			const CLBPString sExtraRequirementName = wszExtraReqName;

			if (sExtraRequirementName == RDK_TEXTURE_ON)
			{
				const bool b = vValue.AsBool();
				if ((bool)m_pImpl1->m_bTextureOn1 != b)
				{
					m_pImpl1->m_bTextureOn1 = b;
					Changed();
				}

				return true;
			}
			else
				if (sExtraRequirementName == RDK_TEXTURE_AMOUNT)
				{
					const double d = vValue.AsDouble() / 100.0;
					if (!LBPIsDoubleEqual(m_pImpl1->m_textureAmount1, d))
					{
						m_pImpl1->m_textureAmount1 = d;
						Changed();
					}

					return true;
				}
		}
	}
	return false;
}

bool CRhRdkTwoColorTextureBase::GetExtraRequirementParameter(const wchar_t* wszParamName, const wchar_t* wszExtraReqName, CRhRdkVariant& vValueOut) const
{
	if (__super::GetExtraRequirementParameter(wszParamName, wszExtraReqName, vValueOut))
		return true;

	vValueOut.SetNull();

	const CLBPString sChildSlotName = (const wchar_t*)ChildSlotNameFromParamName(wszParamName);
	if (sChildSlotName == CS_TEX_TWO_COLOR_COLOR_2)
	{
		const CLBPString sExtraRequirementName = wszExtraReqName;

		if (sExtraRequirementName == RDK_TEXTURE_ON)
		{
			vValueOut = (bool)m_pImpl2->m_bTextureOn2;
		}
		else
		if (sExtraRequirementName == RDK_TEXTURE_AMOUNT)
		{
			vValueOut = (float)m_pImpl2->m_textureAmount2 * 100.0f + 0.01f;
		}
	}

	if (vValueOut.IsNull())
		return false;

	return true;
}

bool CRhRdkTwoColorTextureBase::SetExtraRequirementParameter(const wchar_t* wszParamName,
	const wchar_t* wszExtraReqName,
	const CRhRdkVariant& vValue, SetContext sc)
{
	if (__super::SetExtraRequirementParameter(wszParamName, wszExtraReqName, vValue, sc))
		return true;

	const ON_wString sChildSlotName = ChildSlotNameFromParamName(wszParamName);
	if (!sChildSlotName.IsEmpty())
	{
		if (sChildSlotName == CS_TEX_TWO_COLOR_COLOR_2)
		{
			const CLBPString sExtraRequirementName = wszExtraReqName;

			if (sExtraRequirementName == RDK_TEXTURE_ON)
			{
				const bool b = vValue.AsBool();
				if ((bool)m_pImpl2->m_bTextureOn2 != b)
				{
					m_pImpl2->m_bTextureOn2 = b;
					Changed();
				}

				return true;
			}
			else
				if (sExtraRequirementName == RDK_TEXTURE_AMOUNT)
				{
					const double d = vValue.AsDouble() / 100.0;
					if (!LBPIsDoubleEqual(m_pImpl2->m_textureAmount2, d))
					{
						m_pImpl2->m_textureAmount2 = d;
						Changed();
					}

					return true;
				}
		}
	}
	return false;
}

void CRhRdkTwoColorTextureBase::AddUISections(IRhRdkExpandableContentUI& ui)
{
	AddTwoColorSection(ui);

	__super::AddUISections(ui);
}

void* CRhRdkOneColorTextureBase::GetShader(const UUID& uuidRenderEngine, void* pvData) const
{
	// Built-in textures have no shader.
	return nullptr;
}

unsigned int CRhRdkOneColorTextureBase::BitFlags(void) const
{
	return (__super::BitFlags() & ~bfTextureSummary) | bfAdjustment | bfFields;
}

bool CRhRdkOneColorTextureBase::IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const
{
	return f.IsKind(CRhRdkContent::Kinds::Texture);
}

CRhRdkContent::CChildSlotIterator* CRhRdkOneColorTextureBase::NewChildSlotIterator(CSIContext context) const
{
	return new CRhRdkOneColorTextureBaseCSI(this, context);
}

CRhRdkContent::CChildSlotIterator* CRhRdkTwoColorTextureBase::NewChildSlotIterator(CSIContext context) const
{
	return new CRhRdkTwoColorTextureBaseCSI(this, context);
}

class CRhRdkOneColorTextureBaseCSI::CImpl
{
public:
	CImpl() : m_iIndex(0) { }
	int m_iIndex;
};

CRhRdkOneColorTextureBaseCSI::CRhRdkOneColorTextureBaseCSI(const CRhRdkContent* pContent, CRhRdkContent::CSIContext context)
	:
	CRhRdkContent::CChildSlotIterator(pContent, context)
{
	m_pImpl = new CImpl;
}

CRhRdkOneColorTextureBaseCSI::~CRhRdkOneColorTextureBaseCSI()
{
	SAFE_DELETE(m_pImpl);
}

int CRhRdkOneColorTextureBaseCSI::Index(void) const
{
	return m_pImpl->m_iIndex;
}

void CRhRdkOneColorTextureBaseCSI::IncIndex(void)
{
	m_pImpl->m_iIndex++;
}

CRhRdkTwoColorTextureBaseCSI::CRhRdkTwoColorTextureBaseCSI(const CRhRdkContent* pContent, CRhRdkContent::CSIContext context)
	:
	CRhRdkOneColorTextureBaseCSI(pContent, context)
{
}

CRhRdkTwoColorTextureBaseCSI::~CRhRdkTwoColorTextureBaseCSI()
{
}

bool CRhRdkOneColorTextureBaseCSI::NextChildSlot(ON_wString& sParamNameOut, ON_wString& sChildSlotNameOut, ON_wString& sDisplayNameOut)
{
	if (0 == Index())
	{
		sChildSlotNameOut = CS_TEX_TWO_COLOR_COLOR_1;
		sDisplayNameOut = RhLocalizeString( L"Color1", 22786);
		sParamNameOut = m_pContent->ParamNameFromChildSlotName(sChildSlotNameOut.Array());

		IncIndex();

		return true;
	}

	return false;
}

bool CRhRdkTwoColorTextureBaseCSI::NextChildSlot(ON_wString& sParamNameOut, ON_wString& sChildSlotNameOut, ON_wString& sDisplayNameOut)
{
	if (CRhRdkOneColorTextureBaseCSI::NextChildSlot(sParamNameOut, sChildSlotNameOut, sDisplayNameOut))
		return true;

	if (1 == Index())
	{
		sChildSlotNameOut = CS_TEX_TWO_COLOR_COLOR_2;
		sDisplayNameOut = RhLocalizeString( L"Color2", 22787);
		sParamNameOut = m_pContent->ParamNameFromChildSlotName(sChildSlotNameOut.Array());

		IncIndex();

		return true;
	}

	return false;
}

void CRhRdkOneColorTextureBase::AddAutoParameters(IRhRdkParamBlock& paramBlock, int sectionId) const
{
	Fields().AddValuesToParamBlock(paramBlock, sectionId, RDK_TEXTURING);
}

void CRhRdkOneColorTextureBase::GetAutoParameters(const IRhRdkParamBlock& paramBlock, int sectionId)
{
	Fields().GetValuesFromParamBlock(paramBlock, sectionId);
}

CRhRdkContent* CRhRdkOneColorTextureBase::MakeCopy(CopyMethods m) const
{
	auto* pCopy = FastMakeCopy(m);

#ifdef _DEBUG
	VerifyCopy(pCopy);
#endif

	return pCopy;
}
