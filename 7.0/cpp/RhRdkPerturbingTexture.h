
#pragma once

#include "RhRcmUtilities.h"
#include "Resource.h"

class CRhRdkPerturbingTexture : public CRhRdkTexture
{
	typedef CRhRdkTexture _super;
public:
	CRhRdkPerturbingTexture(void);
	virtual ~CRhRdkPerturbingTexture(void);

	double Amount(void) const;
	void SetAmount(double d);

public:
	virtual UUID RenderEngineId(void) const override;
	virtual UUID PlugInId(void) const override;
	virtual UUID TypeId(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual const wchar_t* Category(void) const override;
	virtual void* GetShader(const UUID& uuidRenderEngine, void* pvData) const override { return NULL; }
	virtual bool WriteParametersToSection(IRhRdk_XMLSection& section, WriteParamsContext context) const override;
	virtual bool ReadParametersFromSection(const IRhRdk_XMLSection& section, ReadParamsContext context);
	virtual bool SetParameter(const wchar_t* wszName, const CRhRdkVariant& value);
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual bool IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const override;
	virtual CChildSlotIterator* NewChildSlotIterator(CSIContext context) const override;
	virtual unsigned int ComputeRenderCRC(const CRenderCRCFlags& rcrcFlags) const override;
	virtual CRhRdkContent* MakeCopy(CopyMethods m=CopyMethods::Normal) const override;

protected:
	virtual unsigned int BitFlags(void) const override;
	virtual bool IsImageBased(void) const override { return false; }
	virtual bool IsBuiltIn(void) const override { return true; }
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#perturbing"); }
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_PERTURBING, size, dibOut); }

private:
	CRhRdkContentField m_dAmount;
};
