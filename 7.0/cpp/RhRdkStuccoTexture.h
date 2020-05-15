
#pragma once

#include "RhRcmUtilities.h"
#include "Resource.h"

class CRhRdkStuccoTexture : public CRhRdkTwoColorTextureBase
{
	typedef CRhRdkTwoColorTextureBase _super;
public:
	CRhRdkStuccoTexture();

	double Size(void) const;
	void SetSize(double d);

	double Thickness(void) const;
	void SetThickness(double d);

	double Threshold(void) const;
	void SetThreshold(double d);

protected: // Overrides from CRhRdkContent.
	virtual bool Initialize(void) override;
	virtual UUID TypeId(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual const wchar_t* Category(void) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_STUCCO, size, dibOut); }
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#stucco"); }

private:
	CRhRdkContentField m_dThickness;
	CRhRdkContentField m_dThreshold;
};
