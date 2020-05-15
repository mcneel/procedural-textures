
#include "stdafx.h"
#include "RhRdkMaskTexture.h"
#include "RhRcm.h"

CRhRdkMaskTexture::CRhRdkMaskTexture()
	:
	m_sMaskType(*this, FS_TEX_MASK_TYPE, RhLocalizeString( L"Mask Type", 33735), L"Mask Type")
{
	m_sMaskType = FS_TEX_MASK_TYPE_LUMINANCE;
}

CRhRdkMaskTexture::MaskTypes CRhRdkMaskTexture::MaskType(void) const
{
	const CLBPString s = m_sMaskType.Value().AsString();

	if (FS_TEX_MASK_TYPE_RED   == s) return MaskTypes::Red;
	if (FS_TEX_MASK_TYPE_GREEN == s) return MaskTypes::Green;
	if (FS_TEX_MASK_TYPE_BLUE  == s) return MaskTypes::Blue;
	if (FS_TEX_MASK_TYPE_ALPHA == s) return MaskTypes::Alpha;

	return MaskTypes::Luminance;
}

void CRhRdkMaskTexture::SetMaskType(MaskTypes type)
{
	const wchar_t* wsz = FS_TEX_MASK_TYPE_LUMINANCE;
	switch (type)
	{
	case MaskTypes::Red:   wsz = FS_TEX_MASK_TYPE_RED;   break;
	case MaskTypes::Green: wsz = FS_TEX_MASK_TYPE_GREEN; break;
	case MaskTypes::Blue:  wsz = FS_TEX_MASK_TYPE_BLUE;  break;
	case MaskTypes::Alpha: wsz = FS_TEX_MASK_TYPE_ALPHA; break;
	}

	m_sMaskType.SetValue(wsz);
}

UUID CRhRdkMaskTexture::RenderEngineId(void) const
{
	return uuidUniversalRenderEngine;
}

UUID CRhRdkMaskTexture::PlugInId(void) const
{
	return rdk.RhinoPlugInUuid();
}

UUID CRhRdkMaskTexture::TypeId(void) const
{
	return uuidMaskTextureType;
}

ON_wString CRhRdkMaskTexture::InternalName(void) const
{
	return L"rdk-mask-texture";
}

ON_wString CRhRdkMaskTexture::TypeName(void) const
{
	return RhLocalizeString( L"Mask Texture", 23333);
}

ON_wString CRhRdkMaskTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Creates masks from textures.", 23334);
}

const wchar_t* CRhRdkMaskTexture::Category(void) const
{
	return RDK_CAT_PROCEDURAL_2D;
}

bool CRhRdkMaskTexture::WriteParametersToSection(IRhRdk_XMLSection& section, WriteParamsContext context) const
{
	return __super::WriteParametersToSection(section, context);
}

bool CRhRdkMaskTexture::ReadParametersFromSection(const IRhRdk_XMLSection& section, ReadParamsContext context)
{
	return __super::ReadParametersFromSection(section, context);
}

bool CRhRdkMaskTexture::IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const
{
	return f.IsKind(CRhRdkContent::Kinds::Texture);
}

CRhRdkContent* CRhRdkMaskTexture::MakeCopy(CopyMethods m) const
{
	CRhRdkContent* pCopy = FastMakeCopy(m);

#ifdef _DEBUG
	VerifyCopy(pCopy);
#endif

	return pCopy;
}

class CMaskTextureEvaluator : public CRhRdkTextureEvaluator
{
public:
	CMaskTextureEvaluator(const ON_Xform& xform, CRhRdkMaskTexture::MaskTypes maskType, IRhRdkTextureEvaluator* pSourceTE, CEvalFlags ef) 
		: 
		m_xform(xform), 
		m_maskType(maskType), 
		m_pSourceTE(pSourceTE),
		CRhRdkTextureEvaluator(ef)
	{
	}

	virtual ~CMaskTextureEvaluator() 
	{
		SAFE_DELETE_THIS(m_pSourceTE);
	}

	virtual void DeleteThis(void) override { delete this; }
	virtual bool GetColor(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData = nullptr) const override;
	virtual void* EVF(const wchar_t*, void*) override { return nullptr; }

	virtual bool RegisterChildCallback(IChildCallback* pChildCallback, const CRhRdkTexture& texture, void* pvData) override;

protected:

	const ON_Xform m_xform;
	const CRhRdkMaskTexture::MaskTypes m_maskType;
	IRhRdkTextureEvaluator* m_pSourceTE;
	int m_inputChildIndex = -1;
};

bool CMaskTextureEvaluator::GetColor(const ON_3dPoint& uvwOriginal, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData) const
{
	const ON_3dPoint uvw = m_xform * uvwOriginal;

	bool bColorGot = false;
	const IChildCallback* pChildCallback = ChildCallback();
	if (pChildCallback && -1 != m_inputChildIndex)
	{
		const bool bRet = pChildCallback->GetChildColor(m_inputChildIndex, pvData, uvw, duvwdx, duvwdy, colOut);
		if (bRet)
			bColorGot = true;
	}

	if (!bColorGot)
	{
		if (nullptr != m_pSourceTE)
		{
			if (m_pSourceTE->GetColor(uvw, duvwdx, duvwdy, colOut))
				bColorGot = true;
		}
	}

	if (bColorGot)
	{
		float alphaValue = 0.5;
		switch (m_maskType)
		{
		case CRhRdkMaskTexture::MaskTypes::Luminance: alphaValue = colOut.Luminance(); break;
		case CRhRdkMaskTexture::MaskTypes::Red:       alphaValue = colOut.FRed();      break;
		case CRhRdkMaskTexture::MaskTypes::Green:     alphaValue = colOut.FGreen();    break;
		case CRhRdkMaskTexture::MaskTypes::Blue:      alphaValue = colOut.FBlue();     break;
		case CRhRdkMaskTexture::MaskTypes::Alpha:     alphaValue = colOut.FAlpha();    break;
		}
		colOut.Set(alphaValue, alphaValue, alphaValue, 1.0f);

		return true;
	}

	return false;
}

bool CMaskTextureEvaluator::RegisterChildCallback(IRhRdkTextureEvaluator::IChildCallback* pChildCallback, const CRhRdkTexture& texture, void* pvData)
{
	if (!CRhRdkTextureEvaluator::RegisterChildCallback(pChildCallback, texture, pvData))
		return false;

	const CRhRdkContent* pChild = texture.FindChild(CS_TEX_MASK_INPUT_TEXTURE);
	if (nullptr != pChild)
	{
		m_inputChildIndex = pChildCallback->InitializeChild(pChild, pvData);
	}

	return true;
}

IRhRdkTextureEvaluator* CRhRdkMaskTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	IRhRdkTextureEvaluator* pSourceTE = nullptr;

	const CRhRdkTexture* pTexture = dynamic_cast<const CRhRdkTexture*>(FindChild(CS_TEX_MASK_INPUT_TEXTURE));
	if (nullptr != pTexture)
	{
		pSourceTE = pTexture->NewTextureEvaluator(ef);
	}

	return new CMaskTextureEvaluator(LocalMappingTransform(), MaskType(), pSourceTE, ef);
}

unsigned int CRhRdkMaskTexture::BitFlags(void) const
{
	return (__super::BitFlags() & ~(bfTextureSummary | bfLocalTextureMapping)) | bfFields;
}

unsigned int CRhRdkMaskTexture::ComputeRenderCRC(const CRenderCRCFlags& rcrcFlags) const
{
	return __super::ComputeRenderCRC(rcrcFlags) ^ GetCompileDateMash();
}

class CRhRdkMaskTextureCSI : public CRhRdkContent::CChildSlotIterator
{
public:
	CRhRdkMaskTextureCSI(const CRhRdkContent* pContent, CRhRdkContent::CSIContext context)
		: CRhRdkContent::CChildSlotIterator(pContent, context) { m_iIndex = -1; }

	virtual bool NextChildSlot(ON_wString& sParamNameOut, ON_wString& sChildSlotNameOut, ON_wString& sDisplayNameOut);
protected:
	int m_iIndex;
};

CRhRdkContent::CChildSlotIterator* CRhRdkMaskTexture::NewChildSlotIterator(CSIContext context) const
{
	return new CRhRdkMaskTextureCSI(this, context);
}

bool CRhRdkMaskTextureCSI::NextChildSlot(ON_wString& sParamNameOut, ON_wString& sChildSlotNameOut, ON_wString& sDisplayNameOut)
{
	bool bSuccess = false;

	m_iIndex++;

	if (0 == m_iIndex)
	{
		sChildSlotNameOut = CS_TEX_MASK_INPUT_TEXTURE;
		bSuccess = true;
	}

	if (bSuccess)
	{
		sDisplayNameOut = RhLocalizeString( L"InputTexture", 23335);
		sParamNameOut = m_pContent->ParamNameFromChildSlotName(sChildSlotNameOut.Array());
	}

	return bSuccess;
}


bool CRhRdkMaskTexture::PixelSize(int& widthOut, int& heightOut, int& depthOut) const
{
	//http://mcneel.myjetbrains.com/youtrack/issue/RH-32542
	//Mask textures are often used with bitmap sources.  This makes the size data pass through.
	const CRhRdkTexture* pChild = dynamic_cast<const CRhRdkTexture*>(FindChild(CS_TEX_MASK_INPUT_TEXTURE));
	if (pChild)
	{
		return pChild->PixelSize(widthOut, heightOut, depthOut);
	}
	return false;
}
