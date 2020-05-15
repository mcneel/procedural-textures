
#include "stdafx.h"
#include "RhRdkProjectionChangingTextureBase.h"
#include "RhRcmUtilities.h"
#include "RhRdkSimulatedTexture.h"
#include "RhRdkTextureCache.h"
#include "IRhRdk_XMLSection.h"

static const wchar_t* const wszCubeMapSlots[] =
{
	L"x-positive",
	L"x-negative",
	L"y-positive",
	L"y-negative",
	L"z-positive",
	L"z-negative"
};

CRhRdkProjectionChangingTextureBase::CRhRdkProjectionChangingTextureBase()
	:
	m_sProjIn (*this, FS_TEX_PROJ_CHANGE_PROJECTION_IN , RhLocalizeString( L"Input Projection", 35405) , L"Input Projection" ),
	m_sProjOut(*this, FS_TEX_PROJ_CHANGE_PROJECTION_OUT, RhLocalizeString( L"Output Projection", 35406), L"Output Projection")
{
	SetProjectionIn (Projections::lightprobe);
	SetProjectionOut(Projections::same_as_input);
}

const wchar_t* CRhRdkProjectionChangingTextureBase::StringFromProjection(Projections proj) const
{
	switch (proj)
	{
	case Projections::planar:                   return FS_PROJECTION_PLANAR;
	case Projections::lightprobe:               return FS_PROJECTION_LIGHT_PROBE;
	case Projections::equirect:                 return FS_PROJECTION_EQUIRECT;
	case Projections::cubemap:                  return FS_PROJECTION_CUBE_MAP;
	case Projections::horizontal_cross_cubemap: return FS_PROJECTION_CUBE_MAP_HORZ;
	case Projections::vertical_cross_cubemap:   return FS_PROJECTION_CUBE_MAP_VERT;
	case Projections::emap:                     return FS_PROJECTION_EMAP;
	case Projections::hemispherical:            return FS_PROJECTION_HEMISPHERICAL;
	case Projections::same_as_input:            return FS_PROJECTION_SAME_AS_INPUT;
	}

	return L"";
}

CRhRdkProjectionChangingTextureBase::Projections CRhRdkProjectionChangingTextureBase::ProjectionFromString(const CLBPString& s) const
{
	if (FS_PROJECTION_PLANAR        == s) return Projections::planar;
	if (FS_PROJECTION_LIGHT_PROBE   == s) return Projections::lightprobe;
	if (FS_PROJECTION_EQUIRECT      == s) return Projections::equirect;
	if (FS_PROJECTION_CUBE_MAP      == s) return Projections::cubemap;
	if (FS_PROJECTION_CUBE_MAP_HORZ == s) return Projections::horizontal_cross_cubemap;
	if (FS_PROJECTION_CUBE_MAP_VERT == s) return Projections::vertical_cross_cubemap;
	if (FS_PROJECTION_EMAP          == s) return Projections::emap;
	if (FS_PROJECTION_HEMISPHERICAL == s) return Projections::hemispherical;
	if (FS_PROJECTION_SAME_AS_INPUT == s) return Projections::same_as_input;

	return Projections::none;
}

CRhRdkProjectionChangingTextureBase::Projections CRhRdkProjectionChangingTextureBase::ProjectionIn(void) const
{
	const CLBPString s = m_sProjIn.Value().AsString();
	return ProjectionFromString(s);
}

void CRhRdkProjectionChangingTextureBase::SetProjectionIn(Projections p)
{
	m_sProjIn.SetValue(StringFromProjection(p));
}

CRhRdkProjectionChangingTextureBase::Projections CRhRdkProjectionChangingTextureBase::ProjectionOut(void) const
{
	const CLBPString s = m_sProjOut.Value().AsString();
	return ProjectionFromString(s);
}

void CRhRdkProjectionChangingTextureBase::SetProjectionOut(Projections p)
{
	m_sProjOut.SetValue(StringFromProjection(p));
}

bool CRhRdkProjectionChangingTextureBase::WriteParametersToSection(IRhRdk_XMLSection& section, WriteParamsContext context) const
{
	return __super::WriteParametersToSection(section, context);
}

bool CRhRdkProjectionChangingTextureBase::ReadParametersFromSection(const IRhRdk_XMLSection& section, ReadParamsContext context)
{
	return __super::ReadParametersFromSection(section, context);
}

CRhRdkVariant CRhRdkProjectionChangingTextureBase::GetCubeFaceMapParameter(const wchar_t* wszName, int iPart) const
{
	const CRhRdkProjectionChangingTextureBase* pCopy = static_cast<const CRhRdkProjectionChangingTextureBase*>(MakeCopy());
	if (NULL == pCopy)
		return L"";

	const double sixth = 1.0 / 6.0;

	{{{	const CRhRdkContent::Change<CRhRdkProjectionChangingTextureBase> t(*pCopy);
		t().SetRepeat(ON_3dVector(sixth, 1.0, 1.0));
		t().SetOffset(ON_3dVector(iPart, 0.0, 0.0));
		t().SetRotation(ON_origin);
		t().SetProjectionOut(Projections::cubemap);
	}}}

	CRhRdkSimulatedTexture tex;
	pCopy->SimulateTexture(tex, TextureGeneration::Allow);

	delete pCopy;

	return tex.Filename().Array();
}

CRhRdkVariant CRhRdkProjectionChangingTextureBase::GetParameter(const wchar_t* wszName) const
{
	const CLBPString sName = wszName;

	int iPart = -1;

	for (int i = 0; i < 6; i++)
	{
		if (sName == wszCubeMapSlots[i])
		{
			iPart = i;
			break;
		}
	}

	if (iPart != -1)
	{
		return GetCubeFaceMapParameter(wszName, iPart);
	}

	return __super::GetParameter(wszName);
}

unsigned int CRhRdkProjectionChangingTextureBase::BitFlags(void) const
{
	return (__super::BitFlags() & ~bfTextureSummary) | bfFields;
}

CRhRdkTexture::EnvironmentMappingModes CRhRdkProjectionChangingTextureBase::AutomaticEnvironmentMappingMode(void) const
{
	using PROJ = Projections;
	using EMM = CRhRdkTexture::EnvironmentMappingModes;

	const auto proj = (ProjectionOut() == PROJ::same_as_input) ? ProjectionIn() : ProjectionOut();
	switch (proj)
	{
	case PROJ::equirect:                 return EMM::Spherical;
	case PROJ::lightprobe:               return EMM::Lightprobe;
	case PROJ::cubemap:                  return EMM::Cubemap;
	case PROJ::vertical_cross_cubemap:   return EMM::VerticalCrossCubemap;
	case PROJ::horizontal_cross_cubemap: return EMM::HorizontalCrossCubemap;
	case PROJ::emap:                     return EMM::Emap;
	case PROJ::hemispherical:            return EMM::Hemispherical;
	}

	return EMM::Emap;
}

void* CRhRdkProjectionChangingTextureBase::EVF(const wchar_t* wszFunc, void* pvData)
{
	if (_wcsicmp(L"AutomaticEnvironmentMappingMode", wszFunc) == 0)
	{
		RHRDK_ASSERT_EVF_DEPRECATED_RETURN("AutomaticEnvironmentMappingMode");
		const auto pemm = reinterpret_cast<EnvironmentMappingModes*>(pvData);
		if (NULL != pemm) *pemm = AutomaticEnvironmentMappingMode();
		return this;
	}

	return __super::EVF(wszFunc, pvData);
}

void CRhRdkProjectionChangingTextureBase::SimulateTexture(CRhRdkSimulatedTexture& texOut, CRhRdkTexture::TextureGeneration tg, int, const CRhinoObject* pObject) const
{
	CRhRdkRendererSupportOptions options;
	const int s = options.TextureSize();
	ON_2iSize size(s, s);

	const auto out = ProjectionOut();
	switch (out)
	{
	case Projections::equirect:                 size.cx *= 2;               break;
	case Projections::cubemap:                  size.cx *= 6;               break;
	case Projections::vertical_cross_cubemap:   size.cx *= 3; size.cy *= 4; break;
	case Projections::horizontal_cross_cubemap: size.cx *= 4; size.cy *= 3; break;
	case Projections::hemispherical:            size.cx *= 4;               break;
	}

	// Store the offset and repeat values since we can put these directly into the simulation.
	const ON_3dVector vOriginalOffset = Offset();
	const ON_3dVector vOriginalRepeat = Repeat();
	const ON_3dVector vOriginalRotation = Rotation();

	// Equirect / Aziumuth optimization.
	const auto vOriginalAzimuth  = GetParameter(L"azimuth");
	bool bEquiHack = false;

	const CRhRdkContent::Change<CRhRdkProjectionChangingTextureBase> t(*this);

	t().SetOffset(ON_3dVector(0.0, 0.0, 0.0));
	t().SetRepeat(ON_3dVector(1.0, 1.0, 1.0));
	t().SetRotation(ON_3dVector(0.0, 0.0, 0.0));

	if ((Projections::equirect == out) && !vOriginalAzimuth.IsNull())
	{
		bEquiHack = true;
		t().SetParameter(L"azimuth", 0.0);
	}

	// Added the 1 to ensure that these images are recalculated for Jeff
	// Since he's unlikely to want to bother with resetting the cache.
	const auto crc = ComputeRenderCRC(CRenderCRCFlags::rcrc_ForSimulation) + 1;
	CRhRdkTextureCache tc(size, crc);

	if (TextureGeneration::Allow == tg)
	{
		if (!tc.HasCachedTexture() && !DuplicationInProgress())
		{
			RHRDK_ASSERT((size.cx > 0) && (size.cy > 0));

			if ((size.cx > 0) && (size.cy > 0))
			{
				CRhinoDib dib(size.cx, size.cy, 32);
				if (GenerateTextureSimulation(dib, IRhRdkTextureEvaluator::CEvalFlags::efDisableLocalMapping))
				{
					tc.AddTexture(dib);
				}
			}
		}
	}

	t().SetOffset(vOriginalOffset);
	t().SetRepeat(vOriginalRepeat);
	t().SetRotation(vOriginalRotation);

	texOut.SetFilename(tc.Filename(), DocumentAssoc(), TextureGeneration::Allow == tg);
	texOut.SetRepeat(vOriginalRepeat.x, vOriginalRepeat.y);
	texOut.SetRotation(vOriginalRotation.z);
	texOut.SetWrapType(WrapTypes::Repeating);
	texOut.SetMappingAndProjection(ProjectionMode(), MappingChannel(), EnvironmentMappingMode());

	if (bEquiHack)
	{
		t().SetParameter(L"azimuth",  vOriginalAzimuth);

		const double dAzimuth = vOriginalAzimuth.AsDouble();
		const double dAzimuthDegrees = LBP_RadiansToDegrees(dAzimuth);
		texOut.SetOffset(vOriginalOffset.x - (dAzimuthDegrees / 360.0), vOriginalOffset.y);
	}
	else
	{
		texOut.SetOffset(vOriginalOffset.x, vOriginalOffset.y);
	}
}
