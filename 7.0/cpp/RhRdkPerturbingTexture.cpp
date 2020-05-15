
#include "stdafx.h"
#include "RhRdkPerturbingTexture.h"
#include "RhRcm.h"

CRhRdkPerturbingTexture::CRhRdkPerturbingTexture()
	:
	m_dAmount(*this, FS_TEX_PERTURBING_AMOUNT, RhLocalizeString( L"Amount", 33752), L"Amount")
{
	m_dAmount = 0.1;
	m_dAmount.SetLimits(0.0, CRhRdkVariant::Null());
}

CRhRdkPerturbingTexture::~CRhRdkPerturbingTexture()
{
}

double CRhRdkPerturbingTexture::Amount(void) const
{
	return m_dAmount;
}

void CRhRdkPerturbingTexture::SetAmount(double d)
{
	m_dAmount.SetValue(d);
}

UUID CRhRdkPerturbingTexture::RenderEngineId(void) const
{
	return uuidUniversalRenderEngine;
}

UUID CRhRdkPerturbingTexture::PlugInId(void) const
{
	return rdk.RhinoPlugInUuid();
}

UUID CRhRdkPerturbingTexture::TypeId(void) const
{
	return uuidPerturbingTextureType;
}

ON_wString CRhRdkPerturbingTexture::InternalName(void) const
{
	return L"rdk-perturbing-texture";
}

ON_wString CRhRdkPerturbingTexture::TypeName(void) const
{
	return RhLocalizeString( L"Perturbing Texture", 23368);
}

ON_wString CRhRdkPerturbingTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Use to make one texture perturb another", 23369);
}

const wchar_t* CRhRdkPerturbingTexture::Category(void) const
{
	return RDK_CAT_PROCEDURAL_3D;
}

bool CRhRdkPerturbingTexture::SetParameter(const wchar_t* wszName, const CRhRdkVariant& value)
{
	return __super::SetParameter(wszName, value);
}

bool CRhRdkPerturbingTexture::WriteParametersToSection(IRhRdk_XMLSection& section, WriteParamsContext context) const
{
	return __super::WriteParametersToSection(section, context);
}

bool CRhRdkPerturbingTexture::ReadParametersFromSection(const IRhRdk_XMLSection& section, ReadParamsContext context)
{
	return __super::ReadParametersFromSection(section, context);
}

bool CRhRdkPerturbingTexture::IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const
{
	return f.IsKind(CRhRdkContent::Kinds::Texture);
}

unsigned int CRhRdkPerturbingTexture::ComputeRenderCRC(const CRenderCRCFlags& rcrcFlags) const
{
	return __super::ComputeRenderCRC(rcrcFlags) ^ GetCompileDateMash();
}

class CRhRdkPerturbingTextureCSI : public CRhRdkContent::CChildSlotIterator
{
public:
	CRhRdkPerturbingTextureCSI(const CRhRdkContent* pContent, CRhRdkContent::CSIContext context)
		: CRhRdkContent::CChildSlotIterator(pContent, context) { m_iIndex = -1; }

	virtual bool NextChildSlot(ON_wString& sParamNameOut, ON_wString& sChildSlotNameOut, ON_wString& sDisplayNameOut);

protected:
	int m_iIndex;
};

CRhRdkContent::CChildSlotIterator* CRhRdkPerturbingTexture::NewChildSlotIterator(CSIContext context) const
{
	return new CRhRdkPerturbingTextureCSI(this, context);
}

bool CRhRdkPerturbingTextureCSI::NextChildSlot(ON_wString& sParamNameOut, ON_wString& sChildSlotNameOut, ON_wString& sDisplayNameOut)
{
	bool bSuccess = false;

	ON_wString s;

	m_iIndex++;

	if (0 == m_iIndex)
	{
		sChildSlotNameOut = CS_TEX_PERTURBING_SOURCE_TEXTURE;
		s = RhLocalizeString( L"SourceTexture", 23370);
		bSuccess = true;
	}
	else
	if (1 == m_iIndex)
	{
		sChildSlotNameOut = CS_TEX_PERTURBING_PERTURB_TEXTURE;
		s = RhLocalizeString( L"PerturbTexture", 23371);
		bSuccess = true;
	}

	if (bSuccess)
	{
		sDisplayNameOut = s;
		sParamNameOut = m_pContent->ParamNameFromChildSlotName(sChildSlotNameOut.Array());
	}

	return bSuccess;
}


class CRhRdkPerturbingTextureEvaluator : public CRhRdkTextureEvaluator
{
public:
	CRhRdkPerturbingTextureEvaluator(double amount, IRhRdkTextureEvaluator* pSourceTE, IRhRdkTextureEvaluator* pPerturbTE, const ON_Xform& xform, CEvalFlags ef)
		:
		m_xform(xform),
		m_amount(amount),
		m_pSourceTE(pSourceTE),
		m_pPerturbTE(pPerturbTE),
		CRhRdkTextureEvaluator(ef)
	{
		m_sourceChildIndex = -1;
		m_perturbChildIndex = -1;
	}

	virtual ~CRhRdkPerturbingTextureEvaluator()
	{
		SAFE_DELETE_THIS(m_pSourceTE);
		SAFE_DELETE_THIS(m_pPerturbTE);
	}

	bool GetPerturbColor(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData) const;
	bool GetSourceColor(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData) const;

	virtual void DeleteThis(void) override { delete this; }
	virtual bool GetColor(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData = nullptr) const override;
	virtual void* EVF(const wchar_t*, void*) override { return nullptr; }

	virtual bool RegisterChildCallback(IChildCallback* pChildCallback, const CRhRdkTexture& texture, void* pvData);

protected:
	const ON_Xform m_xform;
	const double m_amount;
	IRhRdkTextureEvaluator* m_pSourceTE;
	IRhRdkTextureEvaluator* m_pPerturbTE;

	int m_sourceChildIndex;
	int m_perturbChildIndex;
};

bool CRhRdkPerturbingTextureEvaluator::GetPerturbColor(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData) const
{
	const IChildCallback* pChildCallback = ChildCallback();
	if (pChildCallback && -1 != m_perturbChildIndex)
	{
		if (pChildCallback->GetChildColor(m_perturbChildIndex, pvData, uvw, duvwdx, duvwdy, colOut))
			return true;
	}

	if (nullptr != m_pPerturbTE)
	{
		if (m_pPerturbTE->GetColor(uvw, duvwdx, duvwdy, colOut))
			return true;
	}

	// Default perturb color to black if there is no texture in the child slots.
	// This allows for the source texture to still appear if there is no perturb texture
	colOut.Set(0.0, 0.0, 0.0);

	return true;
}

bool CRhRdkPerturbingTextureEvaluator::GetSourceColor(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData) const
{
	const IChildCallback* pChildCallback = ChildCallback();
	if (pChildCallback && -1 != m_sourceChildIndex)
	{
		if (pChildCallback->GetChildColor(m_sourceChildIndex, pvData, uvw, duvwdx, duvwdy, colOut))
			return true;
	}

	if (nullptr != m_pSourceTE)
	{
		if (m_pSourceTE->GetColor(uvw, duvwdx, duvwdy, colOut))
			return true;
	}

	// Default source color to red if there is no texture in the child slots
	colOut.Set(1.0, 0.0, 0.0);

	return true;
}

bool CRhRdkPerturbingTextureEvaluator::GetColor(const ON_3dPoint& uvwOriginal, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData) const
{
	const ON_3dPoint uvw = m_xform * uvwOriginal;

	CRhRdkColor color(0, 0, 0, 0);

	if (GetPerturbColor(uvw, duvwdx, duvwdy, color, pvData))
	{
		const double xs = (double)(color.FRed() + color.FGreen() + color.FBlue()) / 1.5 * m_amount - m_amount;
		if (GetPerturbColor(ON_xaxis - uvw, duvwdx, duvwdy, color, pvData))
		{
			const double ys = (double)(color.FRed() + color.FGreen() + color.FBlue()) / 1.5 * m_amount - m_amount;
			if (GetPerturbColor(ON_yaxis - uvw, duvwdx, duvwdy, color, pvData))
			{
				const double zs = (double)(color.FRed() + color.FGreen() + color.FBlue()) / 1.5 * m_amount - m_amount;

				return GetSourceColor(uvw + ON_3dPoint(xs, ys, zs), duvwdx, duvwdy, colOut, pvData);
			}
		}
	}

	// This should never be reached
	return false;
}

bool CRhRdkPerturbingTextureEvaluator::RegisterChildCallback(IRhRdkTextureEvaluator::IChildCallback* pChildCallback, const CRhRdkTexture& texture, void* pvData)
{
	if (!CRhRdkTextureEvaluator::RegisterChildCallback(pChildCallback, texture, pvData))
		return false;

	const CRhRdkContent* pPerturbChild = texture.FindChild(CS_TEX_PERTURBING_PERTURB_TEXTURE);
	if (nullptr != pPerturbChild)
	{
		m_perturbChildIndex = pChildCallback->InitializeChild(pPerturbChild, pvData);
	}

	const CRhRdkContent* pSourceChild = texture.FindChild(CS_TEX_PERTURBING_SOURCE_TEXTURE);
	if (nullptr != pSourceChild)
	{
		m_sourceChildIndex = pChildCallback->InitializeChild(pSourceChild, pvData);
	}

	return true;
}

IRhRdkTextureEvaluator* CRhRdkPerturbingTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	IRhRdkTextureEvaluator* pSourceTE = nullptr;
	IRhRdkTextureEvaluator* pPerturbTE = nullptr;

	const CRhRdkTexture* pSource = dynamic_cast<const CRhRdkTexture*>(FindChild(CS_TEX_PERTURBING_SOURCE_TEXTURE));
	if (nullptr != pSource)
	{
		pSourceTE = pSource->NewTextureEvaluator(ef);
	}

	const CRhRdkTexture* pPerturb = dynamic_cast<const CRhRdkTexture*>(FindChild(CS_TEX_PERTURBING_PERTURB_TEXTURE));
	if (nullptr != pPerturb)
	{
		pPerturbTE = pPerturb->NewTextureEvaluator(ef);
	}

	return new CRhRdkPerturbingTextureEvaluator(m_dAmount, pSourceTE, pPerturbTE, LocalMappingTransform(), ef);
}

unsigned int CRhRdkPerturbingTexture::BitFlags(void) const
{
	return (__super::BitFlags() & ~bfTextureSummary) | bfFields;
}

CRhRdkContent* CRhRdkPerturbingTexture::MakeCopy(CopyMethods m) const
{
	CRhRdkContent* pCopy = FastMakeCopy(m);

#ifdef _DEBUG
	VerifyCopy(pCopy);
#endif

	return pCopy;
}
