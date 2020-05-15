
#include "stdafx.h"
#include "RhRdkGridTexture.h"

// CRhRdkGridTextureEvaluator

class CRhRdkGridTextureEvaluator : public CRhRdkTwoColorEvaluator
{
public:
	CRhRdkGridTextureEvaluator(const CRhRdkGridTexture& texture, CEvalFlags ef);
	~CRhRdkGridTextureEvaluator();

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const;

protected:
	int m_iCells;
	double m_dFontThickness;
};

CRhRdkGridTextureEvaluator::CRhRdkGridTextureEvaluator(const CRhRdkGridTexture& texture, CEvalFlags ef)
	:
	CRhRdkTwoColorEvaluator(texture, ef, false),
	m_iCells(texture.Cells()),
	m_dFontThickness(texture.FontThickness())
{
}

CRhRdkGridTextureEvaluator::~CRhRdkGridTextureEvaluator()
{
}

static int GetSegment(double u, double v, double thickness, double margin)
{
	if (v > 0.5)
	{
		return (8 - GetSegment(u, 1.0 - v, thickness, margin)) % 8;
	}
	else
	{
		if (u < 2.0 * margin ||
			1.0 - 2.0 * margin < u ||
			v < margin)
		{
			return 0;
		}

		if (u > 2.0 * (margin + thickness) &&
			u < 1.0 - 2.0 * (margin + thickness) &&
			v > margin + thickness &&
			v < 0.5 - 0.5 * thickness)
		{
			return 0;
		}

		if (u - 2.0 * v > 0.0 && u + 2.0 * v < 1.0)
		{
			return 1;
		}

		if (0.25 * u + v > 0.5 * margin + 0.5 &&
			0.25 * u - v < -0.5 * margin - 0.25)
		{
			return 4;
		}

		if (u < 0.5)
		{
			return 2;
		}

		return 3;
	}
}

static int segLU[10][8] =
{
	{ 0, 2, 2, 2, 1, 2, 2, 2 }, // 0
	{ 0, 1, 1, 2, 1, 2, 1, 1 }, // 1
	{ 0, 2, 1, 2, 2, 1, 2, 2 }, // 2
	{ 0, 2, 1, 2, 2, 2, 1, 2 }, // 3
	{ 0, 1, 2, 2, 2, 2, 1, 1 }, // 4
	{ 0, 2, 2, 1, 2, 2, 1, 2 }, // 5
	{ 0, 2, 2, 1, 2, 2, 2, 2 }, // 6
	{ 0, 2, 1, 2, 1, 2, 1, 1 }, // 7
	{ 0, 2, 2, 2, 2, 2, 2, 2 }, // 8
	{ 0, 2, 2, 2, 2, 2, 1, 2 }, // 9
};

static int TestDigit(int n, double u, double v, double thickness, double margin)
{
	if (n < 0 || 9 < n)
		return 0;

	const int s = GetSegment(u, v, thickness, margin);

	if (s < 0 || 7 < s)
		return 0;

	return segLU[n][s];
}

static int TestNumber(int digits, int number, double thickness, double margin, double u, double v)
{
	double digitIndexDbl = 0.0;
	const double digitU = modf(u * (double)digits, &digitIndexDbl);
	const double digitV = v;
	const int digitIndex = (int)digitIndexDbl;

	if (0 <= digitIndex && digitIndex < digits)
	{
		int nU = number;
		int digit = 0;
		for (int i = 0; i < digits - digitIndex; i++)
		{
			digit = nU % 10;
			nU -= digit;
			nU /= 10;
		}
		return TestDigit(digit, digitU, digitV, thickness, margin);
	}

	return 0;
}

CRhRdkColor CRhRdkGridTextureEvaluator::GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const
{
	const ON_3dPoint uvwNew = LocalMappingTransform() * uvw;
	const ON_3dVector duvwdxNew = LocalMappingTransform() * duvwdx;
	const ON_3dVector duvwdyNew = LocalMappingTransform() * duvwdy;

	double foo = 0.0;
	double u = modf(uvwNew.x, &foo);
	double v = modf(uvwNew.y, &foo);

	if (u < 0.0) u += 1.0;
	if (v < 0.0) v += 1.0;

	double gridU = 0.0;
	double gridV = 0.0;
	double cellU = 0.0;
	double cellV = 0.0;

	cellU = modf((double)m_iCells * u, &gridU);
	cellV = modf((double)m_iCells * v, &gridV);

	const int cellParity = ((int)gridU + (int)gridV) % 2;

	static const CRhRdkColor white(1.0f, 1.0f, 1.0f);
	static const CRhRdkColor black(0.0f, 0.0f, 0.0f);
	const CRhRdkColor& colBackground = (cellParity == 0) ? Color1() : Color2();
	const CRhRdkColor& colNumber = colBackground.Luminance() < 0.5f ? white : black;

	const int cellNumber = (int)gridU + m_iCells * (m_iCells - 1 - (int)gridV);

	const int maxCellNumber = m_iCells * m_iCells - 1;

	const int digitCount = maxCellNumber < 10 ? 1 : maxCellNumber < 100 ? 2 : maxCellNumber < 1000 ? 3 : maxCellNumber < 1000 ? 4 : 5;
	const double numberStartU = (double)(5 - digitCount) * 0.1;
	const double numberEndU = 1.0 - numberStartU;
	const double numberStartV = (double)0.3;
	const double numberEndV = (double)0.7;
	const double numberU = (cellU - numberStartU) / (numberEndU - numberStartU);
	const double numberV = (cellV - numberStartV) / (numberEndV - numberStartV);

	const int hitResult = TestNumber(digitCount, cellNumber, 0.12 * m_dFontThickness, 0.1, numberU, 1.0 - numberV);

	if (2 == hitResult)
		return colNumber;

	return colBackground;
}

// CRhRdkGridTexture

CRhRdkGridTexture::CRhRdkGridTexture()
	:
	m_iCells		(*this, FS_TEX_GRID_CELLS         , RhLocalizeString( L"Cells", 33723)         , L"Cells"         ),
	m_dFontThickness(*this, FS_TEX_GRID_FONT_THICKNESS, RhLocalizeString( L"Font Thickness", 33724), L"Font Thickness")
{
	m_iCells = 10;
	m_dFontThickness = 0.5;

	SetColor1(CRhRdkColor(100, 100, 100, 255));
	SetColor2(CRhRdkColor(200, 200, 200, 255));

	m_iCells.SetLimits(0, 316);
	m_dFontThickness.SetLimits(0.0, 1.0);
}

int CRhRdkGridTexture::Cells(void) const
{
	return m_iCells;
}

void CRhRdkGridTexture::SetCells(int i)
{
	m_iCells.SetValue(i);
}

double CRhRdkGridTexture::FontThickness(void) const
{
	return m_dFontThickness;
}

void CRhRdkGridTexture::SetFontThickness(double d)
{
	m_dFontThickness.SetValue(d);
}

UUID CRhRdkGridTexture::TypeId(void) const
{
	return uuidGridTextureType;
}

ON_wString CRhRdkGridTexture::InternalName(void) const
{
	return L"rdk-grid-texture";
}

ON_wString CRhRdkGridTexture::TypeName(void) const
{
	return RhLocalizeString( L"Grid Texture", 32481);
}

ON_wString CRhRdkGridTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Grid texture", 32482);
}

const wchar_t* CRhRdkGridTexture::Category(void) const
{
	return RDK_CAT_PROCEDURAL_2D;
}

IRhRdkTextureEvaluator* CRhRdkGridTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new CRhRdkGridTextureEvaluator(*this, ef);
}














class CRhRdkAddTextureEvaluator : public CRhRdkTwoColorEvaluator
{
public:
	CRhRdkAddTextureEvaluator(const CRhRdkAddTexture& texture, CEvalFlags ef)
		: CRhRdkTwoColorEvaluator(texture, ef, true) 
	{ }

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const override
	{
		return OutputColor(1, uvw, duvwdx, duvwdy, pvData) + OutputColor(2, uvw, duvwdx, duvwdy, pvData);
	}
};

CRhRdkAddTexture::CRhRdkAddTexture()
{
}

UUID CRhRdkAddTexture::TypeId(void) const
{
	return uuidAddTextureType;
}

ON_wString CRhRdkAddTexture::InternalName(void) const			{	return L"rdk-add-texture";}
ON_wString CRhRdkAddTexture::TypeName(void) const				{	return RhLocalizeString( L"Add Texture", 41957);}
ON_wString CRhRdkAddTexture::TypeDescription(void) const		{	return RhLocalizeString( L"Adds two textures together", 41958);}
const wchar_t* CRhRdkAddTexture::Category(void) const			{	return RDK_CAT_PROCEDURAL_3D;}
IRhRdkTextureEvaluator* CRhRdkAddTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const	{	return new CRhRdkAddTextureEvaluator(*this, ef);}







class CRhRdkMultiplyTextureEvaluator : public CRhRdkTwoColorEvaluator
{
public:
	CRhRdkMultiplyTextureEvaluator(const CRhRdkMultiplyTexture& texture, CEvalFlags ef)
		: CRhRdkTwoColorEvaluator(texture, ef, true)
	{ }

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const override
	{
		return OutputColor(1, uvw, duvwdx, duvwdy, pvData) * OutputColor(2, uvw, duvwdx, duvwdy, pvData);
	}
};

CRhRdkMultiplyTexture::CRhRdkMultiplyTexture()
{
}

UUID CRhRdkMultiplyTexture::TypeId(void) const
{
	return uuidMultiplyTextureType;
}

ON_wString CRhRdkMultiplyTexture::InternalName(void) const { return L"rdk-multiply-texture"; }
ON_wString CRhRdkMultiplyTexture::TypeName(void) const { return RhLocalizeString( L"Multiply Texture", 41959); }
ON_wString CRhRdkMultiplyTexture::TypeDescription(void) const { return RhLocalizeString( L"Multiplies two textures together", 41960); }
const wchar_t* CRhRdkMultiplyTexture::Category(void) const { return RDK_CAT_PROCEDURAL_3D; }
IRhRdkTextureEvaluator* CRhRdkMultiplyTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const { return new CRhRdkMultiplyTextureEvaluator(*this, ef); }
