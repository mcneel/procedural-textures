
#pragma once

#include "RhRcmUtilities.h"
#include "Resource.h"

class CRhRdkDotTexture : public CRhRdkTwoColorTextureBase
{
	typedef CRhRdkTwoColorTextureBase _super;
public:
	CRhRdkDotTexture();

	RhRdkDotSamplerType SamplerType(void) const;
	void SetSamplerType(RhRdkDotSamplerType type);

	RhRdkDotFallOffType FallOffType(void) const;
	void SetFallOffType(RhRdkDotFallOffType type);

	RhRdkDotCompositionType CompositionType(void) const;
	void SetCompositionType(RhRdkDotCompositionType type);

	int NumberOfDots(void) const;
	void SetNumberOfDots(int i);

	double DotProbability(void) const;
	void SetDotProbability(double d);

	double Jitter(void) const;
	void SetJitter(double d);

	int GridSize(void) const;
	void SetGridSize(int i);

	int PatternSize(void) const;
	void SetPatternSize(int i);

	double Radius(void) const;
	void SetRadius(double d);

	double RadiusVariance(void) const;
	void SetRadiusVariance(double d);

	bool Rings(void) const;
	void SetRings(bool b);

	double RingRadius(void) const;
	void SetRingRadius(double d);

	double RingRadiusVariance(void) const;
	void SetRingRadiusVariance(double d);

	double Amplitude(void) const;
	void SetAmplitude(double d);

	double AmplitudeVariance(void) const;
	void SetAmplitudeVariance(double d);

	double HueVariance(void) const;
	void SetHueVariance(double d);

	double SaturationVariance(void) const;
	void SetSaturationVariance(double d);

protected:
	virtual UUID RenderEngineId(void) const override;
	virtual UUID PlugInId(void) const override;
	virtual UUID TypeId(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual const wchar_t* Category(void) const override;
	virtual void* GetShader(const UUID& uuidRenderEngine, void* pvData) const override { return 0; }
	virtual bool Icon(const ON_2iSize& size, CRhinoDib& dibOut) const override { return DibIcon(IDI_TEXTURE_DOTS, size, dibOut); }
	virtual bool IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const override;
	virtual IRhRdkTextureEvaluator* NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const override;
	virtual bool IsBuiltIn() const override { return true; }

protected:
	class Evaluator;
	virtual bool IsImageBased(void) const { return false; }
	virtual bool SupportsContextHelp(void) const { return true; }
	virtual void DisplayContextHelp(HWND h) const { RhinoApp().DoHelp(L"commands/textures.htm#dots"); }

private:
	CRhRdkContentField m_samplerType;
	CRhRdkContentField m_fallOffType;
	CRhRdkContentField m_compositionType;
	CRhRdkContentField m_sampleAreaSize;
	CRhRdkContentField m_numberOfDots;
	CRhRdkContentField m_jitter;
	CRhRdkContentField m_gridSize;
	CRhRdkContentField m_dotProbability;
	CRhRdkContentField m_radius;
	CRhRdkContentField m_radiusVariance;
	CRhRdkContentField m_amplitude;
	CRhRdkContentField m_amplitudeVariance;
	CRhRdkContentField m_hueVariance;
	CRhRdkContentField m_saturationVariance;
	CRhRdkContentField m_bRings;
	CRhRdkContentField m_ringRadius;
	CRhRdkContentField m_ringRadiusVariance;
};
