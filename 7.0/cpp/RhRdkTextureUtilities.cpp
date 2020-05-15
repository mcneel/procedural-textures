
#include "stdafx.h"
#include "RhRdkTextureUtilities.h"
#include "RhRdkContentRegistry.h"
#include "RhRdkHDRImage.h"
#include "RhRdkEXRImage.h"

using namespace std;

const float RDK_M_PI = (float)ON_PI;
const float INV_PI = (float)(1.0/ON_PI);
const float INV_TWOPI = (float)(1.0/2.0*ON_PI);
const float RDK_INFINITY = FLT_MAX;

// Perlin Noise Data
#define NOISE_PERM_SIZE 256
static int NoisePerm[2 * NOISE_PERM_SIZE] = 
{
	151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96,
	53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
	// Rest of noise permutation table
	8, 99, 37, 240, 21, 10, 23,
	190,  6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168,  68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54,  65, 25, 63, 161,  1, 216, 80, 73, 209, 76, 132, 187, 208,  89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186,  3, 64, 52, 217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152,  2, 44, 154, 163,  70, 221, 153, 101, 155, 167,  43, 172, 9,
	129, 22, 39, 253,  19, 98, 108, 110, 79, 113, 224, 232, 178, 185,  112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,  81, 51, 145, 235, 249, 14, 239, 107,
	49, 192, 214,  31, 181, 199, 106, 157, 184,  84, 204, 176, 115, 121, 50, 45, 127,  4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
	151, 160, 137, 91, 90, 15,
	131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
	190,  6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168,  68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54,  65, 25, 63, 161,  1, 216, 80, 73, 209, 76, 132, 187, 208,  89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186,  3, 64, 52, 217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152,  2, 44, 154, 163,  70, 221, 153, 101, 155, 167,  43, 172, 9,
	129, 22, 39, 253,  19, 98, 108, 110, 79, 113, 224, 232, 178, 185,  112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,  81, 51, 145, 235, 249, 14, 239, 107,
	49, 192, 214,  31, 181, 199, 106, 157, 184,  84, 204, 176, 115, 121, 50, 45, 127,  4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

inline double NoiseWeight(double t) 
{
	return t * t * t * (10.0 + t * (6.0 * t - 15.0));
}

inline float NoiseWeight(float t) 
{
	return t * t * t * (10.0f + t * (6.0f * t - 15.0f));
}

__forceinline double Gradient(int h, double dx, double dy, double dz) 
{
	if (h & 1)
	{
		if (h & 2)
		{
			double u = h < 8 ? dx : dy;
			double v = h < 4 ? dy : dz;
			return -u - v;
		}
		else
		{
			double u = h < 8 || h == 13 ? dx : dy;
			double v = h < 4 || h == 13 ? dy : dz;
			return -u + v;
		}
	}
	else
	{
		if (h & 2)
		{
			double u = h < 8 ? dx : dy;
			double v = h < 4 ? dy : dz;
			return u - v;
		}
		else
		{
			double u = h < 8 || h == 12 ? dx : dy;
			double v = h < 4 || h == 12 ? dy : dz;
			return u + v;
		}
	}
}

__forceinline float Gradient(int h, float dx, float dy, float dz) 
{
	if (h & 1)
	{
		if (h & 2)
		{
			float u = h < 8 ? dx : dy;
			float v = h < 4 ? dy : dz;
			return -u - v;
		}
		else
		{
			float u = h < 8 || h == 13 ? dx : dy;
			float v = h < 4 || h == 13 ? dy : dz;
			return -u + v;
		}
	}
	else
	{
		if (h & 2)
		{
			float u = h < 8 ? dx : dy;
			float v = h < 4 ? dy : dz;
			return u - v;
		}
		else
		{
			float u = h < 8 || h == 12 ? dx : dy;
			float v = h < 4 || h == 12 ? dy : dz;
			return u + v;
		}
	}
}

template<typename TYPE>
__forceinline TYPE NoiseImpl(TYPE x, TYPE y, TYPE z)
{
	const TYPE cx = floor(x);
	const TYPE cy = floor(y);
	const TYPE cz = floor(z);

	const TYPE dx = x - cx;
	const TYPE dy = y - cy;
	const TYPE dz = z - cz;

	const int ix = (int)cx & (NOISE_PERM_SIZE - 1);
	const int iy = (int)cy & (NOISE_PERM_SIZE - 1);
	const int iz = (int)cz & (NOISE_PERM_SIZE - 1);

	const int h0 = NoisePerm[ix];
	const int h1 = NoisePerm[ix + 1];
	const int h00 = NoisePerm[h0 + iy] + iz;
	const int h01 = NoisePerm[h1 + iy] + iz;
	const int h10 = NoisePerm[h0 + iy + 1] + iz;
	const int h11 = NoisePerm[h1 + iy + 1] + iz;

	const int h000 = NoisePerm[h00] & 15;
	const int h001 = NoisePerm[h01] & 15;
	const int h010 = NoisePerm[h10] & 15;
	const int h011 = NoisePerm[h11] & 15;
	const int h100 = NoisePerm[h00 + 1] & 15;
	const int h101 = NoisePerm[h01 + 1] & 15;
	const int h110 = NoisePerm[h10 + 1] & 15;
	const int h111 = NoisePerm[h11 + 1] & 15;

	const TYPE w000 = Gradient(h000, dx,   dy,   dz);
	const TYPE w100 = Gradient(h001, dx-1, dy,   dz);
	const TYPE w010 = Gradient(h010, dx,   dy-1, dz);
	const TYPE w110 = Gradient(h011, dx-1, dy-1, dz);
	const TYPE w001 = Gradient(h100, dx,   dy,   dz-1);
	const TYPE w101 = Gradient(h101, dx-1, dy,   dz-1);
	const TYPE w011 = Gradient(h110, dx,   dy-1, dz-1);
	const TYPE w111 = Gradient(h111, dx-1, dy-1, dz-1);

	const TYPE wx = NoiseWeight(dx);
	const TYPE wy = NoiseWeight(dy);
	const TYPE wz = NoiseWeight(dz);

	const TYPE y0 = w000 + wx * (w100 - w000 + wy * (w110 - w010 + w000 - w100)) + wy * (w010 - w000);
	const TYPE y1 = w001 + wx * (w101 - w001 + wy * (w111 - w011 + w001 - w101)) + wy * (w011 - w001);

	return y0 + wz * (y1 - y0);
}

double Noise(double x, double y, double z)
{
	return NoiseImpl(x,y,z);
}

float Noise(float x, float y, float z)
{
	return NoiseImpl(x,y,z);
}

double FBm(const ON_3dPoint& P, const ON_3dVector& dpdx, const ON_3dVector& dpdy, double omega, int maxOctaves)
{
	// Compute number of octaves for anti-aliased FBm
	const double s2 = max(dpdx.LengthSquared(), dpdy.LengthSquared());
	const double dOctaves = (s2 < 0.000001) ? maxOctaves : min((double)maxOctaves, 1.0 - 0.5 * Log2(s2));

	// Compute sum of octaves of noise for FBm
	double sum = 0.0, lambda = 1.0, o = 1.0;
	const int octaves = Floor2Int(dOctaves);
	for (int i = 0; i < octaves; ++i)
	{
		sum += o * Noise(lambda * P);
		lambda *= 1.99;
		o *= omega;
	}

	const double partialOctave = dOctaves - octaves;
	sum += o * SmoothStep(0.3, 0.7, partialOctave) * Noise(lambda * P);

	return sum;
}

double Turbulence(const ON_3dPoint& P, const ON_3dVector& dpdx, const ON_3dVector& dpdy, double omega, int maxOctaves)
{
	// Compute number of octaves for anti-aliased FBm
	const double s2 = max(dpdx.LengthSquared(), dpdy.LengthSquared());
	const double dOctaves = (s2 < 0.000001) ? maxOctaves : min((double)maxOctaves, 1.0 - 0.5 * Log2(s2));

	// Compute sum of octaves of noise for turbulence
	double sum = 0.0, lambda = 1.0, o = 1.0;
	const int octaves = Floor2Int(dOctaves);
	for (int i = 0; i < octaves; ++i)
	{
		sum += o * std::abs(Noise(lambda * P));
		lambda *= 1.99;
		o *= omega;
	}

	const double partialOctave = dOctaves - octaves;
	sum += o * SmoothStep(0.3, 0.7, partialOctave) * std::abs(Noise(lambda * P));

	return sum;
}

// Texture Function Definitions
double Lanczos(double x, double tau) 
{
	x = std::abs(x);
	if (x < 1e-5)
		return 1.0;
	if (x > 1.0)
		return 0.0;
	x *= RDK_M_PI;

	double s = sin(x * tau) / (x * tau);
	double lanczos = sin(x) / x;

	return s * lanczos;
}

template<typename TYPE>
__forceinline static void Normalize(TYPE& d)
{
	// Jussi, Dec-9-2011: If d is negative but very close to zero then
	//                    floor(d) = -1  and d - floor(d) = 0 - -1 = 1.
	//                    Some callers assume the result to be strictly
	//                    less than 1. Adding and subtracting 1 sets
	//                    almost zeros to zeros.
	const TYPE dd = d + 1 - 1;
	const TYPE dFloor = floor(dd);
	d = dd - dFloor;
}

void NormalizeUVW(ON_3dPoint& uvw)
{
	Normalize(uvw.x);
	Normalize(uvw.y);
	Normalize(uvw.z);

	RHRDK_ASSERT(uvw.x >= 0.0 && uvw.x < 1.0);
	RHRDK_ASSERT(uvw.y >= 0.0 && uvw.y < 1.0);
	RHRDK_ASSERT(uvw.z >= 0.0 && uvw.z < 1.0);
}

// Checks if a double is nan or inf
__forceinline static bool IsNanOrInf(double d)
{
	const unsigned long long qwD = *(const unsigned long long*)&d;
	if (0x7ff0000000000000 == (qwD & 0x7ff0000000000000))
		return true;
	else
		return false;
}

bool InsideUnitRange2d(const ON_3dPoint& uvw)
{
	if (IsNanOrInf(uvw.x) || IsNanOrInf(uvw.y))
		return false;

	if (uvw.x > 1.0 || uvw.x < 0.0)		return false;
	if (uvw.y > 1.0 || uvw.y < 0.0)		return false;

	return true;
}

bool InsideUnitRange3d(const ON_3dPoint& uvw)
{
	if (IsNanOrInf(uvw.x) || IsNanOrInf(uvw.y) || IsNanOrInf(uvw.z))
		return false;

	if (uvw.x > 1.0 || uvw.x < 0.0)		return false;
	if (uvw.y > 1.0 || uvw.y < 0.0)		return false;
	if (uvw.z > 1.0 || uvw.z < 0.0)		return false;

	return true;
}

void NormalizeUV(double& u, double& v)
{
	if (IsNanOrInf(u))
		u = 0.0;
	else
		Normalize(u);

	RHRDK_ASSERT(!_isnan(u));
	RHRDK_ASSERT(_finite(u));
	RHRDK_ASSERT(u >= 0.0 && u < 1.0);

	if (IsNanOrInf(v))
		v = 0.0;
	else
		Normalize(v);

	RHRDK_ASSERT(!_isnan(v));
	RHRDK_ASSERT(_finite(v));
	RHRDK_ASSERT(v >= 0.0 && v < 1.0);
}

float ValueNoise(float x, float y, float z)
{
	static bool bValueNoiseInitialized = false;
	static float aValueNoisePRNArray[256];

	// Initialize pseudo random number array if not initialized yet
	if (!bValueNoiseInitialized)
	{
		for (int i = 0; i < 256; i++)
			aValueNoisePRNArray[i] = -1.0f + (float)NoisePerm[i] / 255.0f * 2.0f;
		bValueNoiseInitialized = true;
	}

	float cx = floor(x);
	float cy = floor(y);
	float cz = floor(z);
	const float dx = x - cx;
	const float dy = y - cy;
	const float dz = z - cz;
	const int ix = (int)cx;
	const int iy = (int)cy;
	const int iz = (int)cz;

	const float prn000 = aValueNoisePRNArray[NoisePerm[NoisePerm[NoisePerm[(ix + 0) & 255] + ((iy + 0) & 255)] + ((iz + 0) & 255)]];
	const float prn001 = aValueNoisePRNArray[NoisePerm[NoisePerm[NoisePerm[(ix + 0) & 255] + ((iy + 0) & 255)] + ((iz + 1) & 255)]];
	const float prn010 = aValueNoisePRNArray[NoisePerm[NoisePerm[NoisePerm[(ix + 0) & 255] + ((iy + 1) & 255)] + ((iz + 0) & 255)]];
	const float prn011 = aValueNoisePRNArray[NoisePerm[NoisePerm[NoisePerm[(ix + 0) & 255] + ((iy + 1) & 255)] + ((iz + 1) & 255)]];
	const float prn100 = aValueNoisePRNArray[NoisePerm[NoisePerm[NoisePerm[(ix + 1) & 255] + ((iy + 0) & 255)] + ((iz + 0) & 255)]];
	const float prn101 = aValueNoisePRNArray[NoisePerm[NoisePerm[NoisePerm[(ix + 1) & 255] + ((iy + 0) & 255)] + ((iz + 1) & 255)]];
	const float prn110 = aValueNoisePRNArray[NoisePerm[NoisePerm[NoisePerm[(ix + 1) & 255] + ((iy + 1) & 255)] + ((iz + 0) & 255)]];
	const float prn111 = aValueNoisePRNArray[NoisePerm[NoisePerm[NoisePerm[(ix + 1) & 255] + ((iy + 1) & 255)] + ((iz + 1) & 255)]];

	const float wx = (2.0f * dx - 3.0f) * dx * dx + 1.0f;
	const float wy = (2.0f * dy - 3.0f) * dy * dy + 1.0f;
	const float wz = (2.0f * dz - 3.0f) * dz * dz + 1.0f;

	const float prn00X = prn000 * wz + prn001 * (1.0f - wz);
	const float prn01X = prn010 * wz + prn011 * (1.0f - wz);
	const float prn10X = prn100 * wz + prn101 * (1.0f - wz);
	const float prn11X = prn110 * wz + prn111 * (1.0f - wz);

	const float prn0XX = prn00X * wy + prn01X * (1.0f - wy);
	const float prn1XX = prn10X * wy + prn11X * (1.0f - wy);

	return prn0XX * wx + prn1XX * (1.0f - wx);
}

double ValueNoise(double x, double y, double z)
{
	return (double)ValueNoise((float)x, (float)y, (float)z);
}

const int C3dSimplexNoise::m_t [8] = {0x15, 0x38, 0x32, 0x2c, 0x0d, 0x13, 0x07, 0x2a};

double C3dSimplexNoise::Noise(double x, double y, double z)
{
	double s = (double)(x + y + z) / 3.0;
	m_i = (int)floor(x + s);
	m_j = (int)floor(y + s);
	m_k = (int)floor(z + s);
	s = (double)(m_i + m_j + m_k) / 6.0;
	m_u = x - (double)m_i + s;
	m_v = y - (double)m_j + s;
	m_w = z - (double)m_k + s;
	m_a[0] = m_a[1] = m_a[2] = 0;
	const int hi = (m_u >= m_w ? (m_u >= m_v ? 0 : 1) : (m_v >= m_w ? 1 : 2));
	const int lo = (m_u < m_w ? (m_u < m_v ? 0 : 1) : (m_v < m_w ? 1 : 2));
	const double k1 = K(hi);
	const double k2 = K(3 - hi - lo);
	const double k3 = K(lo);
	const double k4 = K(0);
	return k1 + k2 + k3 + k4;
}

double C3dSimplexNoise::K(int a)
{
	double s = (double)(m_a[0] + m_a[1] + m_a[2]) / 6.0;
	const double x = m_u - (double)m_a[0] + s;
	const double y = m_v - (double)m_a[1] + s;
	const double z = m_w - (double)m_a[2] + s;
	double t = 0.6 - x * x - y * y - z * z;
	const int h = Shuffle(m_i + m_a[0], m_j + m_a[1], m_k + m_a[2]);
	m_a[a]++;
	if (t < 0)
		return 0.0;
	const int b5 = (h >> 5) & 1;
	const int b4 = (h >> 4) & 1;
	const int b3 = (h >> 3) & 1;
	const int b2 = (h >> 2) & 1;
	const int b = (h & 3);
	double p = (b == 1 ? x : (b == 2 ? y : z));
	double q = (b == 1 ? y : (b == 2 ? z : x));
	double r = (b == 1 ? z : (b == 2 ? x : y));
	p = (b5 == b3 ? -p : p);
	q = (b5 == b4 ? -q : q);
	r = (b5 != (b4^b3) ? -r : r);
	t *= t;
	return 8.0 * t * t * (p + (b == 0 ? q + r : (b2 == 0 ? q : r)));
}

int C3dSimplexNoise::Shuffle(int i, int j, int k)
{
	return 
		B(i, j, k, 0) + B(j, k, i, 1) + B(k, i, j, 2) + B(i, j, k, 3) +
		B(j, k, i, 4) + B(k, i, j, 5) + B(i, j, k, 6) + B(j, k, i, 7);
}

int C3dSimplexNoise::B(int i, int j, int k, int b)
{
	return m_t[(B(i, b) << 2) | (B(j, b) << 1) | B(k, b)];
}

int C3dSimplexNoise::B(int n, int b)
{ 
	return (n >> b) & 1;
}

double SimplexNoise(double x, double y, double z)
{
	C3dSimplexNoise sn;
	return sn.Noise(x, y, z);
}

float Catrom2(float d)
{
	#define SAMPRATE 100  /* table entries per unit distance */
	#define NENTRIES (4*SAMPRATE+1)

	float x;
	int i;
	static float table[NENTRIES];
	static bool bInitialized = false;

	if (d >= 4.0f)
		return 0;

	if (!bInitialized)
	{
		for (i = 0; i < NENTRIES; i++)
		{
			x = (float)i / (float)SAMPRATE;
			x = sqrtf(x);
			if (x < 1.0f)
				table[i] = 0.5f * (2.0f + x * x * (-5.0f + x * 3.0f));
			else
				table[i] = 0.5f * (4.0f + x * (-8.0f + x * (5.0f - x)));
		}
		bInitialized = true;
	}

	d = d * (float)SAMPRATE + 0.5f;
	i = (int)floor(d);

	if (i >= NENTRIES)
		return 0;

	return table[i];
}

#define SCNPERM(x)          NoisePerm[(x) & (NOISE_PERM_SIZE - 1)]
#define SCNINDEX(ix, iy, iz)  SCNPERM((ix) + SCNPERM((iy) + SCNPERM(iz)))

#define SCNNEXT(h)     (((h)+1) & (NOISE_PERM_SIZE - 1))
#define SCNNIMPULSES   3

static float pImpulseTab[NOISE_PERM_SIZE * 4];

static void ImpulseTabInit(ULONG seed);

float SCNoise(float x, float y, float z)
{
    static bool bInitialized = false;
    float *fp;
    int i, j, k, h, n;
    int ix, iy, iz;
    float sum = 0;
    float fx, fy, fz, dx, dy, dz, distsq;

    /* Initialize the random impulse table if necessary. */
    if (!bInitialized) {
        ImpulseTabInit(665);
        bInitialized = true;
    }

    ix = (int)floor(x); fx = x - (float)ix;
    iy = (int)floor(y); fy = y - (float)iy;
    iz = (int)floor(z); fz = z - (float)iz;
    
    /* Perform the sparse convolution. */
    for (i = -2; i <= 2; i++)
	{
		for (j = -2; j <= 2; j++)
		{
			for (k = -2; k <= 2; k++)
			{
				/* Compute voxel hash code. */
				h = SCNINDEX(ix + i, iy + j, iz + k);
			    
				for (n = SCNNIMPULSES; n > 0; n--, h = SCNNEXT(h))
				{
					/* Convolve filter and impulse. */
					fp = &pImpulseTab[h * 4];
					dx = fx - ((float)i + *fp++);
					dy = fy - ((float)j + *fp++);
					dz = fz - ((float)k + *fp++);
					distsq = dx * dx + dy * dy + dz * dz;
					sum += Catrom2(distsq) * *fp;
				}
			}
		}
    }

    return sum / (float)SCNNIMPULSES;
}

static void ImpulseTabInit(ULONG seed)
{
    int i;
    float *f = pImpulseTab;

	CLBPMersenneTwister mt(seed); /* Set random number generator seed. */
    for (i = 0; i < NOISE_PERM_SIZE; i++) {
        *f++ = mt.RandomFloat1();
        *f++ = mt.RandomFloat1();
        *f++ = mt.RandomFloat1();
        *f++ = 1.0f - 2.0f * mt.RandomFloat1();
    }
}

#define VCNPERM(x)				NoisePerm[(x) & (NOISE_PERM_SIZE - 1)]
#define VCNINDEX(ix, iy, iz)	VCNPERM((ix) + VCNPERM((iy) + VCNPERM(iz)))

static float VCNValueTab[NOISE_PERM_SIZE];

static void VCNValueTabInit(int seed)
{
    float *table = VCNValueTab;
    int i;

    CLBPMersenneTwister mt(seed);
    for(i = 0; i < NOISE_PERM_SIZE; i++)
        *table++ = 1.0f - 2.0f * mt.RandomFloat1();
}

float VCNoise(float x, float y, float z)
{
    int ix, iy, iz;
    int i, j, k;
    float fx, fy, fz;
    float dx, dy, dz;
    float sum = 0;
    static bool bInitialized = false;

    if (!bInitialized) {
        VCNValueTabInit(665);
        bInitialized = true;
    }

    ix = (int)floor(x);
    fx = x - ix;

    iy = (int)floor(y);
    fy = y - iy;

    iz = (int)floor(z);
    fz = z - iz;

    for (k = -1; k <= 2; k++)
	{
        dz = k - fz;
        dz = dz*dz;
        for (j = -1; j <= 2; j++)
		{
            dy = j - fy;
            dy = dy*dy;
            for (i = -1; i <= 2; i++)
			{
                dx = i - fx;
                dx = dx*dx;
                sum += VCNValueTab[VCNINDEX(ix + i, iy + j, iz + k)]
                    * Catrom2(dx + dy + dz);
            }
        }
    }
    return sum;
}

inline double WHN_frand(int seed)
{
	seed = seed << (13 ^ seed);
	return (1.0 - (double)((int)(seed * (seed * seed * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

inline double WHN_rand3a(int x, int y, int z) { return WHN_frand(67 * x + 59 * y + 71 * z); }
inline double WHN_rand3b(int x, int y, int z) { return WHN_frand(73 * x + 79 * y + 83 * z); }
inline double WHN_rand3c(int x, int y, int z) {	return WHN_frand(89 * x + 97 * y + 101 * z); }
inline double WHN_rand3d(int x, int y, int z) { return WHN_frand(103 * x + 107 * y + 109 * z); }

inline double WHN_hpoly1(double t) { return ((2.0 * t - 3.0) * t * t + 1.0); }
inline double WHN_hpoly2(double t) { return (-2.0 * t + 3.0) * t * t; }
inline double WHN_hpoly3(double t) { return ((t - 2.0) * t + 1.0) * t; }
inline double WHN_hpoly4(double t) { return (t - 1.0) * t * t; }

void WHN_interpolate(double * f, int i, int n, int xlim[3][2], double * pXarg)
{
	double  f0[4], f1[4], hp1, hp2;

	if (n == 0)
	{
		f[0] = WHN_rand3a(xlim[0][i & 1], xlim[1][(i >> 1) & 1], xlim[2][i >> 2]);
		f[1] = WHN_rand3b(xlim[0][i & 1], xlim[1][(i >> 1) & 1], xlim[2][i >> 2]);
		f[2] = WHN_rand3c(xlim[0][i & 1], xlim[1][(i >> 1) & 1], xlim[2][i >> 2]);
		f[3] = WHN_rand3d(xlim[0][i & 1], xlim[1][(i >> 1) & 1], xlim[2][i >> 2]);
	} 
	else 
	{
		n--;
		WHN_interpolate(f0, i, n, xlim, pXarg);
		WHN_interpolate(f1, (i | 1) << n, n, xlim, pXarg);
		hp1 = WHN_hpoly1(pXarg[n]);
		hp2 = WHN_hpoly2(pXarg[n]);
		f[0] = f0[0]*hp1 + f1[0]*hp2;
		f[1] = f0[1]*hp1 + f1[1]*hp2;
		f[2] = f0[2]*hp1 + f1[2]*hp2;
		f[3] = f0[3]*hp1 + f1[3]*hp2 + f0[n]*WHN_hpoly3(pXarg[n]) + f1[n]*WHN_hpoly4(pXarg[n]);
	}
}

double WardsHermiteNoise(double x, double y, double z)
{
	int xlim[3][2];
	double xarg[3];

	double xnew[3];
	xnew[0] = x;
	xnew[1] = y;
	xnew[2] = z;
	double  oldx[3] = {-100000.0, -100000.0, -100000.0};
	double  f[4];
	f[0] = f[1] = f[2] = f[3] = 0.0;

	if (oldx[0] == xnew[0] && oldx[1] == xnew[1] && oldx[2] == xnew[2])
		return f[3];
	oldx[0] = xnew[0]; oldx[1] = xnew[1]; oldx[2] = xnew[2];
	xlim[0][0] = (int)floor(oldx[0]); xlim[0][1] = xlim[0][0] + 1;
	xlim[1][0] = (int)floor(oldx[1]); xlim[1][1] = xlim[1][0] + 1;
	xlim[2][0] = (int)floor(oldx[2]); xlim[2][1] = xlim[2][0] + 1;
	xarg[0] = oldx[0] - xlim[0][0];
	xarg[1] = oldx[1] - xlim[1][0];
	xarg[2] = oldx[2] - xlim[2][0];
	WHN_interpolate(f, 0, 3, xlim, xarg);
	return f[3];
}

static const float ANsquaredRadius[16] =
{ 1.5000000f, 1.8262500f, 2.1524999f, 2.4787498f, 2.8049998f, 3.1312499f, 1.7175000f, 2.0437498f,
  2.3699999f, 2.6962500f, 3.0224998f, 1.6087500f, 1.9349999f, 2.2612500f, 2.5874999f, 2.9137497f };

static const float ANinverseOfSquaredRadius[16] =
{ 0.66666669f, 0.54757017f, 0.46457610f, 0.40342918f, 0.35650626f, 0.31936130f, 0.58224165f, 0.48929667f,
  0.42194095f, 0.37088549f, 0.33085197f, 0.62160063f, 0.51679587f, 0.44223326f, 0.38647345f, 0.34320039f };

static const int ANpermTable[512] =
{ 21, 228, 191, 35, 248, 101, 162, 155, 232, 165, 223, 166, 120, 40, 110, 222, 36, 148, 247, 226, 136,
  249, 55, 43, 100, 221, 79, 207, 208, 189, 15, 179, 244, 61, 167, 46, 89, 161, 174, 186, 17, 197, 178, 
  198, 12, 169, 246, 86, 229, 64, 151, 66, 104, 49, 214, 238, 204, 88, 94, 54, 92, 164, 130, 158, 212,
  201, 62, 206, 33, 205, 235, 39, 133, 200, 83, 50, 44, 9, 11, 99, 121, 160, 47, 10, 145, 0, 163, 118,
  172, 117, 123, 199, 125, 216, 71, 243, 233, 1, 170, 59, 85, 173, 19, 115, 37, 32, 38, 210, 24, 13, 171,
  195, 253, 209, 75, 30, 69, 196, 3, 22, 177, 124, 103, 111, 29, 168, 231, 95, 140, 112, 34, 114, 181, 193,
  98, 159, 45, 41, 6, 107, 56, 237, 67, 219, 157, 109, 119, 135, 116, 185, 90, 183, 93, 176, 150, 143, 52,
  217, 122, 51, 53, 8, 2, 87, 153, 156, 215, 58, 129, 5, 102, 70, 149, 72, 127, 254, 4, 81, 23, 147, 113,
  152, 18, 42, 144, 76, 91, 182, 48, 184, 154, 194, 84, 73, 126, 218, 57, 77, 14, 27, 220, 60, 106, 255,
  180, 137, 139, 175, 240, 96, 78, 138, 236, 128, 227, 202, 108, 245, 251, 190, 65, 132, 7, 230, 192, 80,
  31, 187, 252, 97, 211, 63, 188, 241, 74, 234, 224, 105, 142, 82, 68, 141, 146, 203, 20, 16, 131, 250, 25,
  213, 242, 239, 225, 28, 26, 134, 21, 228, 191, 35, 248, 101, 162, 155, 232, 165, 223, 166, 120, 40, 110,
  222, 36, 148, 247, 226, 136, 249, 55, 43, 100, 221, 79, 207, 208, 189, 15, 179, 244, 61, 167, 46, 89, 161,
  174, 186, 17, 197, 178, 198, 12, 169, 246, 86, 229, 64, 151, 66, 104, 49, 214, 238, 204, 88, 94, 54, 92,
  164, 130, 158, 212, 201, 62, 206, 33, 205, 235, 39, 133, 200, 83, 50, 44, 9, 11, 99, 121, 160, 47, 10,
  145, 0, 163, 118, 172, 117, 123, 199, 125, 216, 71, 243, 233, 1, 170, 59, 85, 173, 19, 115, 37, 32, 38,
  210, 24, 13, 171, 195, 253, 209, 75, 30, 69, 196, 3, 22, 177, 124, 103, 111, 29, 168, 231, 95, 140, 112,
  34, 114, 181, 193, 98, 159, 45, 41, 6, 107, 56, 237, 67, 219, 157, 109, 119, 135, 116, 185, 90, 183, 93,
  176, 150, 143, 52, 217, 122, 51, 53, 8, 2, 87, 153, 156, 215, 58, 129, 5, 102, 70, 149, 72, 127, 254, 4,
  81, 23, 147, 113, 152, 18, 42, 144, 76, 91, 182, 48, 184, 154, 194, 84, 73, 126, 218, 57, 77, 14, 27,
  220, 60, 106, 255, 180, 137, 139, 175, 240, 96, 78, 138, 236, 128, 227, 202, 108, 245, 251, 190, 65,
  132, 7, 230, 192, 80, 31, 187, 252, 97, 211, 63, 188, 241, 74, 234, 224, 105, 142, 82, 68, 141, 146,
  203, 20, 16, 131, 250, 25, 213, 242, 239, 225, 28, 26, 134 };

float AaltonenNoise(float x, float y, float z)
{
	#define maxRadius 1.8f
	#define maxSquaredRadius ((float)maxRadius * (float)maxRadius)
	#define minSquaredRadius 1.5f

	const int sx = (int)ceil(x - maxRadius);
	const int ex = (int)floor(x + maxRadius);
	if (ex >= sx + 4 || sx > ex)
		return 0.0f;

	const int sy = (int)ceil(y - maxRadius);
	const int ey = (int)floor(y + maxRadius);
	if (ey >= sy + 4 || sy > ey)
		return 0.0f;

	const int sz = (int)ceil(z - maxRadius);
	const int ez = (int)floor(z + maxRadius);
	if (ez >= sz + 4 || sz > ez)
		return 0.0f;

	int j = 0;

	float ySqDistTable[2 + (int)maxRadius * 2];
	int idy = 0;
	for (j = sy; j <= ey; j++)
	{
		const float dy = j - y;
		ySqDistTable[idy++] = dy * dy;
	}

	float xSqDistTable[2 + (int)maxRadius * 2];
	int idx = 0;
	for (j = sx; j <= ex; j++)
	{
		const float dx = j - x;
		xSqDistTable[idx++] = dx * dx;
	}

	float result = 0.0;
	for (int iz = sz; iz <= ez; iz++)
	{
		const float dz = iz - z;
		const float sqdz = dz * dz;
		idy = 0;
		const int permZ = ANpermTable[iz & 255];
		for (int iy = sy; iy <= ey; iy++)
		{
			const float sqdydz = ySqDistTable[idy++] + sqdz;
			idx = 0;
			const int permY = ANpermTable[permZ + (iy & 255)];
			for (int ix = sx; ix <= ex; ix++)
			{
				const int prn = ANpermTable[permY + (ix & 255)];
				const float dsq = xSqDistTable[idx++] + sqdydz;
				if (ANsquaredRadius[prn & 15] >= dsq)
				{
					const float t = 1.0f - dsq * ANinverseOfSquaredRadius[prn & 15];
					if ((prn & 128) == 128)
						result -= t;
					else
						result += t;
				}
			}
		}
	}

	return result / 10.333334f;
}


// Texture projection conversions

void EquirectToWorld(float u, float v, ON_3fVector& vec)
{
	const float theta =  (u - 0.5f) * (float)ON_PI * 2.0f;
	const float phi   = -(v - 0.5f) * (float)ON_PI;

	const float cosphi = cos(phi);
	vec.x = sin(theta) * cosphi;
	vec.y = sin(phi);
	vec.z = cos(theta) * cosphi;
}

void WorldToEquirect(const ON_3fVector& vec, float& u, float& v)
{
	double theta, phi;
	ON_Sphere sphere;
	sphere.radius = 1.0;
	ON_3dPoint pt(-vec.z, -vec.x, vec.y);
	sphere.ClosestPointTo(pt, &theta, &phi);

	u = (float)(theta / (2.0 * ON_PI));
	v = (float)((-phi + (0.5 * ON_PI)) / ON_PI);
}

void HemisphericalToWorld(float u, float v, ON_3fVector& vec)
{
	EquirectToWorld(u, 0.5f + 0.5f * v, vec);
}

void WorldToHemispherical(const ON_3fVector& vec, float& u, float& v)
{
	ON_3fVector vtHemiVec(vec.x, min(vec.y, 0.0f), vec.z);
	vtHemiVec.Unitize();
	WorldToEquirect(vtHemiVec, u, v);
	v = 2.0f * v - 1.0f;
}

bool LightProbeToWorld(float u, float v, ON_3fVector& vec)
{
	const float d = sqrt((0.5f-u)*(0.5f-u)+(0.5f-v)*(0.5f-v))*2.0f;
	if (d < FLT_MIN)
	{
		vec.Set(0.0f, 0.0f, 1.0f);
		return true;
	}
	else if (d >= 1.0f)
	{
		vec.Set(1.0f, 0.0f, 0.0f);
		return false;
	}
	const float cosine = cos((float)ON_PI*d);
	const float factor = 2.0f * sqrt(1.0f-cosine*cosine);

	vec.x = (u - 0.5f) * factor / d;
	vec.y = (0.5f - v) * factor / d;
	vec.z = cosine;
	return true;
}

void WorldToLightProbe(const ON_3fVector& vec, float& u, float& v)
{
	float fDivisor = sqrt(vec.x * vec.x + vec.y * vec.y);
	if (fDivisor < FLT_MIN)
		fDivisor = FLT_MIN;

	const float f = (acos(vec.z) / (float)ON_PI) / fDivisor;

	const float px = vec.x * f;
	const float py = vec.y * f;

	u = ( px + 1.0f) * 0.5f;
	v = (-py + 1.0f) * 0.5f;
}

void CubemapToWorld(float u, float v, ON_3fVector& vecOut)
{
	const float uCube = u - floor(u);
	const float vCube = v - floor(v);
	const int subTextureIndex = (int)floor(uCube * 6.0f);
	const float subTextureOffset = (float)subTextureIndex / 6.0f;
	const float uSubTex = (uCube - subTextureOffset) * 6.0f;
	const float vSubTex = vCube;
	const float sc = 2.0f * uSubTex - 1.0f;
	const float tc = 2.0f * vSubTex - 1.0f;

	vecOut.x = 0.0f;
	vecOut.y = 0.0f;
	vecOut.z = 0.0f;

	switch (subTextureIndex)
	{
	case 0:
		vecOut.z = -sc;
		vecOut.y = -tc;
		vecOut.x =  1.0f;
		break;
	case 1:
		vecOut.z =  sc;
		vecOut.y = -tc;
		vecOut.x = -1.0f;
		break;
	case 2:
		vecOut.x =  sc;
		vecOut.z = -tc;
		vecOut.y = -1.0f;
		break;
	case 3:
		vecOut.x =  sc;
		vecOut.z =  tc;
		vecOut.y =  1.0f;
		break;
	case 4:
		vecOut.x =  sc;
		vecOut.y = -tc;
		vecOut.z =  1.0f;
		break;
	case 5:
		vecOut.x = -sc;
		vecOut.y = -tc;
		vecOut.z = -1.0f;
		break;
	}

	vecOut /= (float)vecOut.Length();
}

void WorldToCubemap(const ON_3fVector& vec, float& u, float& v)
{
	static const int subTextureIndexLU[3][2] = { {0, 1}, {3, 2}, {4, 5} };
	const int mainAxis = vec.MaximumCoordinateIndex();
	const float mainAxisDir = vec[mainAxis];
	const int subTextureIndex = subTextureIndexLU[mainAxis][mainAxisDir >= 0.0f ? 0 : 1];
	const float subTextureOffset = (float)subTextureIndex / 6.0f;
	const float ma = std::abs(mainAxisDir);
	float sc = 0.0f;
	float tc = 0.0f;
	switch (subTextureIndex)
	{
	case 0:
		sc = -vec.z;
		tc = -vec.y;
		break;
	case 1:
		sc = vec.z;
		tc = -vec.y;
		break;
	case 2:
		sc = vec.x;
		tc = -vec.z;
		break;
	case 3:
		sc = vec.x;
		tc = vec.z;
		break;
	case 4:
		sc = vec.x;
		tc = -vec.y;
		break;
	case 5:
		sc = -vec.x;
		tc = -vec.y;
		break;
	}

	if (ma > ON_EPSILON)
	{
		u = (sc / ma  + 1.0f) / 12.0f + subTextureOffset;
		v = (tc / ma  + 1.0f) / 2.0f;
	}
	else
		u = v = 0.0f;
}

bool VerticalCrossCubemapToWorld(float u, float v, ON_3fVector& vecOut)
{
	float uCube = u - floor(u);
	float vCube = v - floor(v);

	// Add small colored border around the cross

	const float minU = 1.0f / 3.0f + 1e-6f;
	const float maxU = 2.0f / 3.0f - 1e-6f;
	const float minV = 2.0f / 4.0f + 1e-6f;
	const float maxV = 3.0f / 4.0f - 1e-6f;
	const float borderU = 1.0f / 360.0f;
	const float borderV = 1.0f / 360.0f;

	const float uLtd = (uCube < minU ? minU : (maxU < uCube ? maxU : uCube));
	const float vLtd = (vCube < minV ? minV : (maxV < vCube ? maxV : vCube));
	const float uAdj = std::abs(uLtd - uCube);
	const float vAdj = std::abs(vLtd - vCube);

	if (uAdj != 0.0 && vAdj != 0.0)
	{
		if (uAdj <= vAdj && uAdj < borderU)
		{
			uCube = uLtd;
		}
		else if (vAdj <= uAdj && vAdj < borderV)
		{
			vCube = vLtd;
		}
	}


	const int subTexU = (int)floor(3.0f * uCube);
	const int subTexV = (int)floor(4.0f * vCube);
	const float subTextureStartU = (float)subTexU / 3.0f;
	const float subTextureStartV = (float)subTexV / 4.0f;
	const float uSubTex = 3.0f * (uCube - subTextureStartU);
	const float vSubTex = 4.0f * (vCube - subTextureStartV);
	const float sc = 2.0f * uSubTex - 1.0f;
	const float tc = 2.0f * vSubTex - 1.0f;

	int subTextureIndex = -1;
	if (subTexU == 0)
	{
		if (subTexV == 2)
			subTextureIndex = 1;
	}
	else
	{
		if (subTexU == 1)
		{
			if (subTexV == 0)
				subTextureIndex = 5;
			else if (subTexV == 1)
				subTextureIndex = 2;
			else if (subTexV == 2)
				subTextureIndex = 4;
			else if (subTexV == 3)
				subTextureIndex = 3;
		}
		else
		{
			if (subTexU == 2)
			{
				if (subTexV == 2)
					subTextureIndex = 0;
			}
		}
	}

	if (subTextureIndex == -1)
		return false;

	vecOut.x = 0.0f;
	vecOut.y = 0.0f;
	vecOut.z = 0.0f;

	switch (subTextureIndex)
	{
	case 0:
		vecOut.z = -sc;
		vecOut.y = -tc;
		vecOut.x =  1.0f;
		break;
	case 1:
		vecOut.z =  sc;
		vecOut.y = -tc;
		vecOut.x = -1.0f;
		break;
	case 2:
		vecOut.x =  sc;
		vecOut.z =  tc;
		vecOut.y =  1.0f;
		break;
	case 3:
		vecOut.x =  sc;
		vecOut.z = -tc;
		vecOut.y = -1.0f;
		break;
	case 4:
		vecOut.x =  sc;
		vecOut.y = -tc;
		vecOut.z =  1.0f;
		break;
	case 5:
		vecOut.x = sc;
		vecOut.y = tc;
		vecOut.z = -1.0f;
		break;
	}

	vecOut /= (float)vecOut.Length();

	return true;
}

void WorldToVerticalCrossCubemap(const ON_3fVector& vec, float& u, float& v)
{
	const int mainAxis = vec.MaximumCoordinateIndex();
	const float mainAxisDir = vec[mainAxis];
	const int subTextureIndex = (mainAxis << 1) + (mainAxisDir >= 0.0f ? 0 : 1);

	const float uSubTexStart = subTextureIndex == 1 ? 0.0f : (subTextureIndex == 0 ? 2.0f / 3.0f : 1.0f / 3.0f);
	const float vSubTexStart = subTextureIndex == 5 ? 0.0f : (subTextureIndex == 2 ? 1.0f / 4.0f : (subTextureIndex == 3 ? 3.0f / 4.0f : 2.0f / 4.0f));
	const float ma = std::abs(mainAxisDir);
	float sc = 0.0f;
	float tc = 0.0f;
	switch (subTextureIndex)
	{
	case 0:
		sc = -vec.z;
		tc = -vec.y;
		break;
	case 1:
		sc = vec.z;
		tc = -vec.y;
		break;
	case 2:
		sc = vec.x;
		tc = vec.z;
		break;
	case 3:
		sc = vec.x;
		tc = -vec.z;
		break;
	case 4:
		sc = vec.x;
		tc = -vec.y;
		break;
	case 5:
		sc = vec.x;
		tc = vec.y;
		break;
	}

	if (ma > ON_EPSILON)
	{
		u = (sc / ma  + 1.0f) / 6.0f + uSubTexStart;
		v = (tc / ma  + 1.0f) / 8.0f + vSubTexStart;
	}
	else
		u = v = 0.0f;
}

bool HorizontalCrossCubemapToWorld(float u, float v, ON_3fVector& vecOut)
{
	float uCube = u - floor(u);
	float vCube = v - floor(v);


	// Add small colored border around the cross

	const float minU = 1.0f / 4.0f + 1e-6f;
	const float maxU = 2.0f / 4.0f - 1e-6f;
	const float minV = 1.0f / 3.0f + 1e-6f;
	const float maxV = 2.0f / 3.0f - 1e-6f;
	const float borderU = 1.0f / 360.0f;
	const float borderV = 1.0f / 360.0f;

	const float uLtd = (uCube < minU ? minU : (maxU < uCube ? maxU : uCube));
	const float vLtd = (vCube < minV ? minV : (maxV < vCube ? maxV : vCube));
	const float uAdj = std::abs(uLtd - uCube);
	const float vAdj = std::abs(vLtd - vCube);

	if (uAdj != 0.0 && vAdj != 0.0)
	{
		if (uAdj <= vAdj && uAdj < borderU)
		{
			uCube = uLtd;
		}
		else if (vAdj <= uAdj && vAdj < borderV)
		{
			vCube = vLtd;
		}
	}

	const int subTexU = (int)floor(4.0f * uCube);
	const int subTexV = (int)floor(3.0f * vCube);
	const float subTextureStartU = (float)subTexU / 4.0f;
	const float subTextureStartV = (float)subTexV / 3.0f;
	const float uSubTex = 4.0f * (uCube - subTextureStartU);
	const float vSubTex = 3.0f * (vCube - subTextureStartV);
	const float sc = 2.0f * uSubTex - 1.0f;
	const float tc = 2.0f * vSubTex - 1.0f;

	int subTextureIndex = -1;
	if (subTexV == 0)
	{
		if (subTexU == 1)
			subTextureIndex = 2;
	}
	else
	{
		if (subTexV == 1)
		{
			if (subTexU == 0)
				subTextureIndex = 1;
			else if (subTexU == 1)
				subTextureIndex = 4;
			else if (subTexU == 2)
				subTextureIndex = 0;
			else if (subTexU == 3)
				subTextureIndex = 5;
		}
		else
		{
			if (subTexV == 2)
			{
				if (subTexU == 1)
					subTextureIndex = 3;
			}
		}
	}

	if (subTextureIndex == -1)
		return false;

	vecOut.x = 0.0f;
	vecOut.y = 0.0f;
	vecOut.z = 0.0f;

	switch (subTextureIndex)
	{
	case 0:
		vecOut.z = -sc;
		vecOut.y = -tc;
		vecOut.x =  1.0f;
		break;
	case 1:
		vecOut.z =  sc;
		vecOut.y = -tc;
		vecOut.x = -1.0f;
		break;
	case 2:
		vecOut.x =  sc;
		vecOut.z =  tc;
		vecOut.y =  1.0f;
		break;
	case 3:
		vecOut.x =  sc;
		vecOut.z = -tc;
		vecOut.y = -1.0f;
		break;
	case 4:
		vecOut.x =  sc;
		vecOut.y = -tc;
		vecOut.z =  1.0f;
		break;
	case 5:
		vecOut.x = -sc;
		vecOut.y = -tc;
		vecOut.z = -1.0f;
		break;
	}

	vecOut /= (float)vecOut.Length();

	return true;
}

void WorldToHorizontalCrossCubemap(const ON_3fVector& vec, float& u, float& v)
{
	const int mainAxis = vec.MaximumCoordinateIndex();
	const float mainAxisDir = vec[mainAxis];
	const int subTextureIndex = (mainAxis << 1) + (mainAxisDir >= 0.0f ? 0 : 1);

	const float uSubTexStart = subTextureIndex == 1 ? 0.0f : (subTextureIndex == 0 ? 2.0f / 4.0f : (subTextureIndex == 5 ? 3.0f / 4.0f : 1.0f / 4.0f));
	const float vSubTexStart = subTextureIndex == 2 ? 0.0f : (subTextureIndex == 3 ? 2.0f / 3.0f : 1.0f / 3.0f);
	const float ma = std::abs(mainAxisDir);
	float sc = 0.0f;
	float tc = 0.0f;
	switch (subTextureIndex)
	{
	case 0:
		sc = -vec.z;
		tc = -vec.y;
		break;
	case 1:
		sc = vec.z;
		tc = -vec.y;
		break;
	case 2:
		sc = vec.x;
		tc = vec.z;
		break;
	case 3:
		sc = vec.x;
		tc = -vec.z;
		break;
	case 4:
		sc = vec.x;
		tc = -vec.y;
		break;
	case 5:
		sc = -vec.x;
		tc = -vec.y;
		break;
	}

	if (ma > ON_EPSILON)
	{
		u = (sc / ma  + 1.0f) / 8.0f + uSubTexStart;
		v = (tc / ma  + 1.0f) / 6.0f + vSubTexStart;
	}
	else
		u = v = 0.0f;
}

bool EmapToWorld(float u, float v, ON_3fVector& vec)
{
	const float x =   2.0f * u - 1.0f;
	const float y = -(2.0f * v - 1.0f);

	const float r = sqrt(x * x + y * y);
	if (r > 1.0f)
		return false;

	const float t = 2.0f * asin(r);
	const float divisor = (r < FLT_MIN) ? FLT_MIN : r;
	const float s = sin(t) / divisor;

	vec.x = -s * x;
	vec.y = s * y;
	vec.z = cos(t);

	return true;
}

void WorldToEmap(const ON_3fVector& vec, float& u, float& v)
{
	float fDivisor = sqrt((vec.x * vec.x) + (vec.y * vec.y));
	if (fDivisor < FLT_MIN)
		fDivisor = FLT_MIN;

	const float f = sin(0.5f * acos(vec.z)) / fDivisor;

	const float px = -vec.x * f;
	const float py = vec.y * f;

	u = (1.0f + px) * 0.5f;
	v = (1.0f - py) * 0.5f;
}

bool AdjustColor(CRhRdkColor& color, double gamma, double gain, double multiplier,
                 bool bClamp, double clampMin, double clampMax, bool bScaleToClamp, 
				 double hueShift, double saturation, bool bInvert, bool bTreatAsHDR, bool bGrayscale)
{
	// Extract color components
	float red   = color.FRed();
	float green = color.FGreen();
	float blue  = color.FBlue();
	float alpha = color.FAlpha();

	//Anything could be coming out of the renderer, so make sure we don't have to deal with screwy numbers first
	if (_isnan(red) || _isnan(green) || _isnan(blue) || _isnan(alpha))
		return false;

	// Convert rgb into hsb
	CRhRdkColor tempColor(red, green, blue, alpha);
	float h = 0.0f, s = 0.0f, b = 0.0f;
	if (!rgb2hsb(tempColor, h, s, b))
		return false;

	// Adjust brightness by gain
	if (bTreatAsHDR)
	{
		//Gain does not work on HDR images.
	}
	else
	{
		b = Gain(b, (float)gain);
	}

	// Apply clamping
	if (bClamp)
	{
		if (b > clampMax)
			b = (float)clampMax;
		if (b < clampMin)
			b = (float)clampMin;

		if (bScaleToClamp)
			b = (float)((b - clampMin) / (clampMax - clampMin));
	}

	// Convert hsb to rgb

	// Hue value in 'h' is in degrees and hue shift value in 'm_hueShift' is in radians.
	// hsb2rgb wants hue parameter to be in degrees. In addition it should be in the range
	// [0,360). Compute shifted hue value in degrees and make sure it is in the desired range.
	// If the hue value is a tiny amount less than 0 or a tiny amount more than 360 then it
	// is the same tiny amount away from 0 and it so can be set to 0.
	float hueIn = h + (float)(hueShift * 360.0 / (2.0 * ON_PI));
	while (hueIn >= 360.0f) { hueIn -= 360.0f; }
	while (hueIn < 0.0f) { hueIn += 360.0f; }
	if (hueIn < 0.0f || hueIn >= 360.0f)
		hueIn = 0.0f;

	if (!hsb2rgb(hueIn, s * (float)saturation, b, tempColor))
		return false;

	red = tempColor.FRed();
	green = tempColor.FGreen();
	blue = tempColor.FBlue();
	alpha = tempColor.FAlpha();

	// Adjust gamma
	if (!LBPIsDoubleEqual(gamma, 1.0))
	{
		gamma = max(0.2, min(5.0, gamma));
		const float fGamma = (float)(1.0 / gamma);
		red   = powf(max(0.0f, red),   fGamma);
		green = powf(max(0.0f, green), fGamma);
		blue  = powf(max(0.0f, blue),  fGamma);
	}

	// Invert
	if (bInvert)
	{
		RHRDK_ASSERT(0 <= red);
		RHRDK_ASSERT(0 <= green);
		RHRDK_ASSERT(0 <= blue);

		if (bTreatAsHDR)
		{
			// For HDR colors we apply multiplicative inversion
			red = max(red, FLT_EPSILON);
			green = max(green, FLT_EPSILON);
			blue = max(blue, FLT_EPSILON);

			color.Set(
				1.0f / red,
				1.0f / green,
				1.0f / blue,
				alpha);
		}
		else
		{
			// For normal colors we apply additive inversion
			color.Set(
				max(0.0f, 1.0f - red),
				max(0.0f, 1.0f - green),
				max(0.0f, 1.0f - blue),
				alpha);
		}
	}
	else
	{
		color.Set(red, green, blue, alpha);
	}

	if (bGrayscale)
	{
		const float fGray = color.Luminance();
		color.Set(fGray, fGray, fGray, color.FAlpha());
	}

	// Apply multiplier
	color *= (float)multiplier;

	// Everything went fine
	return true;
}

bool TextureIsOn(const CRhRdkMaterial& material, const wchar_t* sChildSlotName)
{
	const ON_wString sParamName = material.ParamNameFromChildSlotName(sChildSlotName);
	
	CRhRdkVariant vOn, vAmount;
	if (!material.GetExtraRequirementParameter(sParamName, RDK_TEXTURE_ON, vOn))
		return true;
	if (!material.GetExtraRequirementParameter(sParamName, RDK_TEXTURE_AMOUNT, vAmount))
		return vOn.AsBool();

	return vOn.AsBool() && vAmount.AsFloat() > 0.0f;
}

bool IsHDRFile(const wchar_t* wszFilename)
{
	const CLBPString sExt = CLBPFileMgr2::GetExtension(wszFilename);
	if ((L"hdr" == sExt) || (L"hdri" == sExt))
		return true;

	return false;
}

bool IsEXRFile(const wchar_t* wszFilename)
{
	const CLBPString sExt = CLBPFileMgr2::GetExtension(wszFilename);
	if (L"exr" == sExt)
		return true;

	return false;
}

CRhRdkHighDynamicRangeImageEx_RGB* NewHighDynamicRangeImage(const wchar_t* wszFilename, bool bAllocate)
{
	if (IsHDRFile(wszFilename))
	{
		return new CRhRdkHDRImage(bAllocate);
	}
	else
	if (IsEXRFile(wszFilename))
	{
		return new CRhRdkEXRImage(bAllocate);
	}

	return nullptr;
}
