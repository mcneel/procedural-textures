
#include "stdafx.h"
#include "RhRdkFBmTexture.h"
#include "RhRdkTextureUtilities.h"

class CRhRdkFBmTexture::Evaluator : public CRhRdkTwoColorEvaluator
{
public:
	Evaluator(const CRhRdkFBmTexture& texture, CEvalFlags ef);

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const;

protected:
	virtual double Func(const ON_3dPoint& P, const ON_3dVector& dpdx, const ON_3dVector& dpdy, double omega, int maxOctaves) const;

protected:
	int m_iMaxOctaves;
	double m_dGain;
	double m_dRoughness;
};

CRhRdkFBmTexture::Evaluator::Evaluator(const CRhRdkFBmTexture& texture, CEvalFlags ef)
	:
	CRhRdkTwoColorEvaluator(texture, ef)
{
	m_iMaxOctaves = texture.MaxOctaves();
	m_dGain       = texture.Gain();
	m_dRoughness  = texture.Roughness();
}

CRhRdkColor CRhRdkFBmTexture::Evaluator::GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const
{
	const ON_3dPoint P = LocalMappingTransform() * uvw;

	CRhRdkColor colOut1 = OutputColor(1, uvw, duvwdx, duvwdy, pvData);
	CRhRdkColor colOut2 = OutputColor(2, uvw, duvwdx, duvwdy, pvData);

	const ON_3dVector d1 = LocalMappingTransform() * duvwdx;
	const ON_3dVector d2 = LocalMappingTransform() * duvwdy;

	const float t = (float)std::abs(Func(P, d1, d2, m_dRoughness, m_iMaxOctaves) * m_dGain);
	colOut1.BlendTo(t, colOut2, true);
	colOut1.Clamp();

	return colOut1;
}

double CRhRdkFBmTexture::Evaluator::Func(const ON_3dPoint& P, const ON_3dVector& dpdx, const ON_3dVector& dpdy, double omega, int maxOctaves) const
{
	return FBm(P, dpdx, dpdy, m_dRoughness, m_iMaxOctaves);
}

//---------------------

CRhRdkFBmTexture::CRhRdkFBmTexture()
	:
	m_iMaxOctaves(*this, FS_TEX_FBM_MAX_OCTAVES, RhLocalizeString( L"Max Octaves", 33695), L"Max Octaves"),
	m_dRoughness (*this, FS_TEX_FBM_ROUGHNESS  , RhLocalizeString( L"Roughness", 33696)  , L"Roughness"  ),
	m_dGain      (*this, FS_TEX_FBM_GAIN       , RhLocalizeString( L"Gain", 33697)       , L"Gain"       )
{
	m_iMaxOctaves = 5;
	m_dRoughness = 0.5;
	m_dGain = 1.0;

	m_iMaxOctaves.SetLimits(1, 20);
	m_dRoughness.SetLimits(0.0, 2.0);
	m_dGain.SetLimits(0.0, 1000.0);
}

UUID CRhRdkFBmTexture::TypeId(void) const
{
	return uuidFBmTextureType;
}

ON_wString CRhRdkFBmTexture::TypeName(void) const
{
	return RhLocalizeString( L"fBm Texture", 22847);
}

ON_wString CRhRdkFBmTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Fractal noise texture", 22848);
}

ON_wString CRhRdkFBmTexture::InternalName(void) const
{
	return L"rcm-fbm-texture"; // [sic]
}

double CRhRdkFBmTexture::Gain(void) const
{
	return m_dGain;
}

void CRhRdkFBmTexture::SetGain(double d)
{
	m_dGain.SetValue(d);
}

double CRhRdkFBmTexture::Roughness(void) const
{
	return m_dRoughness;
}

void CRhRdkFBmTexture::SetRoughness(double d)
{
	m_dRoughness.SetValue(d);
}

int CRhRdkFBmTexture::MaxOctaves(void) const
{
	return m_iMaxOctaves;
}

void CRhRdkFBmTexture::SetMaxOctaves(int i)
{
	m_iMaxOctaves.SetValue(i);
}

IRhRdkTextureEvaluator* CRhRdkFBmTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new Evaluator(*this, ef);
}

// CRhRdkTurbulenceTexture

class CRhRdkTurbulenceTexture::Evaluator : public CRhRdkFBmTexture::Evaluator
{
public:
	Evaluator(const CRhRdkTurbulenceTexture& texture, CEvalFlags ef);

protected:
	virtual double Func(const ON_3dPoint& P, const ON_3dVector& dpdx, const ON_3dVector& dpdy, double omega, int maxOctaves) const;
};

CRhRdkTurbulenceTexture::Evaluator::Evaluator(const CRhRdkTurbulenceTexture& texture, CEvalFlags ef)
	:
	CRhRdkFBmTexture::Evaluator(texture, ef)
{
}

double CRhRdkTurbulenceTexture::Evaluator::Func(const ON_3dPoint& P, const ON_3dVector& dpdx, const ON_3dVector& dpdy, double omega, int maxOctaves) const
{
	return Turbulence(P, dpdx, dpdy, m_dRoughness, m_iMaxOctaves);
}

UUID CRhRdkTurbulenceTexture::TypeId(void) const
{
	return uuidTurbulenceTextureType;
}

ON_wString CRhRdkTurbulenceTexture::InternalName(void) const
{
	return L"rcm-wrinkled-texture";  // [sic]
}

IRhRdkTextureEvaluator* CRhRdkTurbulenceTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new Evaluator(*this, ef);
}

ON_wString CRhRdkTurbulenceTexture::TypeName(void) const
{
	return RhLocalizeString( L"Turbulence Texture", 22849);
}

ON_wString CRhRdkTurbulenceTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Fractal turbulence texture", 22850);
}
