
#pragma once

#include "RhRcmUtilities.h"
#include "Resource.h"

class CRhRdkSingleColorTexture : public CRhRdkOneColorTextureBase, public CRhinoEventWatcher
{
	typedef CRhRdkOneColorTextureBase _super;
public:
	CRhRdkSingleColorTexture();
	virtual ~CRhRdkSingleColorTexture();

	CRhRdkColor Color(void) const { return Color1(); }
	void SetColor(const CRhRdkColor& col) { SetColor1(col); }

protected: // Overrides from CRhRdkContent.
	virtual UUID TypeId(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual const wchar_t* Category(void) const override { return RDK_CAT_PROCEDURAL_2D; }
	virtual void SimulateTexture(CRhRdkSimulatedTexture& texOut, CRhRdkTexture::TextureGeneration tg, int iSize, const CRhinoObject* pObject) const override;
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_SINGLECOLOR, size, dibOut); }
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#singlecolor"); }
	virtual void OnFieldChanged(const CRhRdkContentField& field, const CRhRdkVariant& vOldValue, RhRdkChangeContext cc, void* pReserved);
	virtual bool IsPerObject(void) const override;

protected: // Rhino event watcher overrides.
	virtual void OnModifyObjectAttributes(CRhinoDoc& doc, CRhinoObject& object, const CRhinoObjectAttributes& old_attributes);

private:
	CRhRdkContentField m_bUseObjectColor;
	class Evaluator;
};
