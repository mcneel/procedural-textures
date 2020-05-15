
#pragma once

#include "RhRcmUtilities.h"

inline float Grad(int x, int y, int z, float dx, float dy, float dz);
inline float NoiseWeight(float t);

double Noise(double x, double y, double z = 0.5);
float  Noise(float  x, float  y, float  z = 0.5f);
inline float Noise(const ON_3fPoint& p) { return Noise(p.x, p.y, p.z); }
inline float Noise(const ON_3dPoint& p) { return (float)Noise(p.x, p.y, p.z); }

class C3dSimplexNoise final
{
public:
	double Noise(double x, double y, double z);

private:
	int m_i = 0;
	int m_j = 0;
	int m_k = 0;
	int m_a[3] = { 0 };
	double m_u = 0.0;
	double m_v = 0.0;
	double m_w = 0.0;
	double K(int a);
	static int Shuffle(int i, int j, int k);
	static inline int B(int i, int j, int k, int b);
	static inline int B(int n, int b);
	static const int m_t[8];
};

double SimplexNoise(double x, double y, double z);
inline double SimplexNoise(const ON_3dPoint & point) { return SimplexNoise(point.x, point.y, point.z); }
inline float SimplexNoise(float x, float y, float z) { return (float)SimplexNoise((double)x, (double)y, (double)z); }
inline float SimplexNoise(const ON_3fPoint & point) { return SimplexNoise(point.x, point.y, point.z); }

inline int Mod(int a, int b)
{
	const int n = int(a / b);
	a -= n * b;
	if (a < 0)
		a += b;
	return a;
}

static const double invLog2 = 1.0 / log(2.0);

inline double Log2(double x)
{
	return log(x) * invLog2;
}

inline int Log2Int(float v)
{
	return ((*(int*)&v) >> 23) - 127;
}

inline bool IsPowerOf2(int v)
{
	return (v & (v - 1)) == 0;
}

inline UINT RoundUpPow2(UINT v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return v+1;
}

//almost .5f = .5f - 1e^(number of exp bit)
#define _doublemagicroundeps (0.5 - 1.4e-11)
#define _doublemagic         6755399441055744.0

inline int Round2Int(double val) 
{
	//2^52 * 1.5,  uses limited precision to floor
	val		= val + _doublemagic;
	return ((long*)&val)[0];
}

inline int Float2Int(double val) 
{
	return (val<0) ?  Round2Int(val+_doublemagicroundeps) :
		   Round2Int(val-_doublemagicroundeps);
}

inline int Floor2Int(double val) 
{
	return Round2Int(val - _doublemagicroundeps);
}

inline int Ceil2Int(double val) 
{
	return Round2Int(val + _doublemagicroundeps);
}

inline bool Quadratic(float A, float B, float C, float *t0,	float *t1) 
{
	// Find quadratic discriminant
	float discrim = B * B - 4.0f * A * C;
	if (discrim < 0.) return false;
	float rootDiscrim = sqrtf(discrim);
	// Compute quadratic _t_ values
	float q;
	if (B < 0) q = -0.5f * (B - rootDiscrim);
	else       q = -0.5f * (B + rootDiscrim);
	*t0 = q / A;
	*t1 = C / q;
	if (*t0 > *t1) std::swap(*t0, *t1);
	return true;
}

inline double SmoothStep(double min, double max, double value) 
{
	const double v = Clamp((value - min) / (max - min), 0.0, 1.0);
	return v * v * (-2.0 * v  + 3.0);
}

inline double ExponentialAverage(double avg, double val, double alpha) 
{
	return (1.0 - alpha) * val + alpha * avg;
}

static const double log05 = log(0.5);

template <class TYPE> 
inline TYPE Bias(TYPE x, TYPE b)
{
	const TYPE smallv = TYPE(1e-6);
	if (b < smallv)
		b = smallv;

	if (x < 0.0)
		x = 0.0;

	const TYPE div = TYPE(log(b)) / TYPE(log05);

	return pow(x, div);
}

template <class TYPE> 
TYPE Gain(TYPE x, TYPE dGain)
{
	const TYPE one = 1.0;
	const TYPE half = 0.5;
	const TYPE two = 2.0;

	dGain = one - dGain;

	RHRDK_ASSERT( dGain >= 0.0 && dGain <= 1.0 );
	RHRDK_ASSERT( x >= 0.0 );

	if (x < half)
	{
		const TYPE dBias = Bias(two * x, dGain);
		RHRDK_ASSERT(!_isnan(dBias) && _finite(dBias));

		return dBias * half;
	}

	const TYPE dBias = Bias(two - two * x, dGain);
	RHRDK_ASSERT(!_isnan(dBias) && _finite(dBias));

	return one - (dBias * half);
}

void init_genrand(ULONG seed);

ON__UINT32 genrand_int32(void);

/* generates a random number on [0,1]-real-interval */
inline float genrand_real1(void) { return genrand_int32()*(1.0f / 4294967295.0f); }

/* generates a random number on [0,1)-real-interval */
inline float genrand_real2(void) { return genrand_int32()*(1.0f / 4294967296.0f); }

inline float RandomFloat() 
{
	return genrand_real2();
}

inline ON__UINT32 RandomUInt() 
{
	return genrand_int32();
}

double FBm(const ON_3dPoint& P, const ON_3dVector& dpdx, const ON_3dVector& dpdy, double omega, int maxOctaves);
double Turbulence(const ON_3dPoint& P, const ON_3dVector& dpdx, const ON_3dVector& dpdy, double omega, int maxOctaves);
double Lanczos(double x, double tau);

void NormalizeUVW(ON_3dPoint& uvw);
bool InsideUnitRange2d(const ON_3dPoint& uvw);
bool InsideUnitRange3d(const ON_3dPoint& uvw);

// Handles NaNs and infs
void NormalizeUV(double& u, double& v);

float ValueNoise(float x, float y, float z);
double ValueNoise(double x, double y, double z);

float SCNoise(float x, float y, float z);
inline double SCNoise(double x, double y, double z) { return (double)SCNoise((float)x, (float)y, (float)z); }

float VCNoise(float x, float y, float z);
inline double VCNoise(double x, double y, double z) { return (double)VCNoise((float)x, (float)y, (float)z); }

double WardsHermiteNoise(double x, double y, double z);
inline float WardsHermiteNoise(float x, float y, float z) { return (float)WardsHermiteNoise((double)x, (double)y, (double)z); }

// Returns a value in range [-1, 1]. Pattern repeats every 256 units in each 3 dimensions.
// Not differentiable but c0-continuous.
float AaltonenNoise(float x, float y, float z);
inline double AaltonenNoise(double x, double y, double z) { return (double)AaltonenNoise((float)x, (float)y, (float)z); }


// Texture projection conversions

void EquirectToWorld(float u, float v, ON_3fVector& vec);

void WorldToEquirect(const ON_3fVector& vec, float& u, float& v);

void HemisphericalToWorld(float u, float v, ON_3fVector& vec);

void WorldToHemispherical(const ON_3fVector& vec, float& u, float& v);

bool LightProbeToWorld(float u, float v, ON_3fVector& vec);

void WorldToLightProbe(const ON_3fVector& vec, float& u, float& v);

void CubemapToWorld(float u, float v, ON_3fVector& vecOut);

void WorldToCubemap(const ON_3fVector& vec, float& u, float& v);

bool VerticalCrossCubemapToWorld(float u, float v, ON_3fVector& vecOut);

void WorldToVerticalCrossCubemap(const ON_3fVector& vec, float& u, float& v);

bool HorizontalCrossCubemapToWorld(float u, float v, ON_3fVector& vecOut);

void WorldToHorizontalCrossCubemap(const ON_3fVector& vec, float& u, float& v);

bool EmapToWorld(float u, float v, ON_3fVector& vec);

void WorldToEmap(const ON_3fVector& vec, float& u, float& v);

bool AdjustColor(CRhRdkColor& color, double gamma, double gain, double multiplier, bool bClamp, double clampMin, double clampMax, bool bScaleToClamp, double hueShift, double saturation, bool bInvert, bool bTreatAsHDR, bool bGrayscale);

bool TextureIsOn(const CRhRdkMaterial& material, const wchar_t* sChildSlotName);

bool IsHDRFile(const wchar_t* wszFilename);
bool IsEXRFile(const wchar_t* wszFilename);

class CRhRdkHighDynamicRangeImageEx_RGB* NewHighDynamicRangeImage(const wchar_t* wszFilename, bool bAllocate);
