
#include "stdafx.h"
#include "RhRdkTileTexture.h"
#include "RhRdkTextureUtilities.h"

class CRhRdkTileTexture::Evaluator : public CRhRdkTwoColorEvaluator
{
public:
	Evaluator(const CRhRdkTileTexture& texture, CEvalFlags ef);

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw,
									const ON_3dVector& duvwdx, 
									const ON_3dVector& duvwdy, void* pvData) const;

	void ApplyPhaseShift(ON_3dPoint& uvw) const;

	bool RectangularTest3d(const ON_3dPoint& uvw) const;
	bool RectangularTest2d(const ON_3dPoint& uvw) const;
	bool HexagonalTest(const ON_3dPoint& uvw) const;
	bool TriangularTest(const ON_3dPoint& uvw) const;
	bool OctagonalTest(const ON_3dPoint& uvw) const;

private:
	double m_dJointWidth[3];
	double m_dPhase[3];
	CRhRdkTileTexture::eTileType m_type;
};

CRhRdkTileTexture::Evaluator::Evaluator(const CRhRdkTileTexture& texture, CEvalFlags ef)
	:
	CRhRdkTwoColorEvaluator(texture, ef)
{
	for (int i = 0; i < 3; i++)
	{
		m_dJointWidth[i] = texture.JointWidth(i);
		m_dPhase[i] = texture.Phase(i);
	}

	m_type = texture.TileType();
}

bool CRhRdkTileTexture::Evaluator::RectangularTest2d(const ON_3dPoint& uvw) const
{
	// Determine if on a joint.
	bool bLineHit = false;

	for (int i = 0; i < 2; i++)
	{
		double dHalfWidth = m_dJointWidth[i] * 0.5 /** m_repeat[i] */;

		if (uvw[i] < dHalfWidth || uvw[i] > 1.0 - dHalfWidth)
		{
			bLineHit = true;
			break;
		}
	}

	return bLineHit;
}

bool CRhRdkTileTexture::Evaluator::RectangularTest3d(const ON_3dPoint& uvw) const
{
	// Determine if on a joint.
	bool bLineHit = false;

	for (int i = 0; i < 3; i++)
	{
		double dHalfWidth = m_dJointWidth[i] * 0.5 /** m_repeat[i] */;

		if (uvw[i] < dHalfWidth || uvw[i] > 1.0 - dHalfWidth)
		{
			bLineHit = true;
			break;
		}
	}

	return bLineHit;
}

bool IsPointNearHalfLineStartingFromOrigin(double pointU, double pointV, double tangentU, double tangentV, double distance)
{
	const double dotP = pointU * tangentU + pointV * tangentV;

	if (0.0 > dotP)
		return false;

	const double pointDistance = pointU * tangentV - pointV * tangentU;

	if (pointDistance > distance)
		return false;

	if (-pointDistance > distance)
		return false;

	return true;
}

bool CRhRdkTileTexture::Evaluator::HexagonalTest(const ON_3dPoint& uvw) const
{
	static const double sqrtOfThree = sqrt(3.0);
	const double u = (uvw.x <= 0.5 ? uvw.x : 1.0 - uvw.x);
	const double v = (uvw.y <= 0.5 ? uvw.y : 1.0 - uvw.y);

	const double knot_u = (u < v ? 0.0 : 0.5);
	const double knot_v = (u < v ? 2.0 / 6.0 : 1.0 / 6.0);
	const double dJunctionVScale = (u < v ? 1.0 : -1.0);

	const double trim_u = (u - knot_u);
	const double trim_v = (v - knot_v) * sqrtOfThree * dJunctionVScale;

	const double maxJointWidth = std::max(m_dJointWidth[0], m_dJointWidth[1]);
	const double maxJointWidthSquared = maxJointWidth * maxJointWidth;

	const double trimLengthSquared = trim_u * trim_u + trim_v * trim_v;

	if (4.0 * trimLengthSquared < maxJointWidthSquared)
		return true;

	if (IsPointNearHalfLineStartingFromOrigin(trim_u, trim_v, 0.0, 1.0, m_dJointWidth[0] / 2.0))
		return true;

	if (IsPointNearHalfLineStartingFromOrigin(trim_u, trim_v, 0.5 * sqrtOfThree, -0.5, m_dJointWidth[1] / 2.0))
		return true;

	if (IsPointNearHalfLineStartingFromOrigin(trim_u, trim_v, -0.5 * sqrtOfThree, -0.5, m_dJointWidth[1] / 2.0))
		return true;

	return false;
}

bool CRhRdkTileTexture::Evaluator::TriangularTest(const ON_3dPoint& uvw) const
{
	static const double vFactor = sqrt(3.0) / 2.0;
	double u = uvw.x > 0.5 ? 1.0 - uvw.x : uvw.x;
	double v = (uvw.y > 0.5 ? 1.0 - uvw.y : uvw.y) * 2.0 * vFactor;

	double dHalfWidth = m_dJointWidth[0] * 0.5;

	if (v < dHalfWidth || v > vFactor - dHalfWidth)
		return true;

	const double v2 = v / vFactor;
	u -= (1.0 - v2) * 0.5;

	dHalfWidth = m_dJointWidth[1] * 0.5 * vFactor;

	return std::abs(u) < dHalfWidth;
}

bool CRhRdkTileTexture::Evaluator::OctagonalTest(const ON_3dPoint& uvw) const
{
	static const double sqrtOfTwo = sqrt(2.0);
	static const double b = 1.0 / (2.0 + sqrtOfTwo);

	double u = uvw.x > 0.5 ? 1.0 - uvw.x : uvw.x;
	double v = uvw.y > 0.5 ? 1.0 - uvw.y : uvw.y;

	if (u + v < b - sqrtOfTwo * 0.5 * m_dJointWidth[0])
	{
		if (v > b - 0.5 * m_dJointWidth[1])
			return true;
	
		if (u > b - 0.5 * m_dJointWidth[1])
			return true;

		return false;
	}

	if (u < 0.5 * m_dJointWidth[1])
		return true;

	if (v < 0.5 * m_dJointWidth[1])
		return true;

	if (u + v < b + sqrtOfTwo * 0.5 * m_dJointWidth[0])
		return true;

	return false;
}

CRhRdkColor CRhRdkTileTexture::Evaluator::GetColorSample(const ON_3dPoint& uvwConst, const ON_3dVector& duvwdx, 
                                                         const ON_3dVector& duvwdy, void* pvData) const
{
	CRhRdkColor colOut = CRhRdkColor::black;
	ON_3dPoint uvw = LocalMappingTransform() * uvwConst;
	const int uCell = Floor2Int(uvw.x), vCell = Floor2Int(uvw.y);//, wCell = Floor2Int(uvw.z);

	ON_3dVector phaseShift = ON_origin;
	if (LBP_IsOdd(vCell)) phaseShift.x += m_dPhase[0];
	if (LBP_IsOdd(uCell)) phaseShift.y += m_dPhase[1];
	if (LBP_IsOdd(uCell)) phaseShift.x += m_dPhase[2];
	uvw = uvw + phaseShift;
	NormalizeUVW(uvw);

	bool bLineHit = false;

	switch (m_type)
	{
	case CRhRdkTileTexture::tt_3d_rectangular:
		bLineHit = RectangularTest3d(uvw);
		break;
	case CRhRdkTileTexture::tt_2d_rectangular:
		bLineHit = RectangularTest2d(uvw);
		break;
	case CRhRdkTileTexture::tt_2d_hexagonal:
		bLineHit = HexagonalTest(uvw);
		break;
	case CRhRdkTileTexture::tt_2d_triangular:
		bLineHit = TriangularTest(uvw);
		break;
	case CRhRdkTileTexture::tt_2d_octagonal:
		bLineHit = OctagonalTest(uvw);
		break;
	}

	colOut = OutputColor(bLineHit ? 1 : 2, uvwConst, duvwdx, duvwdy, pvData);
	return colOut;
}

//---------------------

CRhRdkTileTexture::CRhRdkTileTexture()
	:
	m_Type   (*this, FS_TEX_TILE_TYPE,    RhLocalizeString( L"Tile type", 22761), L"Tile type"),
	m_dWidthX(*this, FS_TEX_TILE_WIDTH_X, RhLocalizeString( L"Width X"  , 22755), L"Width X"),
	m_dWidthY(*this, FS_TEX_TILE_WIDTH_Y, RhLocalizeString( L"Width Y"  , 22756), L"Width Y"),
	m_dWidthZ(*this, FS_TEX_TILE_WIDTH_Z, RhLocalizeString( L"Width Z"  , 22757), L"Width Z"),
	m_dPhaseX(*this, FS_TEX_TILE_PHASE_X, RhLocalizeString( L"Phase X"  , 22758), L"Phase X"),
	m_dPhaseY(*this, FS_TEX_TILE_PHASE_Y, RhLocalizeString( L"Phase Y"  , 22759), L"Phase Y"),
	m_dPhaseZ(*this, FS_TEX_TILE_PHASE_Z, RhLocalizeString( L"Phase Z"  , 22760), L"Phase Z")
{
	m_dWidthX = m_dWidthY = 0.01;
	m_dWidthZ = m_dPhaseX = m_dPhaseY = m_dPhaseZ = 0.0;

	m_Type = TileTypeToString(tt_3d_rectangular);

	m_dWidthX.SetLimits(0.0, CRhRdkVariant::Null());
	m_dWidthY.SetLimits(0.0, CRhRdkVariant::Null());
	m_dWidthZ.SetLimits(0.0, CRhRdkVariant::Null());

	m_dPhaseX.SetLimits(0.0, CRhRdkVariant::Null());
	m_dPhaseY.SetLimits(0.0, CRhRdkVariant::Null());
	m_dPhaseZ.SetLimits(0.0, CRhRdkVariant::Null());
}

UUID CRhRdkTileTexture::TypeId(void) const
{
	return uuidTileTextureType;
}

ON_wString CRhRdkTileTexture::TypeName(void) const
{
	return RhLocalizeString( L"Tile Texture", 22762);
}

ON_wString CRhRdkTileTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Tile texture.", 22763);
}

ON_wString CRhRdkTileTexture::InternalName(void) const
{
	return L"rcm-basic-tile-texture";
}

double CRhRdkTileTexture::JointWidth(int index) const
{
	switch (index)
	{
	case 1: return m_dWidthY;
	case 2: return m_dWidthZ;
	}

	return m_dWidthX;
}

void CRhRdkTileTexture::SetJointWidth(int index, double n)
{
	switch (index)
	{
	case 0: m_dWidthX.SetValue(n); break;
	case 1: m_dWidthY.SetValue(n); break;
	case 2: m_dWidthZ.SetValue(n); break;
	}
}

double CRhRdkTileTexture::Phase(int index) const
{
	switch (index)
	{
	case 1: return m_dPhaseY;
	case 2: return m_dPhaseZ;
	}

	return m_dPhaseX;
}

void CRhRdkTileTexture::SetPhase(int index, double n)
{
	switch (index)
	{
	case 0: m_dPhaseX.SetValue(n); break;
	case 1: m_dPhaseY.SetValue(n); break;
	case 2: m_dPhaseZ.SetValue(n); break;
	}
}

CRhRdkTileTexture::eTileType CRhRdkTileTexture::TileType(void) const
{
	return StringToTileType(m_Type.Value());
}

void CRhRdkTileTexture::SetTileType(eTileType tt)
{
	m_Type.SetValue(TileTypeToString(tt).Array());
}

CRhRdkTileTexture::eTileType CRhRdkTileTexture::StringToTileType(const wchar_t* wsz)
{
	const CLBPString s = wsz;

	if (s == FS_TEX_TILE_TYPE_2D_RECTANGULAR) return tt_2d_rectangular;
	if (s == FS_TEX_TILE_TYPE_2D_TRIANGULAR)  return tt_2d_triangular;
	if (s == FS_TEX_TILE_TYPE_2D_HEXAGONAL)   return tt_2d_hexagonal;
	if (s == FS_TEX_TILE_TYPE_2D_OCTAGONAL)   return tt_2d_octagonal;

	return tt_3d_rectangular;
}

ON_wString CRhRdkTileTexture::TileTypeToString(eTileType tt)
{
	switch (tt)
	{
	case tt_2d_rectangular: return FS_TEX_TILE_TYPE_2D_RECTANGULAR;
	case tt_2d_triangular:  return FS_TEX_TILE_TYPE_2D_TRIANGULAR;
	case tt_2d_hexagonal:   return FS_TEX_TILE_TYPE_2D_HEXAGONAL;
	case tt_2d_octagonal:   return FS_TEX_TILE_TYPE_2D_OCTAGONAL;
	}
	return FS_TEX_TILE_TYPE_3D_RECTANGULAR;
}

IRhRdkTextureEvaluator* CRhRdkTileTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new Evaluator(*this, ef);
}
