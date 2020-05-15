
#pragma once

#include "RhRcmUtilities.h"
#include "Resource.h"

class CRhRdkMarbleTextureEvaluator;

class CRhRdkMarbleTexture : public CRhRdkTwoColorTextureBase
{
	typedef CRhRdkTwoColorTextureBase _super;
public:
	CRhRdkMarbleTexture();

	double Noise(void) const;
	void SetNoise(double bNoise);

	double Blur(void) const;
	void SetBlur(double bBlur);

	double VeinWidth(void) const;
	void SetVeinWidth(double bVeinWidth);

	double Size(void) const;
	void SetSize(double bSize);

protected: // Overrides from CRhRdkContent.
	virtual UUID TypeId(void) const override;
	virtual unsigned int BitFlags(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual const wchar_t* Category(void) const override { return RDK_CAT_PROCEDURAL_3D; }
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_MARBLE, size, dibOut); }
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#marble"); }

private:
	CRhRdkContentField m_dSize;
	CRhRdkContentField m_dVeinWidth;
	CRhRdkContentField m_dBlur;
	CRhRdkContentField m_dNoise;
};
