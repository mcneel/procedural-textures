
#pragma once

#include "RhRcmUtilities.h"
#include "Resource.h"

class CRhRdkBlendTexture : public CRhRdkTwoColorTextureBase
{
	typedef CRhRdkTwoColorTextureBase _super;
public:
	CRhRdkBlendTexture(void);

	double BlendFactor(void) const;
	void SetBlendFactor(double d);

	bool TextureOn(void) const;
	void SetTextureOn(bool b);

protected:
	class Evaluator;
	virtual UUID RenderEngineId(void) const override;
	virtual UUID PlugInId(void) const override;
	virtual UUID TypeId(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual CRhRdkContent* MakeCopy(CopyMethods m=CopyMethods::Normal) const override;
	virtual const wchar_t* Category(void) const override;
	virtual void* GetShader(const UUID& uuidRenderEngine, void* pvData) const override { return NULL; }
	virtual bool IsBuiltIn(void) const override { return true; }
	virtual bool IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const override;
	virtual bool IsImageBased(void) const override { return false; }
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#blend"); }
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_BLEND, size, dibOut); }

public:
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;

private:
	CRhRdkContentField m_bTextureOn;
	CRhRdkContentField m_blendFactor;
};
