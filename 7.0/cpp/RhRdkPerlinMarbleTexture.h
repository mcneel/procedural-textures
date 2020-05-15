
#pragma once

#include "RhRcmUtilities.h"
#include "Resource.h"

class CRhRdkPerlinMarbleTexture : public CRhRdkTwoColorTextureBase
{
	typedef CRhRdkTwoColorTextureBase _super;
public:
	CRhRdkPerlinMarbleTexture();

	double Size(void) const;
	void SetSize(double d);

	int Levels(void) const;
	void SetLevels(int i);

	double Noise(void) const;
	void SetNoise(double d);

	double Blur(void) const;
	void SetBlur(double d);

	double Color1Saturation(void) const;
	void SetColor1Saturation(double d);

	double Color2Saturation(void) const;
	void SetColor2Saturation(double d);

protected: // Overrides from CRhRdkContent.
	virtual UUID TypeId(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual const wchar_t* Category(void) const override { return RDK_CAT_PROCEDURAL_3D; }
	virtual bool SetParameter(const wchar_t* wszName, const CRhRdkVariant& value);
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#perlin"); }
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_PERLIN_MARBLE, size, dibOut); }

private:
	CRhRdkContentField m_dSize;
	CRhRdkContentField m_iLevels;
	CRhRdkContentField m_dNoise;
	CRhRdkContentField m_dBlur;
	CRhRdkContentField m_dCol1Sat;
	CRhRdkContentField m_dCol2Sat;
	class Evaluator;
};
