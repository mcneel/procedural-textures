
#pragma once

#include "RhRdkContentKinds.h"
#include "RhRdkContentField.h"
#include "RhRdkTexture.h"
#include "RhRdkTextureFactory.h"
#include "RhRcmUtilities.h"
#include "RhRdkSubNodeCtrl.h"
#include "Resource.h"
#include "RhRdkHelpButtonHandler.h"
#include "RhRdkBitmapTexture.h"

enum CRhRdkBlurType { box, triangle, gaussian, mitnet };

class CRhRdkResampleTexture : public CRhRdkTexture, protected CRhRdkPixelTextureMixin
{
	typedef CRhRdkTexture _super;
public:
	CRhRdkResampleTexture(void);
	virtual ~CRhRdkResampleTexture(void);

	int DivisionsU(void) const;
	void SetDivisionsU(int i);

	int DivisionsV(void) const;
	void SetDivisionsV(int i);

	int MinSamples(void) const;
	void SetMinSamples(int i);

	int MaxSamples(void) const;
	void SetMaxSamples(int i);

	bool Interpolate(void) const;
	void SetInterpolate(bool b);

	bool BlurOn(void) const;
	void SetBlurOn(bool d);

	double BlurRadius(void) const;
	void SetBlurRadius(double d);

	bool BlurTiledU(void) const;
	void SetBlurTiledU(bool b);

	bool BlurTiledV(void) const;
	void SetBlurTiledV(bool b);

	CRhRdkBlurType BlurType(void) const;
	void SetBlurType(CRhRdkBlurType type);

public:
	virtual bool IsHDRCapable(void) const override;

protected:
	virtual UUID RenderEngineId(void) const override;
	virtual UUID PlugInId(void) const override;
	virtual UUID TypeId(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual const wchar_t* Category(void) const override;
	virtual void* GetShader(const UUID& uuidRenderEngine, void* pvData) const override { return NULL; }
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_RESAMPLE, size, dibOut); }
	virtual bool WriteParametersToSection(IRhRdk_XMLSection& section, WriteParamsContext context) const override;
	virtual bool ReadParametersFromSection(const IRhRdk_XMLSection& section, ReadParamsContext context) override;
	virtual CRhRdkVariant GetParameter(const wchar_t* wszName) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual bool GetExtraRequirementParameter(const wchar_t* wszParamName, const wchar_t* wszExtraReqName, CRhRdkVariant& vValueOut) const override;
	virtual bool IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const override;
	virtual CChildSlotIterator* NewChildSlotIterator(CSIContext context) const override;
	virtual unsigned int ComputeRenderCRC(const CRenderCRCFlags& rcrcFlags) const override;
	virtual unsigned int BitFlags(void) const override;
	virtual CRhRdkContent* MakeCopy(CopyMethods m=CopyMethods::Normal) const override;
	virtual bool IsImageBased(void) const override { return false; }
	virtual bool IsBuiltIn(void) const override { return true; }
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#resample"); }

protected: // Implement CRhRdkPixelTextureMixin.
	virtual bool GetImageSize(CSize& size) const override;
	virtual bool PixelSize(int& u, int& v, int& w) const override;

private:
	CRhRdkContentField m_iDivisionsU;
	CRhRdkContentField m_iDivisionsV;
	CRhRdkContentField m_iMinSamples;
	CRhRdkContentField m_iMaxSamples;
	CRhRdkContentField m_bInterpolate;
	CRhRdkContentField m_bBlurOn;
	CRhRdkContentField m_dBlurRadius;
	CRhRdkContentField m_bBlurUTiled;
	CRhRdkContentField m_bBlurVTiled;
	CRhRdkContentField m_sBlurType;
};
