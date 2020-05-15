
#pragma once

#include "RhRdkTexture.h"
#include "RhRdkBitmapTexture.h"
#include "RhRdkContentField.h"

#define PCTB CRhRdkProjectionChangingTextureBase

class CRhRdkProjectionChangingTextureBase : public CRhRdkFileTexture
{
	typedef CRhRdkFileTexture _super;
public:
	CRhRdkProjectionChangingTextureBase();

	enum class Projections { none, planar, lightprobe, equirect, cubemap, vertical_cross_cubemap,
	                         horizontal_cross_cubemap, emap, same_as_input, hemispherical };

public:
	Projections ProjectionIn(void) const;
	void SetProjectionIn(Projections p);

	Projections ProjectionOut(void) const;
	void SetProjectionOut(Projections p);

	virtual CRhRdkVariant GetParameter(const wchar_t* wszName) const override;

	virtual bool WriteParametersToSection(IRhRdk_XMLSection& section, WriteParamsContext context) const override;
	virtual bool ReadParametersFromSection(const IRhRdk_XMLSection& section, ReadParamsContext context) override;

	virtual void SimulateTexture(CRhRdkSimulatedTexture& tex, CRhRdkTexture::TextureGeneration tg, int size=-1, const CRhinoObject* pObject=nullptr) const override;

	virtual unsigned int BitFlags(void) const override;

	virtual EnvironmentMappingModes AutomaticEnvironmentMappingMode(void) const override;

	virtual void* EVF(const wchar_t* wszFunc, void* pvData) override;

protected:
	const wchar_t* StringFromProjection(Projections proj) const;
	Projections ProjectionFromString(const CLBPString& s) const;
	CRhRdkVariant GetCubeFaceMapParameter(const wchar_t* wszName, int iPart) const;

private:
	CRhRdkContentField m_sProjIn;
	CRhRdkContentField m_sProjOut;
};
