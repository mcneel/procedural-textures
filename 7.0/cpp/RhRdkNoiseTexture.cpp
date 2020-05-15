
#include "stdafx.h"
#include "RhRdkNoiseTexture.h"
#include "RhRdkTextureUtilities.h"
#include "RhRcm.h"

CRhRdkNoiseTexture::CRhRdkNoiseTexture()
	:
	m_sNoiseType            (*this, FS_TEX_NOISE_TYPE          , RhLocalizeString( L"Noise Type", 33736)             , L"Noise Type"             ),
	m_sSpectralSynthesisType(*this, FS_TEX_NOISE_SPECSYNTH_TYPE, RhLocalizeString( L"Spectral Synthesis Type", 33737), L"Spectral Synthesis Type"),
	m_iOctaveCount          (*this, FS_TEX_NOISE_OCTAVE_COUNT  , RhLocalizeString( L"Octave Count", 33738)           , L"Octave Count"           ),
	m_dFrequencyMultiplier  (*this, FS_TEX_NOISE_FREQUENCY_MULT, RhLocalizeString( L"Frequency Multiplier", 33739)   , L"Frequency Multiplier"   ),
	m_dAmplitudeMultiplier  (*this, FS_TEX_NOISE_AMPLITUDE_MULT, RhLocalizeString( L"Amplitude Multiplier", 33740)   , L"Amplitude Multiplier"   ),
	m_dClampMin             (*this, FS_TEX_NOISE_CLAMP_MIN     , RhLocalizeString( L"Clamp Min", 33741)              , L"Clamp Min"              ),
	m_dClampMax             (*this, FS_TEX_NOISE_CLAMP_MAX     , RhLocalizeString( L"Clamp Max", 33742)              , L"Clamp Max"              ),
	m_bScaleToClamp         (*this, FS_TEX_NOISE_SCALE_TO_CLAMP, RhLocalizeString( L"Scale To Clamp", 33743)         , L"Scale To Clamp"         ),
	m_bInverse              (*this, FS_TEX_NOISE_INVERSE       , RhLocalizeString( L"Inverse", 33744)                , L"Inverse"                ),
	m_dGain                 (*this, FS_TEX_NOISE_GAIN          , RhLocalizeString( L"Gain", 33745)                   , L"Gain"                   )
{
	m_sNoiseType = FS_TEX_NOISE_TYPE_PERLIN;
	m_sSpectralSynthesisType = FS_TEX_NOISE_SPECSYNTH_TYPE_FRACTAL_SUM;
	m_iOctaveCount = 3;
	m_dFrequencyMultiplier = 2.0;
	m_dAmplitudeMultiplier = 0.5;
	m_dClampMin = -1.0;
	m_dClampMax = 1.0;
	m_bScaleToClamp = false;
	m_bInverse = false;
	m_dGain = 0.5;

	m_iOctaveCount.SetLimits(1, 10);
	m_dFrequencyMultiplier.SetLimits(0.0, CRhRdkVariant::Null());
	m_dAmplitudeMultiplier.SetLimits(0.0, CRhRdkVariant::Null());
	m_dClampMin.SetLimits(-1.0, 1.0);
	m_dClampMax.SetLimits(-1.0, 1.0);
	m_dGain.SetLimits(0.0, 1.0);
}

CRhRdkNoiseTexture::~CRhRdkNoiseTexture()
{
}

CRhRdkNoiseTexture::NoiseType CRhRdkNoiseTexture::GetNoiseType(void) const
{
	const CLBPString s = (const wchar_t*)m_sNoiseType;

	if (FS_TEX_NOISE_TYPE_VALUE_NOISE == s)
		return NoiseType::kValueNoise;

	if (FS_TEX_NOISE_TYPE_PERLIN_PLUS_VALUE == s)
		return NoiseType::kPerlinPlusValue;

	if (FS_TEX_NOISE_TYPE_SIMPLEX == s)
		return NoiseType::kSimplex;

	if (FS_TEX_NOISE_TYPE_SPARSE_CONVOLUTION == s)
		return NoiseType::kSparseConvolution;

	if (FS_TEX_NOISE_TYPE_LATTICE_CONVOLUTION == s)
		return NoiseType::kLatticeConvolution;

	if (FS_TEX_NOISE_TYPE_WARDS_HERMITE == s)
		return NoiseType::kWardsHermite;

	if (FS_TEX_NOISE_TYPE_AALTONEN == s)
		return NoiseType::kAaltonen;

	return NoiseType::kPerlin;
}

void CRhRdkNoiseTexture::SetNoiseType(NoiseType type)
{
	const wchar_t* wsz = FS_TEX_NOISE_TYPE_PERLIN;
	switch (type)
	{
	case NoiseType::kValueNoise:         wsz = FS_TEX_NOISE_TYPE_VALUE_NOISE;         break;
	case NoiseType::kPerlinPlusValue:    wsz = FS_TEX_NOISE_TYPE_PERLIN_PLUS_VALUE;   break;
	case NoiseType::kSimplex:            wsz = FS_TEX_NOISE_TYPE_SIMPLEX;             break;
	case NoiseType::kSparseConvolution:  wsz = FS_TEX_NOISE_TYPE_SPARSE_CONVOLUTION;  break;
	case NoiseType::kLatticeConvolution: wsz = FS_TEX_NOISE_TYPE_LATTICE_CONVOLUTION; break;
	case NoiseType::kWardsHermite:       wsz = FS_TEX_NOISE_TYPE_WARDS_HERMITE;       break;
	case NoiseType::kAaltonen:           wsz = FS_TEX_NOISE_TYPE_AALTONEN;            break;
	}

	m_sNoiseType.SetValue(wsz);
}

CRhRdkNoiseTexture::SpecSynthType CRhRdkNoiseTexture::GetSpecSynthType(void) const
{
	const CLBPString s = (const wchar_t*)m_sSpectralSynthesisType;

	if (FS_TEX_NOISE_SPECSYNTH_TYPE_TURBULENCE == s)
		return SpecSynthType::kTurbulence;

	return SpecSynthType::kFractalSum;
}

void CRhRdkNoiseTexture::SetSpecSynthType(SpecSynthType type)
{
	const wchar_t* wsz = FS_TEX_NOISE_SPECSYNTH_TYPE_FRACTAL_SUM;
	if (type == SpecSynthType::kTurbulence)
		wsz = FS_TEX_NOISE_SPECSYNTH_TYPE_TURBULENCE;

	m_sSpectralSynthesisType.SetValue(wsz);
}

int CRhRdkNoiseTexture::OctaveCount(void) const
{
	return m_iOctaveCount;
}

void CRhRdkNoiseTexture::SetOctaveCount(int i)
{
	m_iOctaveCount.SetValue(i);
}

double CRhRdkNoiseTexture::FrequencyMultiplier(void) const
{
	return m_dFrequencyMultiplier;
}

void CRhRdkNoiseTexture::SetFrequencyMultiplier(double d)
{
	m_dFrequencyMultiplier.SetValue(d);
}

double CRhRdkNoiseTexture::AmplitudeMultiplier(void) const
{
	return m_dAmplitudeMultiplier;
}

void CRhRdkNoiseTexture::SetAmplitudeMultiplier(double d)
{
	m_dAmplitudeMultiplier.SetValue(d);
}

double CRhRdkNoiseTexture::ClampMin(void) const
{
	return m_dClampMin;
}

void CRhRdkNoiseTexture::SetClampMin(double d)
{
	m_dClampMin.SetValue(d);
}

double CRhRdkNoiseTexture::ClampMax(void) const
{
	return m_dClampMax;
}

void CRhRdkNoiseTexture::SetClampMax(double d)
{
	m_dClampMax.SetValue(d);
}

double CRhRdkNoiseTexture::Gain(void) const
{
	return m_dGain;
}

void CRhRdkNoiseTexture::SetGain(double d)
{
	m_dGain.SetValue(d);
}

bool CRhRdkNoiseTexture::ScaleToClamp(void) const
{
	return m_bScaleToClamp;
}

void CRhRdkNoiseTexture::SetScaleToClamp(bool b)
{
	m_bScaleToClamp.SetValue(b);
}

bool CRhRdkNoiseTexture::Inverse(void) const
{
	return m_bInverse;
}

void CRhRdkNoiseTexture::SetInverse(bool b)
{
	m_bInverse.SetValue(b);
}

UUID CRhRdkNoiseTexture::RenderEngineId(void) const
{
	return uuidUniversalRenderEngine;
}

UUID CRhRdkNoiseTexture::PlugInId(void) const
{
	return rdk.RhinoPlugInUuid();
}

UUID CRhRdkNoiseTexture::TypeId(void) const
{
	return uuidNoiseTextureType;
}

ON_wString CRhRdkNoiseTexture::InternalName(void) const
{
	return L"NoiseTexture";
}

ON_wString CRhRdkNoiseTexture::TypeName(void) const
{
	return RhLocalizeString( L"Noise Texture", 32483);
}

ON_wString CRhRdkNoiseTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Noise texture", 32484);
}

const wchar_t* CRhRdkNoiseTexture::Category(void) const
{
	return RDK_CAT_PROCEDURAL_3D;	
}

bool CRhRdkNoiseTexture::IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const
{
	return f.IsKind(CRhRdkContent::Kinds::Texture);
}

class CRhRdkNoiseTextureEvaluator : public CRhRdkTwoColorEvaluator
{
public:
	CRhRdkNoiseTextureEvaluator(const CRhRdkNoiseTexture& texture, CEvalFlags ef)
	: CRhRdkTwoColorEvaluator(texture, ef)
	{
		m_noiseType           = texture.GetNoiseType();
		m_octaveCount         = texture.OctaveCount();
		m_SpecSynthType       = texture.GetSpecSynthType();
		m_frequencyMultiplier = (float)texture.FrequencyMultiplier();
		m_amplitudeMultiplier = (float)texture.AmplitudeMultiplier();
		m_clampMin            = (float)texture.ClampMin();
		m_clampMax            = (float)texture.ClampMax();
		m_bScaleToClamp       = texture.ScaleToClamp();
		m_bInverse            = texture.Inverse();
		m_dGain               = (float)texture.Gain();
		m_xform               = texture.LocalMappingTransform();
	}

	virtual ~CRhRdkNoiseTextureEvaluator() { }

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw,
									   const ON_3dVector& duvwdx,
									   const ON_3dVector& duvwdy, void* pvData) const;

	virtual void DeleteThis(void) override { delete this; }
	virtual void* EVF(const wchar_t*, void*) override { return nullptr; }

protected:
	ON_Xform m_xform;
	CRhRdkNoiseTexture::NoiseType m_noiseType;
	CRhRdkNoiseTexture::SpecSynthType m_SpecSynthType;
	int m_octaveCount;
	float m_frequencyMultiplier;
	float m_amplitudeMultiplier;
	float m_clampMin;
	float m_clampMax;
	bool m_bInverse;
	bool m_bScaleToClamp;
	float m_dGain;
};

CRhRdkColor CRhRdkNoiseTextureEvaluator::GetColorSample(const ON_3dPoint& uvwOriginal, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const
{
	const ON_3fPoint uvw( m_xform * uvwOriginal);

	float totalValue = 0.0;
	float freq = 1.0;
	float weight = 1.0;
	float totalWeight = 0.0;

	for (int o = 0; o < m_octaveCount; o++)
	{
		const float fo = float(o);
		float value = 0.0;
		const float x = uvw.x * freq + fo;
		const float y = uvw.y * freq + fo * 2.0f;
		const float z = uvw.z * freq - fo;
		
		switch (m_noiseType)
		{
			case CRhRdkNoiseTexture::NoiseType::kPerlin:
				value = Noise(x, y, z);
				break;
			case CRhRdkNoiseTexture::NoiseType::kValueNoise:
				value = ValueNoise(x, y, z);
				break;
			case CRhRdkNoiseTexture::NoiseType::kPerlinPlusValue:
				value = 0.5f * ValueNoise(x, y, z) + 0.5f * Noise(x, y, z);
				break;
			case CRhRdkNoiseTexture::NoiseType::kSimplex:
				value = (float)SimplexNoise(x, y, z);
				break;
			case CRhRdkNoiseTexture::NoiseType::kSparseConvolution:
				value = SCNoise(x, y, z);
				break;
			case CRhRdkNoiseTexture::NoiseType::kLatticeConvolution:
				value = VCNoise(x, y, z);
				break;
			case CRhRdkNoiseTexture::NoiseType::kWardsHermite:
				value = WardsHermiteNoise(x, y, z);
				break;
			case CRhRdkNoiseTexture::NoiseType::kAaltonen:
				value = AaltonenNoise(x, y, z);
				break;
			default:
				break;
		}

		if (m_SpecSynthType == CRhRdkNoiseTexture::SpecSynthType::kTurbulence && (value < 0.0f))
			value = -value;
		totalValue += weight * value;

		totalWeight += weight;

		freq *= m_frequencyMultiplier;
		weight *= m_amplitudeMultiplier;
	}

	if (totalWeight > 0.0f)
		totalValue /= totalWeight;

	if (m_SpecSynthType == CRhRdkNoiseTexture::SpecSynthType::kTurbulence)
		totalValue = 2.0f * totalValue - 1.0f;

	if (totalValue >= m_clampMax)
		totalValue  = m_clampMax;

	if (totalValue <= m_clampMin)
		totalValue  = m_clampMin;

	if (m_bScaleToClamp)
	{
		if (m_clampMax - m_clampMin != 0.0f)
		{
			totalValue = -1.0f + 2.0f * (totalValue - m_clampMin) / (m_clampMax - m_clampMin);
		}
		else
		{
			totalValue = -1.0f;
		}
	}

	if (m_bInverse)
	{
		totalValue = -totalValue;
	}

	const float colorValueBeforeGain = 0.5f * (1.0f + totalValue);
	const float colorValueAfterGain = Gain(colorValueBeforeGain, m_dGain);

	CRhRdkColor colOut1 = OutputColor(1, uvwOriginal, duvwdx, duvwdy, pvData);
	CRhRdkColor colOut2 = OutputColor(2, uvwOriginal, duvwdx, duvwdy, pvData);

	colOut1.BlendTo(colorValueAfterGain, colOut2, true);

	return colOut1;
}

IRhRdkTextureEvaluator* CRhRdkNoiseTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new CRhRdkNoiseTextureEvaluator(*this, ef);
}

unsigned int CRhRdkNoiseTexture::BitFlags(void) const
{
	return __super::BitFlags() & ~bfTextureSummary;
}
