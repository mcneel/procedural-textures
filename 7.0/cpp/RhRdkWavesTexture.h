
#pragma once

#include "RhRcmUtilities.h"
#include "RhRcmButton.h"
#include "RhRdkHelpButtonHandler.h"
#include "Resource.h"

class CRhRdkWavesTexture : public CRhRdkTwoColorTextureBase
{
	typedef CRhRdkTwoColorTextureBase _super;
public:
	CRhRdkWavesTexture();

	double Contrast1(void) const;
	void SetContrast1(double dContrast1);

	double Contrast2(void) const;
	void SetContrast2(double dContrast2);

	double WaveWidth(void) const;
	void SetWaveWidth(double dWaveWidth);

	bool WaveWidthTextureOn(void) const;
	void SetWaveWidthTextureOn(bool b);

	RhRdkWaveType WaveType(void) const;
	void SetWaveType(RhRdkWaveType type);

protected: // Overrides from CRhRdkContent.
	virtual UUID TypeId(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_WAVES, size, dibOut); }
	virtual const wchar_t* Category(void) const override { return RDK_CAT_PROCEDURAL_2D; }
	virtual bool IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const override;
	virtual CChildSlotIterator* NewChildSlotIterator(CSIContext context) const override;
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#waves"); }

private:
	CRhRdkContentField m_dContrast1;
	CRhRdkContentField m_dContrast2;
	CRhRdkContentField m_iWaveType;
	CRhRdkContentField m_bWaveWidthTextureOn;
	CRhRdkContentField m_dWaveWidth;

	class Evaluator;
};
