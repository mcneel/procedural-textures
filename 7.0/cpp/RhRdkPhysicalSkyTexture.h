
#pragma once

#include "RhRcmUtilities.h"
#include "RhRcm.h"
#include "Resource.h"

ON_wString CRhRdkPhysicalSkyBase_AddUISections_LocalString();
ON_wString CRhRdkPhysicalSkyBase_AddUISections_EnglishString();

template<class BASE>
class CRhRdkPhysicalSkyBase : public BASE, protected CRhRdkEventWatcher
{
public:
	CRhRdkPhysicalSkyBase(void);

	virtual UUID RenderEngineId(void) const override { return uuidUniversalRenderEngine; }
	virtual UUID PlugInId(void) const override { return rdk.RhinoPlugInUuid(); }

	virtual void* GetShader(const UUID& uuidRenderEngine, void* pvData) const override { return nullptr; }
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_PHYSICAL_SKY, size, dibOut); }

	virtual void AddAutoParameters(IRhRdkParamBlock& paramBlock, int sectionId) const override;
	virtual void GetAutoParameters(const IRhRdkParamBlock& paramBlock, int sectionId) override { BASE::Fields().GetValuesFromParamBlock(paramBlock, sectionId); }

	virtual bool IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const override { return false; }

	virtual unsigned int ComputeRenderCRC(const CRhRdkContent::CRenderCRCFlags&) const override
	{
		// Don't call the base class; avoids including loads of texture fields that aren't needed.

		ON__UINT32 crc = 0;

		const bool bUseDocumentSun = m_bUseDocumentSun;
		if (bUseDocumentSun)
		{
			// Include the date/time and location information from the document sun.
			const auto pRdkDoc = CRhRdkContent::RdkDocumentAssoc();
			if (nullptr != pRdkDoc)
			{
				UPDATE_CRC(L"CRhRdkPhysicalSkyBase pRdkDoc->Sun().CRC()", crc, pRdkDoc->Sun().CRC());
			}
		}
		else
		{
			// Include the date/time and location information from this texture.
			UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_time",     crc, m_time.Value().AsInteger());
			UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_vLatLong", crc, Integerize(m_vLatLong.Value().AsVector2d().x));
			UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_vLatLong", crc, Integerize(m_vLatLong.Value().AsVector2d().y));
		}

		// Always include all the other sun properties.
		UPDATE_CRC(L"CRhRdkPhysicalSkyBase::bUseDocumentSun",       crc, bUseDocumentSun);
		UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_bShowSun",            crc, m_bShowSun.Value().AsBool());
		UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_SunColor",            crc, m_SunColor.Value().AsInteger());
		UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_fSunSize",            crc, Integerize(m_fSunSize.Value().AsFloat()));
		UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_fSunBrightness",      crc, Integerize(m_fSunBrightness.Value().AsFloat()));
		UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_fAtmosphericDensity", crc, Integerize(m_fAtmosphericDensity.Value().AsFloat()));
		UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_fLightScattering",    crc, Integerize(m_fLightScattering.Value().AsFloat()));
		UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_fHaze",               crc, Integerize(m_fHaze.Value().AsFloat()));
		UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_fExposure",           crc, Integerize(m_fExposure.Value().AsFloat()));
		UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_Wavelengths.x",       crc, Integerize(m_Wavelengths.Value().AsVector3d().x));
		UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_Wavelengths.y",       crc, Integerize(m_Wavelengths.Value().AsVector3d().y));
		UPDATE_CRC(L"CRhRdkPhysicalSkyBase::m_Wavelengths.z",       crc, Integerize(m_Wavelengths.Value().AsVector3d().z));

		return crc;
	}

protected: // RDK event watcher overrides.
	virtual void OnSunSettingsChanged(const CRhRdkDocument& rdkDoc, const IRhRdkSun& newSun, const IRhRdkSun& oldSun, const void*) override
	{
		if (m_bUseDocumentSun && (&rdkDoc == CRhRdkContent::RdkDocumentOwner()))
		{
			const CRhRdkContent::Change<BASE> t(*this, RhRdkChangeContext::UI);
			BASE::Changed();
		}
	}

protected:
	virtual void AddUISections(IRhRdkExpandableContentUI& ui) override
	{
		BASE::AddAutomaticUISection(ui, CRhRdkPhysicalSkyBase_AddUISections_LocalString(), CRhRdkPhysicalSkyBase_AddUISections_EnglishString());
		BASE::AddUISections(ui);
	}

	virtual bool IsImageBased(void) const { return false; }
	virtual bool IsBuiltIn(void) const override { return true; }
	virtual bool SupportsContextHelp(void) const override { return false; }

	virtual bool WriteParametersToSection(IRhRdk_XMLSection& section, CRhRdkContent::WriteParamsContext context) const override { return BASE::WriteParametersToSection(section, context); }
	virtual bool ReadParametersFromSection(const IRhRdk_XMLSection& section, CRhRdkContent::ReadParamsContext context) override { return BASE::ReadParametersFromSection(section, context); }

	virtual bool GetExtraRequirementParameter(const wchar_t* wszParamName, const wchar_t* wszExtraReqName, CRhRdkVariant& vValueOut) const override
	{
		if (0 == _wcsicmp(wszExtraReqName, RDK_DATETIME_LOCATION))
		{
			vValueOut = m_vLatLong.Value();
			return true;
		}
		else
		if (0 == _wcsicmp(wszExtraReqName, RDK_LOCATION_DATETIME))
		{
			vValueOut = m_time.Value();
			return true;
		}
		else
		if (0 == _wcsicmp(wszExtraReqName, RDK_COLOR_BUTTON_ALPHA))
		{
			vValueOut = false;
			return true;
		}
		else
		if (0 == _wcsicmp(wszExtraReqName, RDK_CONTROLSET_ENABLE))
		{
			if (0 == _wcsicmp(wszParamName, m_vLatLong.InternalName()))
			{
				vValueOut = !m_bUseDocumentSun.Value().AsSmartBool();
			}
			if (0 == _wcsicmp(wszParamName, m_time.InternalName()))
			{
				vValueOut = !m_bUseDocumentSun.Value().AsSmartBool();
			}
		}

		return false;
	}

public:
	CRhRdkContentField m_time;
	CRhRdkContentField m_bUseDocumentSun;
	CRhRdkContentField m_bShowSun;
	CRhRdkContentField m_fSunBrightness;
	CRhRdkContentField m_fSunSize;
	CRhRdkContentField m_fAtmosphericDensity;
	CRhRdkContentField m_fLightScattering;
	CRhRdkContentField m_fHaze;
	CRhRdkContentField m_Wavelengths;
	CRhRdkContentField m_SunColor;
	CRhRdkContentField m_fExposure;
	CRhRdkContentField m_vLatLong;
};

enum e_PSS_StringId
{
	pss_date,
	pss_use_doc_sun,
	pss_show_sun,
	pss_sun_size,
	pss_coronal_color,
	pss_sun_brightness,
	pss_atmos_den,
	pss_light_scat,
	pss_part_scat,
	pss_light_wave,
	pss_exposure,
	pss_latlong
};

const wchar_t* EnglishPhysicalSkyString(e_PSS_StringId id);
const wchar_t* LocalPhysicalSkyString(e_PSS_StringId id);

template<class BASE>
CRhRdkPhysicalSkyBase<BASE>::CRhRdkPhysicalSkyBase()
	:
	m_time               (*this, L"date",                LocalPhysicalSkyString(pss_date),           EnglishPhysicalSkyString(pss_date)),
	m_bUseDocumentSun    (*this, L"use-document-sun",    LocalPhysicalSkyString(pss_use_doc_sun),    EnglishPhysicalSkyString(pss_use_doc_sun)),
	m_bShowSun           (*this, L"show-sun",            LocalPhysicalSkyString(pss_show_sun),       EnglishPhysicalSkyString(pss_show_sun)),
	m_fSunSize           (*this, L"sun-size",            LocalPhysicalSkyString(pss_sun_size),       EnglishPhysicalSkyString(pss_sun_size)),
	m_SunColor           (*this, L"coronal-color",       LocalPhysicalSkyString(pss_coronal_color),  EnglishPhysicalSkyString(pss_coronal_color)),
	m_fSunBrightness     (*this, L"sun-brightness",      LocalPhysicalSkyString(pss_sun_brightness), EnglishPhysicalSkyString(pss_sun_brightness)),
	m_fAtmosphericDensity(*this, L"atmospheric-density", LocalPhysicalSkyString(pss_atmos_den),      EnglishPhysicalSkyString(pss_atmos_den)),
	m_fLightScattering   (*this, L"light-scattering",    LocalPhysicalSkyString(pss_light_scat),     EnglishPhysicalSkyString(pss_light_scat)),
	m_fHaze              (*this, L"particle-scattering", LocalPhysicalSkyString(pss_part_scat),      EnglishPhysicalSkyString(pss_part_scat)),
	m_Wavelengths        (*this, L"light-wavelengths",   LocalPhysicalSkyString(pss_light_wave),     EnglishPhysicalSkyString(pss_light_wave)),
	m_fExposure          (*this, L"exposure",            LocalPhysicalSkyString(pss_exposure),       EnglishPhysicalSkyString(pss_exposure)),
	m_vLatLong           (*this, L"latlong",             LocalPhysicalSkyString(pss_latlong),        EnglishPhysicalSkyString(pss_latlong))
{
	m_time = CTime::GetCurrentTime().GetTime();

	m_bUseDocumentSun = false;
	m_bShowSun = true;
	m_fSunSize = 15.0f;
	m_SunColor = ON_Color( (int)(1.0f * 255), (int)(1.0f * 255), (int)(0.874509f * 255));
	m_fSunBrightness = 18.0f;
	m_fAtmosphericDensity = 50.0f;
	m_fLightScattering = 2.50f;
	m_fHaze = 2.0f;
	m_Wavelengths = ON_3dVector( 0.650f, 0.570f, 0.475f );
	m_fExposure = 1.0f;
	m_fSunSize.SetLimits(0.0f, 1000.0f);
	m_fSunBrightness.SetLimits(0.0f, 1000.0f);
	m_fAtmosphericDensity.SetLimits(1.0f, 1000.0f);
	m_fLightScattering.SetLimits(0.0f, 50.0f);
	m_fHaze.SetLimits(1.0f, 100.0f);
	m_fExposure.SetLimits(0.0f, 100.0f);
	m_vLatLong.SetLimits(ON_2dVector(LBP_DegreesToRadians(-90.0), LBP_DegreesToRadians(90.0)), ON_2dVector(LBP_DegreesToRadians(90.0), LBP_DegreesToRadians(180.0)));
	m_vLatLong.SetConversion(CRhRdkContentField::conv_radians);

	// We can't initialize the position from the earth anchor point like in V5 because at this point
	// there is no document to use. But it doesn't actually need to be as it's going to use a map
	// on the texture's UI anyway.
	m_vLatLong = ON_2dVector(0.0, 0.0);
}

using UiHints = IRhRdkParamBlock::UiHints;

template<class BASE>
void CRhRdkPhysicalSkyBase<BASE>::AddAutoParameters(IRhRdkParamBlock& paramBlock, int id) const
{
	if ( id != m_time.SectionId() )
		return;

	{	// Use document sun.
		CRhRdkVariant vMin, vMax;
		paramBlock.Add( m_bUseDocumentSun.InternalName(), BASE::ChildSlotNameFromParamName( m_bUseDocumentSun.InternalName() ),
		                m_bUseDocumentSun.LocalName(), m_bUseDocumentSun.Value(), vMin, vMax, UiHints::None, nullptr, this );
	}
	{	// Date and Time.
		CRhRdkVariant vMin, vMax;
		static const CLBPString s = RDK_DATETIME_LOCATION + CLBPString(L";") + RDK_CONTROLSET_ENABLE;
		paramBlock.Add( m_time.InternalName(), BASE::ChildSlotNameFromParamName( m_time.InternalName() ),
		                m_time.LocalName(), m_time.Value(), vMin, vMax, UiHints::None, s, this );
	}
	{	// Earth Location.
		CRhRdkVariant vMin, vMax;
		paramBlock.Add( m_vLatLong.InternalName(), BASE::ChildSlotNameFromParamName( m_vLatLong.InternalName() ),
			m_vLatLong.LocalName(), m_vLatLong.Value(), vMin, vMax, UiHints::None, RDK_CONTROLSET_ENABLE, this );
	}
	{	// Show sun.
		CRhRdkVariant vMin, vMax;
		paramBlock.Add( m_bShowSun.InternalName(), BASE::ChildSlotNameFromParamName( m_bShowSun.InternalName() ),
		                m_bShowSun.LocalName(), m_bShowSun.Value(), vMin, vMax, UiHints::None, nullptr, this );
	}
	{	// Sun size.
		CRhRdkVariant vMin = 0.0f, vMax = 1000.0f;
		paramBlock.Add( m_fSunSize.InternalName(), BASE::ChildSlotNameFromParamName( m_fSunSize.InternalName() ),
		                m_fSunSize.LocalName(), m_fSunSize.Value(), vMin, vMax, UiHints::None, nullptr, this );
	}
	{	// Sun brightness.
		CRhRdkVariant vMin = 0.0f, vMax = 1000.0f;
		paramBlock.Add( m_fSunBrightness.InternalName(), BASE::ChildSlotNameFromParamName( m_fSunBrightness.InternalName() ),
		                m_fSunBrightness.LocalName(), m_fSunBrightness.Value(), vMin, vMax, UiHints::None, nullptr, this );
	}
	{	// Sun coronal color.
		CRhRdkVariant vMin, vMax;
		paramBlock.Add( m_SunColor.InternalName(), BASE::ChildSlotNameFromParamName( m_SunColor.InternalName() ),
		                m_SunColor.LocalName(), m_SunColor.Value(), vMin, vMax, UiHints::None, RDK_COLOR_BUTTON_ALPHA, this );
	}
	{	// Atmospheric density.
		CRhRdkVariant vMin = 1.0f, vMax = 1000.0f;
		paramBlock.Add( m_fAtmosphericDensity.InternalName(), BASE::ChildSlotNameFromParamName( m_fAtmosphericDensity.InternalName() ),
		                m_fAtmosphericDensity.LocalName(), m_fAtmosphericDensity.Value(), vMin, vMax, UiHints::None, nullptr, this );
	}
	{	// Light scattering.
		CRhRdkVariant vMin = 0.0f, vMax = 50.0f;
		paramBlock.Add( m_fLightScattering.InternalName(), BASE::ChildSlotNameFromParamName( m_fLightScattering.InternalName() ),
		                m_fLightScattering.LocalName(), m_fLightScattering.Value(), vMin, vMax, UiHints::None, nullptr, this );
	}
	{	// Particle scattering (haze).
		CRhRdkVariant vMin = 1.0f, vMax = 100.0f;
		paramBlock.Add( m_fHaze.InternalName(), BASE::ChildSlotNameFromParamName( m_fHaze.InternalName() ),
		                m_fHaze.LocalName(), m_fHaze.Value(), vMin, vMax, UiHints::None, nullptr, this );
	}
	{	// Light wavelengths.
		CRhRdkVariant vMin, vMax;
		paramBlock.Add( m_Wavelengths.InternalName(), BASE::ChildSlotNameFromParamName( m_Wavelengths.InternalName() ),
		                m_Wavelengths.LocalName(), m_Wavelengths.Value(), vMin, vMax, UiHints::None, nullptr, this );
	}
	{	// Exposure.
		CRhRdkVariant vMin = 0.0f, vMax = 100.0f;
		paramBlock.Add( m_fExposure.InternalName(), BASE::ChildSlotNameFromParamName( m_fExposure.InternalName() ),
		                m_fExposure.LocalName(), m_fExposure.Value(), vMin, vMax, UiHints::None, nullptr, this );
	}
}

////////////////////////////////////////////////////////////////////////////
// Environment
////////////////////////////////////////////////////////////////////////////

class CRhRdkPhysicalSkyEnvironment : public CRhRdkPhysicalSkyBase<CRhRdkEnvironment>
{
protected:
	virtual unsigned int BitFlags(void) const override
	{
		return (CRhRdkEnvironment::BitFlags() & ~bfTextureSummary ) | bfFields;
	}

	virtual UUID TypeId(void) const override
	{
		static const GUID uuid = { 0xae8913b1, 0x1621, 0x12c5, { 0x8f, 0xd2, 0xe1, 0xf0, 0x29, 0xdd, 0x4a, 0xb4 } }; return uuid;
	}

	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual const wchar_t* Category(void) const override { return RDK_CAT_GENERAL; }
	virtual ON_wString InternalName(void) const override { return L"rdk-physical-sky-environment"; }
	virtual void SimulateEnvironment(CRhRdkSimulatedEnvironment& env, CRhRdkTexture::TextureGeneration tg, int iSize) const override;
};

////////////////////////////////////////////////////////////////////////////
// Texture
////////////////////////////////////////////////////////////////////////////

class CRhRdkPhysicalSkyTexture : public CRhRdkPhysicalSkyBase<CRhRdkTexture>
{
public:
	//Provide a constructor to create this texture from an environment - to copy over all of the state.
	//Make sure you implement this function correctly to ensure the environment version works.
	CRhRdkPhysicalSkyTexture(const CRhRdkPhysicalSkyEnvironment& env)
	{
		m_time = env.m_time.Value();
		m_bUseDocumentSun = env.m_bUseDocumentSun.Value();
	}

	CRhRdkPhysicalSkyTexture() : CRhRdkPhysicalSkyBase<CRhRdkTexture>()	{ }

	virtual bool IsHDRCapable(void) const override { return true; }

protected:
	unsigned int BitFlags(void) const override
	{
		return (CRhRdkTexture::BitFlags() & ~(bfTextureSummary | bfLocalTextureMapping) ) | bfFields;
	}

	virtual UUID TypeId(void) const override
	{
		static const GUID uuid = { 0xfd8913b1, 0x1654, 0x47c5, { 0x8f, 0xd2, 0xe1, 0xf0, 0x29, 0xdd, 0x4a, 0xb4 } }; return uuid;
	}

	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual const wchar_t* Category(void) const override { return RDK_CAT_UTILITY; }
	virtual ON_wString InternalName(void) const override { return L"rdk-physical-sky-texture"; }
	virtual void* EVF(const wchar_t* wszFunc, void* pvData) override;
	virtual EnvironmentMappingModes AutomaticEnvironmentMappingMode(void) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual CRhRdkVariant GetParameter(const wchar_t* wszParam) const override;
};
