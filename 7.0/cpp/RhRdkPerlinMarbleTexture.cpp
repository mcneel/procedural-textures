
#include "stdafx.h"
#include "RhRdkPerlinMarbleTexture.h"
#include "RhRdkTextureUtilities.h"

class CRhRdkPerlinMarbleTexture::Evaluator : public CRhRdkTwoColorEvaluator
{
public:
	Evaluator(const CRhRdkPerlinMarbleTexture& texture, CEvalFlags ef);

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const;

private:
	double m_dSize;
	double m_iLevels;
	double m_dNoise;
	double m_dBlur;
	double m_dCol1Sat;
	double m_dCol2Sat;
};

CRhRdkPerlinMarbleTexture::Evaluator::Evaluator(const CRhRdkPerlinMarbleTexture& texture, CEvalFlags ef)
	:
	CRhRdkTwoColorEvaluator(texture, ef)
{
	m_dSize    = texture.Size();
	m_iLevels  = texture.Levels();
	m_dNoise   = texture.Noise();
	m_dBlur    = texture.Blur();
	m_dCol1Sat = texture.Color1Saturation();
	m_dCol2Sat = texture.Color2Saturation();
}

CRhRdkColor CRhRdkPerlinMarbleTexture::Evaluator::GetColorSample(const ON_3dPoint& uvwOriginal, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const
{
	ON_3dVector uvw = LocalMappingTransform() * uvwOriginal;

	const ON_3dVector d1 = LocalMappingTransform() * duvwdx;
	const ON_3dVector d2 = LocalMappingTransform() * duvwdx;

	const CRhRdkColor colOut1 = OutputColor(1, uvwOriginal, duvwdx, duvwdy, pvData);
	const CRhRdkColor colOut2 = OutputColor(2, uvwOriginal, duvwdx, duvwdy, pvData);

	double totalValue = 0.0;
	double freq = 1.0;
	double weight = m_dNoise;
	for (int o = 0; o < m_iLevels; o++)
	{
		double value = 0.0;
		const double x = uvw.x * freq + (double)o;
		const double y = uvw.y * freq + (double)o * 2.0;
		const double z = uvw.z * freq - (double)o;
		value = ::Noise(x * m_dSize, y * m_dSize, z * m_dSize);
		totalValue += weight * value;
		freq *= 2.17;
		weight *= 0.5;
	}

	double phase = totalValue - floor(totalValue);

	CRhRdkColor startColor;
	CRhRdkColor endColor;
	double tweenValue = 0.0;

	if (0.0 <= phase && phase < 0.1)
	{
		tweenValue = phase * 10.0;

		startColor = colOut1;
		endColor = startColor * (float)m_dCol1Sat;
	}
	if (0.1 <= phase && phase < 0.5)
	{
		tweenValue = (phase - 0.1) * 2.5;

		startColor = colOut1;
		startColor *= (float)m_dCol1Sat;

		endColor = colOut2;
	}
	if (0.5 <= phase && phase < 0.6)
	{
		tweenValue = (phase - 0.5) * 10.0;

		startColor = colOut2;
		endColor = startColor * (float)m_dCol2Sat;
	}
	if (0.6 <= phase && phase <= 1.0)
	{
		tweenValue = (phase - 0.6) * 2.5;

		startColor = colOut2;
		startColor *= (float)m_dCol2Sat;

		endColor = colOut1;
	}

	if (m_dBlur > 0.0)
	{
		tweenValue = 0.5 + (2.0 * tweenValue - 1.0) / m_dBlur * 0.5;
		if (tweenValue < 0.0)
			tweenValue = 0.0;
		if (tweenValue > 1.0)
			tweenValue = 1.0;
	}
	else
		tweenValue = tweenValue > 0.5 ? 1.0 : 0.0;

	CRhRdkColor colOut = startColor * (float)(1.0 - tweenValue) + endColor * (float)tweenValue;

	return colOut;
}

// CRhRdkPerlinMarbleTexture

CRhRdkPerlinMarbleTexture::CRhRdkPerlinMarbleTexture()
	:
	m_dCol1Sat(*this,  FS_TEX_PERLIN_MARBLE_SAT_COLOR_1, RhLocalizeString( L"color 1 Saturation", 33750), L"Color 1 Saturation"),
	m_dCol2Sat(*this,  FS_TEX_PERLIN_MARBLE_SAT_COLOR_2, RhLocalizeString( L"color 2 Saturation", 33751), L"Color 2 Saturation"),
	m_dSize   (*this,  FS_TEX_PERLIN_MARBLE_SIZE       , RhLocalizeString( L"Size"              , 33746), L"Size"              ),
	m_iLevels (*this,  FS_TEX_PERLIN_MARBLE_LEVELS     , RhLocalizeString( L"Levels"            , 33747), L"Levels"            ),
	m_dNoise  (*this,  FS_TEX_PERLIN_MARBLE_NOISE      , RhLocalizeString( L"Noise"             , 33748), L"Noise"             ),
	m_dBlur   (*this,  FS_TEX_PERLIN_MARBLE_BLUR       , RhLocalizeString( L"Blur"              , 33749), L"Blur"              )
{
	m_dSize = 0.5;
	m_iLevels = 5;
	m_dNoise = 1.0;
	m_dBlur = 0.5;
	m_dCol1Sat = 1.0;
	m_dCol2Sat = 0.5;

	SetColor1(CRhRdkColor(0,  40, 160, 255));
	SetColor2(CRhRdkColor(0, 110, 210, 255));

	m_dSize.SetLimits(0.0, 10.0);
	m_iLevels.SetLimits(1, 10);
	m_dNoise.SetLimits(0.0, 10.0);
	m_dBlur.SetLimits(0.0, 1.0);
	m_dCol1Sat.SetLimits(0.0, 2.0);
	m_dCol2Sat.SetLimits(0.0, 2.0);
}

UUID CRhRdkPerlinMarbleTexture::TypeId(void) const
{
	return uuidPerlinMarbleTextureType;
}

ON_wString CRhRdkPerlinMarbleTexture::TypeName(void) const
{
	return RhLocalizeString( L"Perlin Marble Texture", 23365);
}

ON_wString CRhRdkPerlinMarbleTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Perlin marble texture", 23366);
}

ON_wString CRhRdkPerlinMarbleTexture::InternalName(void) const
{
	return L"rdk-perlin-marble-texture";
}

double CRhRdkPerlinMarbleTexture::Size(void) const
{
	return m_dSize;
}

void CRhRdkPerlinMarbleTexture::SetSize(double d)
{
	m_dSize.SetValue(d);
}

int CRhRdkPerlinMarbleTexture::Levels(void) const
{
	return m_iLevels;
}

void CRhRdkPerlinMarbleTexture::SetLevels(int i)
{
	m_iLevels.SetValue(i);
}

double CRhRdkPerlinMarbleTexture::Noise(void) const
{
	return m_dNoise;
}

void CRhRdkPerlinMarbleTexture::SetNoise(double d)
{
	m_dNoise.SetValue(d);
}

double CRhRdkPerlinMarbleTexture::Blur(void) const
{
	return m_dBlur;
}

void CRhRdkPerlinMarbleTexture::SetBlur(double d)
{
	m_dBlur.SetValue(d);
}

double CRhRdkPerlinMarbleTexture::Color1Saturation(void) const
{
	return m_dCol1Sat;
}

void CRhRdkPerlinMarbleTexture::SetColor1Saturation(double d)
{
	m_dCol1Sat.SetValue(d);
}

double CRhRdkPerlinMarbleTexture::Color2Saturation(void) const
{
	return m_dCol2Sat;
}

void CRhRdkPerlinMarbleTexture::SetColor2Saturation(double d)
{
	m_dCol2Sat.SetValue(d);
}

bool CRhRdkPerlinMarbleTexture::SetParameter(const wchar_t* wszName, const CRhRdkVariant& value)
{
	return __super::SetParameter(wszName, value);
}

IRhRdkTextureEvaluator* CRhRdkPerlinMarbleTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new Evaluator(*this, ef);
}
