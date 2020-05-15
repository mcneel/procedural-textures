
#pragma once

#include "RhRcmUtilities.h"
#include "RhRdkHelpButtonHandler.h"
#include "Resource.h"

class CRhRdkMaskTexture : public CRhRdkTexture
{
	typedef CRhRdkTexture _super;
public:
	CRhRdkMaskTexture(void);

	enum class MaskTypes { Luminance, Red, Green, Blue, Alpha };
	MaskTypes MaskType(void) const;
	void SetMaskType(MaskTypes type);

public:
	virtual UUID RenderEngineId(void) const override;
	virtual UUID PlugInId(void) const override;
	virtual UUID TypeId(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual const wchar_t* Category(void) const override;
	virtual void* GetShader(const UUID& uuidRenderEngine, void* pvData) const override { return nullptr; }
	virtual bool WriteParametersToSection(IRhRdk_XMLSection& section, WriteParamsContext context) const override;
	virtual bool ReadParametersFromSection(const IRhRdk_XMLSection& section, ReadParamsContext context);
	virtual bool IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const override;
	virtual CChildSlotIterator* NewChildSlotIterator(CSIContext context) const override;
	virtual unsigned int ComputeRenderCRC(const CRenderCRCFlags& rcrcFlags) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;

protected:
	virtual unsigned int BitFlags(void) const override;
	virtual bool IsImageBased(void) const override { return false; }
	virtual bool IsBuiltIn(void) const override { return true; }
	virtual CRhRdkContent* MakeCopy(CopyMethods m=CopyMethods::Normal) const override;
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#mask"); }
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_MASK, size, dibOut); }
	virtual bool PixelSize(int& widthOut, int& heightOut, int& depthOut) const override;

private:
	CRhRdkContentField m_sMaskType;
};
