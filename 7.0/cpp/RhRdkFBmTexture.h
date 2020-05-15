
#pragma once

#include "RhRcmUtilities.h"
#include "Resource.h"

class CRhRdkFBmTexture : public CRhRdkTwoColorTextureBase
{
	typedef CRhRdkTwoColorTextureBase _super;
public:
	CRhRdkFBmTexture();

	double Gain(void) const;
	void SetGain(double bGain);

	double Roughness(void) const;
	void SetRoughness(double bRoughness);

	int MaxOctaves(void) const;
	void SetMaxOctaves(int bMaxOctaves);

protected: // Overrides from CRhRdkContent.
	virtual UUID TypeId(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual const wchar_t* Category(void) const override { return RDK_CAT_PROCEDURAL_3D; }
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#fbm"); }
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_FBM, size, dibOut); }

	class Evaluator;

private:
	CRhRdkContentField m_iMaxOctaves;
	CRhRdkContentField m_dRoughness;
	CRhRdkContentField m_dGain;
};

class CRhRdkTurbulenceTexture : public CRhRdkFBmTexture
{
protected: // Overrides from CRhRdkContent.
	virtual UUID TypeId(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual const wchar_t* Category(void) const override { return RDK_CAT_PROCEDURAL_3D; }
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_TURBULENCE, size, dibOut); }

	class Evaluator;
};
