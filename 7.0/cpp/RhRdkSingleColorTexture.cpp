
#include "stdafx.h"
#include "RhRdkSingleColorTexture.h"
#include "RhRdkContentImpl.h"

class CRhRdkSingleColorTexture::Evaluator : public CRhRdkOneColorEvaluator
{
public:
	Evaluator(const CRhRdkSingleColorTexture& texture, CEvalFlags ef);

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const;
};

CRhRdkSingleColorTexture::Evaluator::Evaluator(const CRhRdkSingleColorTexture& texture, CEvalFlags ef)
	:
	CRhRdkOneColorEvaluator(texture, ef)
{
}

CRhRdkColor CRhRdkSingleColorTexture::Evaluator::GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const
{
	return OutputColor(1, uvw, duvwdx, duvwdy, pvData);
}

CRhRdkSingleColorTexture::CRhRdkSingleColorTexture()
	:
	m_bUseObjectColor(*this, FS_TEX_SINGLE_COLOR_USE_OBJECT_COLOR, RhLocalizeString( L"Use object color", 35374), L"Use object color")
{
	Imp().AddToPreventInitAssert(+1);

	Register();

	m_bUseObjectColor = false;

	Imp().AddToPreventInitAssert(-1);
}

CRhRdkSingleColorTexture::~CRhRdkSingleColorTexture()
{
	UnRegister();
}

UUID CRhRdkSingleColorTexture::TypeId(void) const
{
	return uuidSingleColorTextureType;
}

ON_wString CRhRdkSingleColorTexture::TypeName(void) const
{
	return RhLocalizeString( L"Single Color Texture", 23502);
}

ON_wString CRhRdkSingleColorTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Single color texture", 23503);
}

ON_wString CRhRdkSingleColorTexture::InternalName(void) const
{
	return L"rcm-single-color-texture";
}

IRhRdkTextureEvaluator* CRhRdkSingleColorTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	if (m_bUseObjectColor.Value().AsSmartBool())
		return NULL;

	return new Evaluator(*this, ef);
}

void CRhRdkSingleColorTexture::SimulateTexture(CRhRdkSimulatedTexture& texOut, CRhRdkTexture::TextureGeneration tg, int iSize, const CRhinoObject* pObject) const
{
	if (!m_bUseObjectColor.Value().AsSmartBool())
	{
		__super::SimulateTexture(texOut, tg, iSize, pObject);
	}
	else
	{
		CRhRdkColor col(128, 128, 128, 255);

		const CRhinoObject* pRhinoObject = dynamic_cast<const CRhinoObject*>(pObject);
		if (NULL != pRhinoObject)
		{
			if (pRhinoObject->Attributes().ColorSource() != ON::color_from_parent) // color_from_parent is not supported.
			{
				col = pRhinoObject->ObjectDrawColor(true);
				col.SetAlpha(1.0f);
			}
		}

		const ON_2iSize size(1, 1);

		const DWORD dwCRC = col.ColorRefAlpha();
		CRhRdkTextureCache cache(size, dwCRC);
		if (!cache.HasCachedTexture())
		{
			CRhinoDib dib(size.cx, size.cy, 32);
			dib.SetPixel(0L, 0L, col.FRed(), col.FGreen(), col.FBlue(), col.FAlpha());
			cache.AddTexture(dib);
		}

		texOut.SetFilename(cache.Filename(), DocumentAssoc());
	}
}

void CRhRdkSingleColorTexture::OnFieldChanged(const CRhRdkContentField& field, const CRhRdkVariant& vOldValue, RhRdkChangeContext cc, void* pReserved)
{
	if (field.InternalName() == FS_TEX_SINGLE_COLOR_USE_OBJECT_COLOR)
	{
		const bool b = field.Value().AsSmartBool();
		Enable(b);
	}
}

bool CRhRdkSingleColorTexture::IsPerObject(void) const
{
	return m_bUseObjectColor.Value().AsSmartBool();
}

void CRhRdkSingleColorTexture::OnModifyObjectAttributes(CRhinoDoc&, CRhinoObject&, const CRhinoObjectAttributes&)
{
	RHRDK_ASSERT(m_bUseObjectColor.Value().AsSmartBool());

	const CRhRdkContent::Change<CRhRdkContent> c(*this, RhRdkChangeContext::Program);
	c().Changed();
}
