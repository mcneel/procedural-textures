
#pragma once

#include "RhRcmUtilities.h"
#include "RhRdkHelpButtonHandler.h"
#include "Resource.h"

class CRhRdkWoodTextureEvaluator;

class CRhRdkWoodTexture : public CRhRdkTwoColorTextureBase
{
	typedef CRhRdkTwoColorTextureBase _super;
public:
	CRhRdkWoodTexture();
	virtual ~CRhRdkWoodTexture();

	double GrainThickness(void) const;
	void   SetGrainThickness(double dc);

	double RadialNoise(void) const;
	void   SetRadialNoise(double d);

	double AxialNoise(void) const;
	void   SetAxialNoise(double d);

	double Blur1(void) const;
	void   SetBlur1(double d);

	double Blur2(void) const;
	void   SetBlur2(double d);

public:
	virtual UUID RenderEngineId(void) const override;
	virtual UUID PlugInId(void) const override;
	virtual UUID TypeId(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual const wchar_t* Category(void) const override;
	virtual void* GetShader(const UUID& uuidRenderEngine, void* pvData) const override { return 0; }
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_WOOD, size, dibOut); }
	virtual bool IsBuiltIn(void) const override { return true; }
	virtual bool IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;

protected:
	virtual bool IsImageBased(void) const override { return false; }
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#wood"); }

private:
	CRhRdkContentField m_grainThickness;
	CRhRdkContentField m_radialNoise;
	CRhRdkContentField m_axialNoise;
	CRhRdkContentField m_blur1;
	CRhRdkContentField m_blur2;
};
