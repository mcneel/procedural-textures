
#pragma once

#include "RhRcmUtilities.h"
#include "Resource.h"

class CRhRdkCheckerTexture : public CRhRdkTwoColorTextureBase
{
	typedef CRhRdkTwoColorTextureBase _super;
public:
	CRhRdkCheckerTexture();
	virtual ~CRhRdkCheckerTexture();

	bool RemapTextures(void) const;
	void SetRemapTextures(bool bRemap);

	// Modifies the local mapping to get 4 checks on a single UV space block.
	virtual ON_Xform LocalMappingTransform(void) const override;

protected: // Overrides from CRhRdkContent.
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_CHECKER, size, dibOut); }
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#2d_checker"); }

private:
	CRhRdkContentField m_bRemapTextures;

protected:
	class Evaluator;
};

class CRhRdk2DCheckerTexture : public CRhRdkCheckerTexture
{
	typedef CRhRdkCheckerTexture _super;
public:
	virtual UUID TypeId(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual const wchar_t* Category(void) const override { return RDK_CAT_PROCEDURAL_2D; }

protected:
	class Evaluator;
};

class CRhRdk3DCheckerTexture : public CRhRdkCheckerTexture
{
	typedef CRhRdkCheckerTexture _super;
public:
	virtual UUID TypeId(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual const wchar_t* Category(void) const override { return RDK_CAT_PROCEDURAL_3D; }

protected:
	class Evaluator;
};
