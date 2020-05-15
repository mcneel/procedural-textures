
#include "stdafx.h"
#include "RhRdkProjectionChangerTexture.h"
#include "RhRdkTextureUtilities.h"
#include "RhRcm.h"

CRhRdkProjectionChangerTexture::CRhRdkProjectionChangerTexture()
	:
	m_azimuth (*this, FS_TEX_PROJ_CHANGE_AZIMUTH , RhLocalizeString( L"Azimuth", 33753) , L"Azimuth" ),
	m_altitude(*this, FS_TEX_PROJ_CHANGE_ALTITUDE, RhLocalizeString( L"Altitude", 33754), L"Altitude")
{
	SetProjectionIn (Projections::equirect);
	SetProjectionOut(Projections::same_as_input);

	m_azimuth = 0.0;
	m_azimuth.SetLimits(LBP_DegreesToRadians(-360.0), LBP_DegreesToRadians(360.0));
	m_azimuth.SetConversion(CRhRdkContentField::conv_radians);

	m_altitude = 0.0;
	m_altitude.SetLimits(LBP_DegreesToRadians(-90.0), LBP_DegreesToRadians(90.0));
	m_altitude.SetConversion(CRhRdkContentField::conv_radians);
}

double CRhRdkProjectionChangerTexture::Azimuth(void) const
{
	return m_azimuth;
}

void CRhRdkProjectionChangerTexture::SetAzimuth(double d)
{
	m_azimuth.SetValue(d);
}

double CRhRdkProjectionChangerTexture::Altitude() const
{
	return m_altitude;
}

void CRhRdkProjectionChangerTexture::SetAltitude(double d)
{
	m_altitude.SetValue(d);
}

UUID CRhRdkProjectionChangerTexture::RenderEngineId(void) const
{
	return uuidUniversalRenderEngine;
}

UUID CRhRdkProjectionChangerTexture::PlugInId(void) const
{
	return rdk.RhinoPlugInUuid();
}

UUID CRhRdkProjectionChangerTexture::TypeId(void) const
{
	return uuidProjectionChangerTextureType;
}

ON_wString CRhRdkProjectionChangerTexture::InternalName(void) const
{
	return L"rdk-projection-changer-texture";
}

ON_wString CRhRdkProjectionChangerTexture::TypeName(void) const
{
	return RhLocalizeString( L"Projection Changer Texture", 23385);
}

ON_wString CRhRdkProjectionChangerTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Changes projection of a texture.", 23386);
}

const wchar_t* CRhRdkProjectionChangerTexture::Category(void) const
{
	return RDK_CAT_GENERAL;
}

bool CRhRdkProjectionChangerTexture::SetParameter(const wchar_t* wszName, const CRhRdkVariant& value)
{
	return __super::SetParameter(wszName, value);
}

bool CRhRdkProjectionChangerTexture::WriteParametersToSection(IRhRdk_XMLSection& section, WriteParamsContext context) const
{
	return __super::WriteParametersToSection(section, context);
}

bool CRhRdkProjectionChangerTexture::ReadParametersFromSection(const IRhRdk_XMLSection& section, ReadParamsContext context)
{
	return __super::ReadParametersFromSection(section, context);
}

bool CRhRdkProjectionChangerTexture::IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const
{
	return f.IsKind(CRhRdkContent::Kinds::Texture);
}

unsigned int CRhRdkProjectionChangerTexture::BitFlags(void) const
{
	return __super::BitFlags() & ~bfTextureSummary;
}

CRhRdkContent* CRhRdkProjectionChangerTexture::MakeCopy(CopyMethods m) const
{
	auto* pCopy = FastMakeCopy(m);

#ifdef _DEBUG
	VerifyCopy(pCopy);
#endif

	return pCopy;
}

class CRhRdkProjectionChangerTextureCSI : public CRhRdkContent::CChildSlotIterator
{
public:
	CRhRdkProjectionChangerTextureCSI(const CRhRdkContent* pContent, CRhRdkContent::CSIContext context)
		: CRhRdkContent::CChildSlotIterator(pContent, context) { m_iIndex = -1; }

	virtual bool NextChildSlot(ON_wString& sParamNameOut, ON_wString& sChildSlotNameOut, ON_wString& sDisplayNameOut);

protected:
	int m_iIndex;
};

CRhRdkContent::CChildSlotIterator* CRhRdkProjectionChangerTexture::NewChildSlotIterator(CSIContext context) const
{
	return new CRhRdkProjectionChangerTextureCSI(this, context);
}

bool CRhRdkProjectionChangerTextureCSI::NextChildSlot(ON_wString& sParamNameOut, ON_wString& sChildSlotNameOut, ON_wString& sDisplayNameOut)
{
	bool bSuccess = false;

	m_iIndex++;

	if (0 == m_iIndex)
	{
		sChildSlotNameOut = CS_TEX_PROJ_CHANGE_INPUT_TEXTURE;
		bSuccess = true;
	}

	if (bSuccess)
	{
		sDisplayNameOut = RhLocalizeString( L"InputTexture", 23387);
		sParamNameOut = m_pContent->ParamNameFromChildSlotName(sChildSlotNameOut.Array());
	}

	return bSuccess;
}

class CRhRdkProjectionChangerTextureEvaluator : public CRhRdkTextureEvaluator
{
public:
	CRhRdkProjectionChangerTextureEvaluator(const CRhRdkProjectionChangerTexture& texture, CEvalFlags ef)
		: CRhRdkTextureEvaluator(ef)
	{
		m_inputChildIndex = -1;

		m_pInputTE = nullptr;

		const CRhRdkContent* pInput = texture.FindChild(CS_TEX_PROJ_CHANGE_INPUT_TEXTURE);
		const CRhRdkTexture* pTexture = dynamic_cast<const CRhRdkTexture*>(pInput);
		if (pTexture != nullptr)
		{
			m_pInputTE = pTexture->NewTextureEvaluator(ef);
		}

		m_inputProjectionType = texture.ProjectionIn();
		m_outputProjectionType = ProjectionChangeOn() ? texture.ProjectionOut() : texture.ProjectionIn();
		m_fAzimuth  = (float)texture.Azimuth();
		m_fAltitude = (float)texture.Altitude();
		m_xform = LocalMappingOn() ? texture.LocalMappingTransform() : ON_Xform(1);
	}

	virtual ~CRhRdkProjectionChangerTextureEvaluator()
	{
		SAFE_DELETE_THIS(m_pInputTE);
	}

	virtual void DeleteThis(void) override { delete this; }
	virtual bool GetColor(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData = nullptr) const override;
	virtual void* EVF(const wchar_t*, void*) override { return nullptr; }

	virtual bool RegisterChildCallback(IChildCallback* pChildCallback, const CRhRdkTexture& texture, void* pvData);

protected:
	ON_Xform m_xform;
	CRhRdkProjectionChangerTexture::Projections m_inputProjectionType;
	CRhRdkProjectionChangerTexture::Projections m_outputProjectionType;
	float m_fAzimuth;
	float m_fAltitude;
	int m_inputChildIndex;
	IRhRdkTextureEvaluator* m_pInputTE;
};

bool CRhRdkProjectionChangerTextureEvaluator::GetColor(const ON_3dPoint& uvwOriginal, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData) const
{
	using PROJ = PCTB::Projections;

	bool bOk = true;

	const auto outputProjectionType =
		m_outputProjectionType == PROJ::same_as_input ? m_inputProjectionType : m_outputProjectionType;

	ON_3dPoint uvw(uvwOriginal);
	uvw.Transform(m_xform);
	NormalizeUVW(uvw);

	float s = (float)uvw.x;
	float t = (float)uvw.y;

	bool bQuick = false;

	if (m_inputProjectionType == PROJ::planar)
		bQuick = true;
	else
	if ((outputProjectionType == m_inputProjectionType) && (m_fAzimuth == 0.0f) && (m_fAltitude == 0.0f))
		bQuick = true;

	if (!bQuick)
	{
		ON_3fVector vec(0.0f, 0.0f, 0.0f);

		switch (outputProjectionType)
		{
			case PROJ::lightprobe:
				bOk = LightProbeToWorld(s, t, vec);
				break;
			case PROJ::equirect:
				EquirectToWorld(s, t, vec);
				break;
			case PROJ::cubemap:
				CubemapToWorld(s, t, vec);
				break;
			case PROJ::vertical_cross_cubemap: 
				bOk = VerticalCrossCubemapToWorld(s, t, vec);
				break;
			case PROJ::horizontal_cross_cubemap: 
				bOk = HorizontalCrossCubemapToWorld(s, t, vec);
				break;
			case PROJ::emap: 
				bOk = EmapToWorld(s, t, vec);
				break;
			case PROJ::hemispherical:
				HemisphericalToWorld(s, t, vec);
				break;
		}

		if (bOk)
		{
			ON_3dVector v = vec;
			const ON_3dVector rotate_axis = ON_yaxis;
			v.Rotate(m_fAzimuth,  rotate_axis);
		
			if (!v.IsParallelTo(rotate_axis))
			{
				ON_3dVector cross_axis = ON_CrossProduct(v, rotate_axis);
				cross_axis.Unitize();
				v.Rotate(m_fAltitude, cross_axis);
			}
			vec = v;

			switch (m_inputProjectionType)
			{
				case PROJ::lightprobe:
					WorldToLightProbe(vec, s, t);
					break;
				case PROJ::equirect:
					WorldToEquirect(vec, s, t);
					break;
				case PROJ::cubemap:
					WorldToCubemap(vec, s, t);
					break;
				case PROJ::vertical_cross_cubemap:
					WorldToVerticalCrossCubemap(vec, s, t);
					break;
				case PROJ::horizontal_cross_cubemap:
					WorldToHorizontalCrossCubemap(vec, s, t);
					break;
				case PROJ::emap:
					WorldToEmap(vec, s, t);
					break;
				case PROJ::hemispherical:
					WorldToHemispherical(vec, s, t);
					break;
			}
		}
	}

	if (bOk)
	{
		bool bColorGot = false;
		const auto* pChildCallback = ChildCallback();
		if (pChildCallback && -1 != m_inputChildIndex)
		{
			if (pChildCallback->GetChildColor(m_inputChildIndex, pvData, ON_3dPoint(s, t, uvw.z), duvwdx, duvwdy, colOut))
				bColorGot = true;
		}

		if (!bColorGot)
		{
			if (nullptr != m_pInputTE)
			{
				if (m_pInputTE->GetColor(ON_3dPoint(s, t, uvw.z), duvwdx, duvwdy, colOut))
					bColorGot = true;
			}
		}

		bOk = bColorGot;
	}

	if (!bOk)
	{
		// This is not an error condition.
		colOut.Set(0.0, 0.0, 0.0);
	}

	return true;
}

bool CRhRdkProjectionChangerTextureEvaluator::RegisterChildCallback(IRhRdkTextureEvaluator::IChildCallback* pChildCallback, const CRhRdkTexture& texture, void* pvData)
{
	if (!CRhRdkTextureEvaluator::RegisterChildCallback(pChildCallback, texture, pvData))
		return false;

	const auto* pChild = texture.FindChild(CS_TEX_PROJ_CHANGE_INPUT_TEXTURE);
	if (nullptr != pChild)
	{
		m_inputChildIndex = pChildCallback->InitializeChild(pChild, pvData);
	}

	return true;
}

IRhRdkTextureEvaluator* CRhRdkProjectionChangerTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new CRhRdkProjectionChangerTextureEvaluator(*this, ef);
}
