
#pragma once

#include "RhRcmUtilities.h"
#include "Resource.h"

class CRhRdkTileTexture : public CRhRdkTwoColorTextureBase
{
	typedef CRhRdkTwoColorTextureBase _super;
public:
	CRhRdkTileTexture();

	double JointWidth(int index) const;
	void SetJointWidth(int index, double n);

	double Phase(int index) const;
	void SetPhase(int index, double n);

	enum eTileType
	{
		tt_3d_rectangular,
		tt_2d_rectangular,
		tt_2d_triangular,
		tt_2d_hexagonal,
		tt_2d_octagonal
	};

	eTileType TileType() const;
	void SetTileType(eTileType tt);

	static eTileType StringToTileType(const wchar_t* wsz);
	static ON_wString TileTypeToString(eTileType);

protected: // Overrides from CRhRdkContent.
	virtual UUID TypeId(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual const wchar_t* Category(void) const override { return RDK_CAT_PROCEDURAL_3D; }
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_TILE, size, dibOut); }
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#tile"); }

private:
	class Evaluator;
	CRhRdkContentField m_dWidthX;
	CRhRdkContentField m_dWidthY;
	CRhRdkContentField m_dWidthZ;
	CRhRdkContentField m_dPhaseX;
	CRhRdkContentField m_dPhaseY;
	CRhRdkContentField m_dPhaseZ;
	CRhRdkContentField m_Type;
};
