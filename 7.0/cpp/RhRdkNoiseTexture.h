
#pragma once

#include "RhRcmUtilities.h"
#include "Resource.h"

class CRhRdkNoiseTexture : public CRhRdkTwoColorTextureBase
{
	typedef CRhRdkTwoColorTextureBase _super;
public:
	CRhRdkNoiseTexture(void);
	virtual ~CRhRdkNoiseTexture(void);

	enum class NoiseType : unsigned int
	{
		kPerlin,
		kValueNoise,
		kPerlinPlusValue,
		kSimplex,
		kSparseConvolution,
		kLatticeConvolution,
		kWardsHermite,
		kAaltonen,
	};

	NoiseType GetNoiseType(void) const;
	void SetNoiseType(NoiseType type);

	enum SpecSynthType : unsigned int
	{
		kFractalSum,
		kTurbulence,
	};

	SpecSynthType GetSpecSynthType(void) const;
	void SetSpecSynthType(SpecSynthType type);

	int OctaveCount(void) const;
	void SetOctaveCount(int i);

	bool Inverse(void) const;
	void SetInverse(bool b);

	bool ScaleToClamp(void) const;
	void SetScaleToClamp(bool b);

	double FrequencyMultiplier(void) const;
	void SetFrequencyMultiplier(double d);

	double AmplitudeMultiplier(void) const;
	void SetAmplitudeMultiplier(double d);

	double ClampMin(void) const;
	void SetClampMin(double d);

	double ClampMax(void) const;
	void SetClampMax(double d);

	double Gain(void) const;
	void SetGain(double d);

public:
	virtual UUID RenderEngineId(void) const override;
	virtual UUID PlugInId(void) const override;
	virtual UUID TypeId(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual const wchar_t* Category(void) const override;
	virtual void* GetShader(const UUID& uuidRenderEngine, void* pvData) const override { return 0; }
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_NOISE, size, dibOut); }
	virtual bool IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;

protected:
	virtual unsigned int BitFlags(void) const override;
	virtual bool IsImageBased(void) const override { return false; }
	virtual bool IsBuiltIn(void) const override { return true; }
	virtual bool SupportsContextHelp(void) const override { return true; }
	virtual void DisplayContextHelp(HWND h) const override { RhinoApp().DoHelp(L"commands/textures.htm#noise"); }

private:
	CRhRdkContentField m_sNoiseType;
	CRhRdkContentField m_sSpectralSynthesisType;
	CRhRdkContentField m_iOctaveCount;
	CRhRdkContentField m_bInverse;
	CRhRdkContentField m_bScaleToClamp;
	CRhRdkContentField m_dFrequencyMultiplier;
	CRhRdkContentField m_dAmplitudeMultiplier;
	CRhRdkContentField m_dClampMin;
	CRhRdkContentField m_dClampMax;
	CRhRdkContentField m_dGain;
};
