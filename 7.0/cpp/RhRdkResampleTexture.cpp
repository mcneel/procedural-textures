
#include "stdafx.h"
#include "RhRdkResampleTexture.h"
#include "RhRcm.h"
#include "RhRdkTextureFactories.h"

using namespace std;

CRhRdkResampleTexture::CRhRdkResampleTexture()
	:
	m_iDivisionsU (*this, FS_TEX_RESAMPLE_U_DIVISIONS, RhLocalizeString( L"U Divisions", 33755), L"U Divisions"),
	m_iDivisionsV (*this, FS_TEX_RESAMPLE_V_DIVISIONS, RhLocalizeString( L"V Divisions", 33756), L"V Divisions"),
	m_iMinSamples (*this, FS_TEX_RESAMPLE_MIN_SAMPLES, RhLocalizeString( L"Min Samples", 33757), L"Min Samples"),
	m_iMaxSamples (*this, FS_TEX_RESAMPLE_MAX_SAMPLES, RhLocalizeString( L"Max Samples", 33758), L"Max Samples"),
	m_bInterpolate(*this, FS_TEX_RESAMPLE_INTERPOLATE, RhLocalizeString( L"Interpolate", 33759), L"Interpolate"),
	m_bBlurOn     (*this, FS_TEX_RESAMPLE_BLUR_ON    , RhLocalizeString( L"Blur On",     33760), L"Blur On"    ),
	m_dBlurRadius (*this, FS_TEX_RESAMPLE_BLUR_RADIUS, RhLocalizeString( L"Blur Radius", 33761), L"Blur Radius"),
	m_bBlurUTiled (*this, FS_TEX_RESAMPLE_BLUR_WRAP_U, RhLocalizeString( L"Blur Wrap U", 33762), L"Blur Wrap U"),
	m_bBlurVTiled (*this, FS_TEX_RESAMPLE_BLUR_WRAP_V, RhLocalizeString( L"Blur Wrap V", 33763), L"Blur Wrap V"),
	m_sBlurType   (*this, FS_TEX_RESAMPLE_BLUR_TYPE  , RhLocalizeString( L"Blur Type",   33764), L"Blur Type"  )
{
	m_iDivisionsU = 16;
	m_iDivisionsV = 16;
	m_iMinSamples = 1;
	m_iMaxSamples = 16;
	m_bInterpolate = true;
	m_bBlurOn = false;
	m_dBlurRadius = 0.01;
	m_bBlurUTiled = false;
	m_bBlurVTiled = false;
	m_sBlurType = FS_TEX_RESAMPLE_BLUR_TYPE_BOX;

	m_iDivisionsU.SetLimits(4, 4096);
	m_iDivisionsV.SetLimits(4, 4096);
	m_iMinSamples.SetLimits(0, CRhRdkVariant::Null());
	m_iMaxSamples.SetLimits(0, CRhRdkVariant::Null());
	m_dBlurRadius.SetLimits(0, CRhRdkVariant::Null());
}

CRhRdkResampleTexture::~CRhRdkResampleTexture()
{
}

int CRhRdkResampleTexture::DivisionsU(void) const
{
	return m_iDivisionsU;
}

void CRhRdkResampleTexture::SetDivisionsU(int i)
{
	if (m_iDivisionsU.Value().AsInteger() != i)
	{
		DimensionsChanged(); // Must be before setting the field because event sinks may rely on it.

		m_iDivisionsU.SetValue(i);
	}
}

int CRhRdkResampleTexture::DivisionsV(void) const
{
	return m_iDivisionsV;
}

void CRhRdkResampleTexture::SetDivisionsV(int i)
{
	if (m_iDivisionsV.Value().AsInteger() != i)
	{
		DimensionsChanged(); // Must be before setting the field because event sinks may rely on it.

		m_iDivisionsV.SetValue(i);
	}
}

int CRhRdkResampleTexture::MinSamples(void) const
{
	return m_iMinSamples;
}

void CRhRdkResampleTexture::SetMinSamples(int i)
{
	m_iMinSamples.SetValue(i);
}

int CRhRdkResampleTexture::MaxSamples(void) const
{
	return m_iMaxSamples;
}

void CRhRdkResampleTexture::SetMaxSamples(int i)
{
	m_iMaxSamples.SetValue(i);
}

bool CRhRdkResampleTexture::Interpolate(void) const
{
	return m_bInterpolate;
}

void CRhRdkResampleTexture::SetInterpolate(bool b)
{
	m_bInterpolate.SetValue(b);
}

bool CRhRdkResampleTexture::BlurOn(void) const
{
	return m_bBlurOn;
}

void CRhRdkResampleTexture::SetBlurOn(bool b)
{
	m_bBlurOn.SetValue(b);
}

double CRhRdkResampleTexture::BlurRadius(void) const
{
	return m_dBlurRadius;
}

void CRhRdkResampleTexture::SetBlurRadius(double d)
{
	m_dBlurRadius.SetValue(d);
}

bool CRhRdkResampleTexture::BlurTiledU(void) const
{
	return m_bBlurUTiled;
}

void CRhRdkResampleTexture::SetBlurTiledU(bool b)
{
	m_bBlurUTiled.SetValue(b);
}

bool CRhRdkResampleTexture::BlurTiledV(void) const
{
	return m_bBlurVTiled;
}

void CRhRdkResampleTexture::SetBlurTiledV(bool b)
{
	m_bBlurVTiled.SetValue(b);
}

CRhRdkBlurType CRhRdkResampleTexture::BlurType(void) const
{
	const CLBPString s = m_sBlurType.Value().AsString();

	if (FS_TEX_RESAMPLE_BLUR_TYPE_TRIANGLE == s) return triangle;
	if (FS_TEX_RESAMPLE_BLUR_TYPE_GAUSSIAN == s) return gaussian;
	if (FS_TEX_RESAMPLE_BLUR_TYPE_MITNET == s)   return mitnet;

	return box;
}

void CRhRdkResampleTexture::SetBlurType(CRhRdkBlurType type)
{
	const wchar_t* wsz = FS_TEX_RESAMPLE_BLUR_TYPE_BOX;
	switch (type)
	{
	case triangle: wsz = FS_TEX_RESAMPLE_BLUR_TYPE_TRIANGLE; break;
	case gaussian: wsz = FS_TEX_RESAMPLE_BLUR_TYPE_GAUSSIAN; break;
	case mitnet:   wsz = FS_TEX_RESAMPLE_BLUR_TYPE_MITNET;   break;
	}
	m_sBlurType.SetValue(wsz);
}

UUID CRhRdkResampleTexture::RenderEngineId(void) const
{
	return uuidUniversalRenderEngine;
}

UUID CRhRdkResampleTexture::PlugInId(void) const
{
	return rdk.RhinoPlugInUuid();
}

UUID CRhRdkResampleTexture::TypeId(void) const
{
	return uuidResampleTextureType;
}

ON_wString CRhRdkResampleTexture::InternalName(void) const
{
	return L"ResampleTexture";
}

ON_wString CRhRdkResampleTexture::TypeName(void) const
{
	return RhLocalizeString( L"Resample Texture", 32485);
}

ON_wString CRhRdkResampleTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Resample Texture", 32486);
}

const wchar_t* CRhRdkResampleTexture::Category(void) const
{
	return RDK_CAT_PROCEDURAL_2D;
}

bool CRhRdkResampleTexture::WriteParametersToSection(IRhRdk_XMLSection& section, WriteParamsContext context) const
{
	return __super::WriteParametersToSection(section, context);
}

bool CRhRdkResampleTexture::ReadParametersFromSection(const IRhRdk_XMLSection& section, ReadParamsContext context)
{
	return __super::ReadParametersFromSection(section, context);
}

bool CRhRdkResampleTexture::GetImageSize(CSize& size) const
{
	size.cx = int(m_iDivisionsU);
	size.cy = int(m_iDivisionsV);

	return true;
}

bool CRhRdkResampleTexture::PixelSize(int& u, int& v, int& w) const
{
	return CRhRdkPixelTextureMixin::PixelSize(u,v,w);
}

CRhRdkVariant CRhRdkResampleTexture::GetParameter(const wchar_t* wszName) const
{
	CRhRdkVariant v;
	if (CRhRdkPixelTextureMixin::GetParameter(wszName, v))
		return v;

	return __super::GetParameter(wszName);
}

bool CRhRdkResampleTexture::GetExtraRequirementParameter(const wchar_t* wszParamName, const wchar_t* wszExtraReqName, CRhRdkVariant& vValueOut) const
{
	if (CLBPString(wszExtraReqName) == RDK_DECIMAL_PLACES_AMOUNT)
	{
		if (CLBPString(wszParamName) == FS_TEX_RESAMPLE_BLUR_RADIUS)
		{
			vValueOut = 2;
			return true;
		}
	}

	return __super::GetExtraRequirementParameter(wszParamName, wszExtraReqName, vValueOut);
}

bool CRhRdkResampleTexture::IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const
{
	return f.IsKind(CRhRdkContent::Kinds::Texture);
}

unsigned int CRhRdkResampleTexture::ComputeRenderCRC(const CRenderCRCFlags& rcrcFlags) const
{
	return __super::ComputeRenderCRC(rcrcFlags) ^ GetCompileDateMash();
}

bool CRhRdkResampleTexture::IsHDRCapable(void) const
{
	const CRhRdkTexture* pTexture = dynamic_cast<const CRhRdkTexture*>(FindChild(CS_TEX_RESAMPLE_TEXTURE));
	if (nullptr != pTexture)
	{
		return pTexture->IsHDRCapable();
	}

	return false;
}

class CRhRdkResampleTextureCSI : public CRhRdkContent::CChildSlotIterator
{
public:
	CRhRdkResampleTextureCSI(const CRhRdkContent* pContent, CRhRdkContent::CSIContext context)
		: CRhRdkContent::CChildSlotIterator(pContent, context) { m_iIndex = -1; }

	virtual bool NextChildSlot(ON_wString& sParamNameOut, ON_wString& sChildSlotNameOut, ON_wString& sDisplayNameOut);

protected:
	int m_iIndex;
};

CRhRdkContent::CChildSlotIterator* CRhRdkResampleTexture::NewChildSlotIterator(CSIContext context) const
{
	return new CRhRdkResampleTextureCSI(this, context);
}

bool CRhRdkResampleTextureCSI::NextChildSlot(ON_wString& sParamNameOut, ON_wString& sChildSlotNameOut, ON_wString& sDisplayNameOut)
{
	bool bSuccess = false;

	m_iIndex++;

	if (0 == m_iIndex)
	{
		sChildSlotNameOut = CS_TEX_RESAMPLE_TEXTURE;
		bSuccess = true;
	}

	if (bSuccess)
	{
		sDisplayNameOut = RhLocalizeString( L"InputTexture", 23438);
		sParamNameOut = m_pContent->ParamNameFromChildSlotName(sChildSlotNameOut.Array());
	}

	return bSuccess;
}

class CResampleTextureEvaluator : public CRhRdkTextureEvaluator
{
public:
	CResampleTextureEvaluator(const CRhRdkResampleTexture& texture, CEvalFlags ef)
		: 
		m_mitNetB(1.0 / 3.0),
		m_mitNetC(1.0 / 3.0),
		m_pDownsampleCache(nullptr),
		m_pBlurCache(nullptr),
		m_refCount(1),
		m_crc(texture.RenderCRC(CRhRdkContent::CRenderCRCFlags::rcrc_ExcludeLinearWorkflow)),
		CRhRdkTextureEvaluator(ef)
	{
		m_pSourceTE = nullptr;

		const CRhRdkTexture* pTexture = dynamic_cast<const CRhRdkTexture*>(texture.FindChild(CS_TEX_RESAMPLE_TEXTURE));
		if (pTexture != nullptr)
		{
			m_pSourceTE = pTexture->NewTextureEvaluator(ef);
			if (nullptr == m_pSourceTE)
			{
				m_pSourceTE = pTexture->NewSimulatedTextureEvaluator();
			}
		}

		m_iDivisionsU  = texture.DivisionsU();
		m_iDivisionsV  = texture.DivisionsV();
		m_iMinSamples  = texture.MinSamples();
		m_iMaxSamples  = texture.MaxSamples();
		m_bInterpolate = texture.Interpolate();
		m_bBlurOn      = texture.BlurOn();
		m_dBlurRadius  = texture.BlurRadius();
		m_blurType     = texture.BlurType();
		m_bBlurUTiled  = texture.BlurTiledU();
		m_bBlurVTiled  = texture.BlurTiledV();
		m_xform        = texture.LocalMappingTransform();

		const int cacheSize = m_iDivisionsU * m_iDivisionsV;
		m_pDownsampleCache = new CCachedColor[cacheSize];
		m_pBlurCache       = new CCachedColor[cacheSize];

		m_inputChildIndex = -1;

		m_csCache.Lock();
		m_cache.Append(this);
		m_csCache.Unlock();
	}

	virtual ~CResampleTextureEvaluator()
	{
		m_csCache.Lock();
		for (int i=0;i<m_cache.Count();i++)
		{
		  if (m_cache[i] == this)
		  {
			m_cache.Remove(i);
		  }
		}
		m_csCache.Unlock();

		delete[] m_pDownsampleCache;
		delete[] m_pBlurCache;

		SAFE_DELETE_THIS(m_pSourceTE);
	}

	unsigned long long AddRef(void)
	{
		return ::InterlockedIncrement(&m_refCount);
	}

	ON__INT32 RenderCRC(void) const
	{
		return m_crc + EvaluatorFlags().Flags();
	}

	virtual void DeleteThis(void)
	{
		/*unsigned long long ulRefCount = */::InterlockedDecrement(&m_refCount);

		//Implement lazy delete on end command.
		//if (0== ulRefCount)
		//{
		//  delete this; 
		//}
	}

	static void CleanUp(void)
	{
		m_csCache.Lock();
		for (int i = 0; i < m_cache.Count(); i++)
		{
			if (m_cache[i] && 0 == m_cache[i]->m_refCount)
			{
				delete m_cache[i--];
			}
		}
		m_csCache.Unlock();
	}

	virtual bool GetColor(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData = nullptr) const override;
	virtual void* EVF(const wchar_t*, void*) override { return nullptr; }

	bool GetColor2(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void * pvData) const;

	bool GetSourceColor(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData) const;

	virtual bool RegisterChildCallback(IChildCallback* pChildCallback, const CRhRdkTexture& texture, void* pvData);

protected:
	class CCachedColor
	{
	public:
		CCachedColor() : m_bCached(false) { }
		CRhRdkColor m_value;
		bool m_bCached;
	};

	bool GetDownsampledColor(int uDiv, int vDiv, CRhRdkColor& colOut, void * pvData) const;

	bool PrecacheDownsampledImage(void * pvData);
	bool PrecacheBlurredImage();

	double MitNet1D(double x) const;

private:
	ON_Xform m_xform;
	int m_iDivisionsU;
	int m_iDivisionsV;
	int m_iMinSamples;
	int m_iMaxSamples;
	bool m_bInterpolate;
	bool m_bBlurOn;
	double m_dBlurRadius;
	CRhRdkBlurType m_blurType;
	bool m_bBlurUTiled;
	bool m_bBlurVTiled;
	double m_mitNetB;
	double m_mitNetC;
	IRhRdkTextureEvaluator* m_pSourceTE;
	CCachedColor* m_pDownsampleCache;
	CCachedColor* m_pBlurCache;
	int m_inputChildIndex;

private:
	volatile REFCOUNT_INT m_refCount;
	const ON__INT32 m_crc;
	static ON_SimpleArray<CResampleTextureEvaluator*> m_cache;
	static CCriticalSection m_csCache;
	friend CRhRdkResampleTexture;
	friend CRhRdkResampleTextureFactory;
};

ON_SimpleArray<CResampleTextureEvaluator*> CResampleTextureEvaluator::m_cache;
CCriticalSection CResampleTextureEvaluator::m_csCache;


CRhRdkResampleTextureFactory::~CRhRdkResampleTextureFactory()
{
	CResampleTextureEvaluator::CleanUp();
	RHRDK_ASSERT(CResampleTextureEvaluator::m_cache.Count() == 0);
}

bool CResampleTextureEvaluator::GetSourceColor(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData) const
{
	const IChildCallback* pChildCallback = ChildCallback();
	if (pChildCallback && -1 != m_inputChildIndex)
	{
		if (pChildCallback->GetChildColor(m_inputChildIndex, pvData, uvw, duvwdx, duvwdy, colOut))
			return true;
	}

	if (nullptr != m_pSourceTE)
	{
		if (m_pSourceTE->GetColor(uvw, duvwdx, duvwdy, colOut))
			return true;
	}

	colOut.Set(1.0, 0.0, 0.0);

	return false;
}

bool CResampleTextureEvaluator::PrecacheDownsampledImage(void * pvData)
{
	bool bFailed = false;

	if (nullptr != m_pDownsampleCache)
	{
		const double uCellSize = 1.0 / (double)m_iDivisionsU;
		const double vCellSize = 1.0 / (double)m_iDivisionsV;

		ON_2dPoint ptSamplePoint(0.0, 0.0);
		int pointIndex = 0;

		CLBPRh2dBestCandidateSamplePattern * pSp = rdk.BestCandidateSamplePatternCreator2d().NewPattern(m_iMaxSamples * 4);

		ON_2dPointArray samplePointArray(m_iMaxSamples * 4);
		while (pSp->GetPoint(pointIndex, ptSamplePoint))
		{
			samplePointArray.Append(ptSamplePoint);
			pointIndex ++;
		}

		CRhRdkColor color(0.0f, 0.0f, 0.0f);
		ON_3dPoint ptSource(0.0, 0.0, 0.0);
		const ON_3dPoint ptOrigin(0.0, 0.0, 0.0);

		for (int v = 0; v < m_iDivisionsV; v++)
		{
			const double cellStartV = (double)v * vCellSize;

			for (int u = 0; u < m_iDivisionsU; u++)
			{
				const double cellStartU = (double)u * uCellSize;

				pointIndex = 0;
				CRhRdkColor totalColor(0.0f, 0.0f, 0.0f);
				double totalColorWeight = 0;
				int sampleSetsTaken = 0;

				while (!bFailed)
				{
					// Take sample set
					CRhRdkColor sampleSetColor(0.0f, 0.0f, 0.0f);
					double sampleSetColorWeight = 0;
					for (int i = 0; i < 4; i++)
					{
						ptSource.x = cellStartU + samplePointArray[pointIndex].x * uCellSize;
						ptSource.y = cellStartV + samplePointArray[pointIndex].y * vCellSize;

						if (GetSourceColor(ptSource, ptOrigin, ptOrigin, color, pvData))
						{
							sampleSetColor += color;
							sampleSetColorWeight += 1.0;
						}

						pointIndex ++;
					}

					if (sampleSetsTaken < m_iMinSamples)
					{
						totalColor += sampleSetColor;
						totalColorWeight += sampleSetColorWeight;
					}
					else
					{
						CRhRdkColor oldColor = totalColor;
						if (totalColorWeight > 0.0)
							oldColor /= (float)totalColorWeight;

						CRhRdkColor newColor = totalColor;
						newColor += sampleSetColor;
						if (totalColorWeight + sampleSetColorWeight > 0.0)
							newColor /= (float)(totalColorWeight + sampleSetColorWeight);

						totalColor += sampleSetColor;
						totalColorWeight += sampleSetColorWeight;	

						const double threshold = 0.01;
						float deltaAlpha = std::abs(newColor.FAlpha() - oldColor.FAlpha());
						if (deltaAlpha < threshold)
						{
							float deltaGreen = std::abs(newColor.FGreen() - oldColor.FGreen());
							if (deltaGreen < threshold)
							{
								float deltaBlue = std::abs(newColor.FBlue() - oldColor.FBlue());
								if (deltaBlue < threshold)
								{
									float deltaRed = std::abs(newColor.FRed() - oldColor.FRed());
									if (deltaRed < threshold)
										break;
								}
							}
						}
					}
					
					sampleSetsTaken++;

					if (sampleSetsTaken >= m_iMaxSamples)
						break;
				}

				if (totalColorWeight > 0)
					totalColor /= (float)totalColorWeight;

				m_pDownsampleCache[u + v * m_iDivisionsU].m_bCached = true;
				m_pDownsampleCache[u + v * m_iDivisionsU].m_value = totalColor;
			}
		}
		delete pSp;
	}
	else
	{
		bFailed = true;
	}

	return !bFailed;
}

bool CResampleTextureEvaluator::PrecacheBlurredImage()
{
	if ((nullptr == m_pDownsampleCache) || (nullptr == m_pBlurCache))
		return false;

	int uCell = 0;
	int vCell = 0;

	if (m_bBlurOn)
	{
		const int blurRadiusU = (int)(ceil(m_dBlurRadius * (double)m_iDivisionsU));
		const int blurRadiusV = (int)(ceil(m_dBlurRadius * (double)m_iDivisionsV));

		const double blurRadiusUInv = 1.0 / (double)(blurRadiusU + 1);
		const double blurRadiusVInv = 1.0 / (double)(blurRadiusV + 1);

		const double alpha = 0.5;
		const double expUV = (m_blurType == gaussian) ? exp(-alpha * 1.0) : 1.0;

		double* uFactorLookup = new double [2 * blurRadiusU + 1];
		for (int u = 0; u <= blurRadiusU; u++)
		{
			if (m_blurType == box)
				uFactorLookup[blurRadiusU + u] = 1.0;
			else
			{
				const double uDist = double(abs(u)) * blurRadiusUInv;
				if (m_blurType == triangle)
					uFactorLookup[blurRadiusU + u] = 1.0 - uDist;
				else
				if (m_blurType == gaussian)
					uFactorLookup[blurRadiusU + u] = max(0.0, exp(-alpha * uDist * uDist) - expUV);
				else
				if (m_blurType == mitnet)
					uFactorLookup[blurRadiusU + u] = MitNet1D(uDist);
			}

			if (u != 0)
				uFactorLookup[blurRadiusU - u] = uFactorLookup[blurRadiusU + u];
		}

		double* vFactorLookup = new double [2 * blurRadiusV + 1];
		for (int v = 0; v <= blurRadiusV; v++)
		{
			if (m_blurType == box)
				vFactorLookup[blurRadiusV + v] = 1.0;
			else
			{
				const double vDist = double(abs(v)) * blurRadiusVInv;
				if (m_blurType == triangle)
					vFactorLookup[blurRadiusV + v] = 1.0 - vDist;
				else
				if (m_blurType == gaussian)
					vFactorLookup[blurRadiusV + v] = max(0.0, exp(-alpha * vDist * vDist) - expUV);
				else
				if (m_blurType == mitnet)
					vFactorLookup[blurRadiusV + v] = MitNet1D(vDist);
			}
			if (v != 0)
				vFactorLookup[blurRadiusV - v] = uFactorLookup[blurRadiusV + v];
		}

		for (vCell = 0; vCell < m_iDivisionsV; vCell++)
		{
			int vStart = -blurRadiusV;
			int vEnd = blurRadiusV;
			if (!m_bBlurVTiled)
			{
				if (vCell + vStart < 0)
					vStart = -vCell;
				if (vCell + vEnd >= m_iDivisionsV)
					vEnd = m_iDivisionsV - 1 - vCell;
			}

			for (uCell = 0; uCell < m_iDivisionsU; uCell++)
			{
				int uStart = -blurRadiusU;
				int uEnd = blurRadiusU;
				if (!m_bBlurUTiled)
				{
					if (uCell + uStart < 0)
						uStart = -uCell;
					if (uCell + uEnd >= m_iDivisionsU)
						uEnd = m_iDivisionsU - 1 - uCell;
				}


				CRhRdkColor totalColor(0.0f, 0.0f, 0.0f);
				double totalColorWeight = 0;

				for (int v = vStart; v <= vEnd; v++)
				{
					for (int u = uStart; u <= uEnd; u++)
					{
						CRhRdkColor cellColor(m_pDownsampleCache[(uCell + u) + m_iDivisionsU * (vCell + v)].m_value);
						const double colorWeight = uFactorLookup[blurRadiusU + u] * vFactorLookup[blurRadiusV + v];
						cellColor *= (float)colorWeight;
						totalColor += cellColor;
						totalColorWeight += colorWeight;
					}
				}

				if (totalColorWeight > 0.0)
					totalColor /= (float)totalColorWeight;

				m_pBlurCache[uCell + vCell * m_iDivisionsU].m_value = totalColor;
				m_pBlurCache[uCell + vCell * m_iDivisionsU].m_bCached = true;
			}
		}

		delete [] uFactorLookup;
		delete [] vFactorLookup;
	}
	else
	{
		memcpy(m_pBlurCache, m_pDownsampleCache, sizeof(CCachedColor) * m_iDivisionsV * m_iDivisionsU);
	}

	return true;
}

double CResampleTextureEvaluator::MitNet1D(double x) const
{
	const double mx = 2.0 * std::abs(x);
	const double b = m_mitNetB;
	const double c = m_mitNetC;
	if (mx > 1.0)
	{
		return ((-b - 6.0 * c) * mx * mx * mx + 
			   (6.0 * b + 30.0 * c) * mx * mx +
			   (-12.0 * b - 48.0 * c) * mx +
			   (8.0 * b + 24.0 * c)) * (1.0 / 6.0);
	}
	else
	{
		return ((12.0 - 9.0 * b - 6.0 * c) * mx * mx * mx +
				(-18.0 + 12.0 * b + 6.0 * c) * mx * mx +
				(6.0 - 2.0 * b)) * (1.0 / 6.0);
	}
}

bool CResampleTextureEvaluator::GetColor(const ON_3dPoint& uvwOriginal, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData) const
{
	// If interpolation is switched off then use the normal GetColor2
	if (!m_bInterpolate)
		return GetColor2(uvwOriginal, duvwdx, duvwdy, colOut, pvData);

	const int uDiv = m_iDivisionsU;
	const int vDiv = m_iDivisionsV;

	const double dUInt = ((int)(0.5 + (uvwOriginal.x * uDiv))) / (double)uDiv;
	const double dVInt = ((int)(0.5 + (uvwOriginal.y * vDiv))) / (double)vDiv;

	ON_2dPoint uvIntersection(dUInt, dVInt);

	const double dHalfU = (1.0 / uDiv) * 0.5;
	const double dHalfV = (1.0 / vDiv) * 0.5;

	ON_2dPoint uv[4];
	uv[0] = uvIntersection + ON_2dVector( dHalfU,  dHalfV);
	uv[1] = uvIntersection + ON_2dVector( dHalfU, -dHalfV);
	uv[2] = uvIntersection + ON_2dVector(-dHalfU,  dHalfV);
	uv[3] = uvIntersection + ON_2dVector(-dHalfU, -dHalfV);

	colOut.Set(0,0,0);

	const ON_2dPoint vUV(uvwOriginal.x, uvwOriginal.y);

	// float dDivisor = 0.0;

	// Find colors for vertices "current cell"
	CRhRdkColor col[4];
	for (int i = 0; i < 4; i++)
		GetColor2(uv[i], duvwdx, duvwdy, col[i], pvData);

	// Find relative coordinates of sample point inside the "current cell"
	const float fXPrime = (float)((vUV.x - uv[2].x) / (uv[0].x - uv[2].x));
	const float fYPrime = (float)((vUV.y - uv[1].y) / (uv[0].y - uv[1].y));
	const float fX = (3.0f - 2.0f * fXPrime) * fXPrime * fXPrime;
	const float fY = (3.0f - 2.0f * fYPrime) * fYPrime * fYPrime;

	// Use 3 linear interpolations to get final color
	colOut = fY * (1.0f - fX) * col[2] + fY * fX * col[0] + (1.0f - fY) * (1.0f - fX) * col[3] + (1.0f - fY) * fX * col[1];

	return true;
}

bool CResampleTextureEvaluator::GetColor2(const ON_3dPoint& uvwOriginal, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void * pvData) const
{
	colOut.Set(0.0, 0.0, 0.0, 0.0);

	const int uDivision = m_iDivisionsU;
	const int vDivision = m_iDivisionsV;

	if ((0 == uDivision) || (0 == vDivision))
		return true;

	bool bFailed = false;

	const ON_3dPoint uvw = m_xform * uvwOriginal;

	const int cuCellIndex = (int)floor(uvw.x * (double)uDivision) % uDivision;
	const int cvCellIndex = (int)floor(uvw.y * (double)vDivision) % vDivision;
	const int cuCell = cuCellIndex < 0 ? cuCellIndex + uDivision : cuCellIndex;
	const int cvCell = cvCellIndex < 0 ? cvCellIndex + vDivision : cvCellIndex;

	if (m_pBlurCache && m_pBlurCache[cuCell + cvCell * uDivision].m_bCached)
	{
		colOut = m_pBlurCache[cuCell + cvCell * uDivision].m_value;
		return true;
	}

	CRhRdkColor totalColor(0.0f, 0.0f, 0.0f);
	if (m_bBlurOn)
	{
		const int blurRadiusU = (m_bBlurOn ? (int)(ceil(m_dBlurRadius * (double)uDivision)) : 0);
		const int blurRadiusV = (m_bBlurOn ? (int)(ceil(m_dBlurRadius * (double)vDivision)) : 0);

		int uStart = -blurRadiusU;
		if (!m_bBlurUTiled && cuCell + uStart < 0)
			uStart = -cuCell;
		int uEnd = blurRadiusU;
		if (!m_bBlurUTiled && cuCell + uEnd >= uDivision)
			uEnd = uDivision - 1 - cuCell;
		int vStart = -blurRadiusV;
		if (!m_bBlurVTiled && cvCell + vStart < 0)
			vStart = -cvCell;
		int vEnd = blurRadiusV;
		if (!m_bBlurVTiled && cvCell + vEnd >= vDivision)
			vEnd = vDivision - 1 - cvCell;

		double totalColorWeight = 0;
		int totalColorsAdded = 0;

		const double alpha = 0.5;
		const double expUV = (m_blurType == gaussian) ? exp(-alpha * 1.0) : 1.0;

		const double blurRadiusUInv = 1.0 / (double)(blurRadiusU + 1);
		const double blurRadiusVInv = 1.0 / (double)(blurRadiusV + 1);

		int vCellIndex = (cvCell + vStart) % vDivision;
		vCellIndex = vCellIndex < 0 ? vCellIndex + vDivision : vCellIndex;

		CRhRdkColor cellColor( 0.0f, 0.0f, 0.0f );

		for( int v = vStart; v <= vEnd; v++, vCellIndex++ )
		{
			if( vCellIndex >= vDivision ) {
				vCellIndex -= vDivision;
			}

			const double vDist = double(abs(v)) * blurRadiusVInv;

			double factorV = 1.0;
			if (m_blurType == triangle)
				factorV = 1.0 - vDist;
			else
			if (m_blurType == gaussian)
				factorV = max(0.0, exp(-alpha * vDist * vDist) - expUV);
			else
			if (m_blurType == mitnet)
				factorV = MitNet1D(vDist);

			int uCellIndex = (cuCell + uStart) % uDivision;
			uCellIndex = uCellIndex < 0 ? uCellIndex + uDivision : uCellIndex;

			for (int u = uStart; u <= uEnd; u++, uCellIndex++)
			{
				if( uCellIndex >= uDivision ) {
					uCellIndex -= uDivision;
				}

				const double uDist = double(abs(u)) * blurRadiusUInv;

				double factorU = 1.0;
				if (m_blurType == triangle)
					factorU = 1.0 - uDist;
				else
				if (m_blurType == gaussian)
					factorU = max(0.0, exp(-alpha * uDist * uDist) - expUV);
				else
				if (m_blurType == mitnet)
					factorU = MitNet1D(uDist);

				const int iDSCacheIndex = uCellIndex + vCellIndex * m_iDivisionsU;

				bool bOk = false;

				// We check this here instead of inside 'GetDownsampledColor' since there is a large
				// overhead calling the function
				if( (NULL != m_pDownsampleCache) && m_pDownsampleCache[iDSCacheIndex].m_bCached )
				{
					cellColor = m_pDownsampleCache[iDSCacheIndex].m_value;
					bOk = true;
				}
				else
				if (GetDownsampledColor(cuCell + u, cvCell + v, cellColor, pvData ) )
				{
					bOk = true;
				}

				if( bOk ) {
					double colorWeight = factorU * factorV;
					cellColor *= (float)colorWeight;
					totalColor += cellColor;
					totalColorsAdded++;
					totalColorWeight += colorWeight;
				}
			}
		}

		if (totalColorWeight > 0.0)
			totalColor /= (float)totalColorWeight;
	}
	else
	{
		if (!GetDownsampledColor(cuCell, cvCell, totalColor, pvData))
			bFailed = true;
	}

	if (!bFailed)
	{
		if (m_pBlurCache != 0)
		{
			m_pBlurCache[cuCell + cvCell * uDivision].m_bCached = true;
			m_pBlurCache[cuCell + cvCell * uDivision].m_value = totalColor;
		}
		colOut = totalColor;
	}
	else
	{
		colOut.Set(0.0, 0.0, 0.0);
	}

	return !bFailed;
}

bool CResampleTextureEvaluator::GetDownsampledColor(int uPixel, int vPixel, CRhRdkColor& colOut, void * pvData) const
{
	const int uDivision = m_iDivisionsU;
	const int vDivision = m_iDivisionsV;
	int uCellIndex = uPixel % uDivision;
	int vCellIndex = vPixel % vDivision;
	const int uCell = uCellIndex < 0 ? uCellIndex + uDivision : uCellIndex;
	const int vCell = vCellIndex < 0 ? vCellIndex + vDivision : vCellIndex;

	if ((nullptr != m_pDownsampleCache) && m_pDownsampleCache[uCell + vCell * uDivision].m_bCached)
	{
		colOut = m_pDownsampleCache[uCell + vCell * uDivision].m_value;
		return true;
	}

	bool bFailed = false;

	colOut.Set(0.0, 0.0, 0.0, 0.0);

	const double uCellSize = 1.0 / (double)uDivision;
	const double vCellSize = 1.0 / (double)vDivision;
	const double uCellStart = uCell * uCellSize;
	const double vCellStart = vCell * vCellSize;
	const double sampleBoxUSize = uCellSize;
	const double sampleBoxVSize = vCellSize;

	CLBPRh2dBestCandidateSamplePattern * pSp = rdk.BestCandidateSamplePatternCreator2d().NewPattern(m_iMaxSamples * 4);
	if (nullptr == pSp)
		return false;

	int pointIndex = 0;
	CRhRdkColor totalColor(0.0f, 0.0f, 0.0f);
	double totalColorWeight = 0;
	int totalColorsAdded = 0;
	ON_2dPoint ptSamplePoint;
	int sampleSetsTaken = 0;
	while (!bFailed)
	{
		// Take sample set
		CRhRdkColor sampleSetColor(0.0f, 0.0f, 0.0f);
		double sampleSetColorWeight = 0;
		int sampleSetColorsAdded = 0;
		for (int i = 0; i < 4; i++)
		{
			if (pSp->GetPoint(pointIndex, ptSamplePoint))
			{
				CRhRdkColor color(0.0f, 0.0f, 0.0f);
				const double sourceX = (0.5 * uCellSize + uCellStart) + (ptSamplePoint.x - 0.5) * sampleBoxUSize;
				const double sourceY = (0.5 * vCellSize + vCellStart) + (ptSamplePoint.y - 0.5) * sampleBoxVSize;
				if ((sourceX >= 0.0 && sourceX < 1.0) && (sourceY >= 0.0 && sourceY < 1.0))
				{
					ON_3dPoint ptSource(sourceX, sourceY, 0.0);

					if (GetSourceColor(ptSource, ON_origin, ON_origin, color, pvData))
					{
						sampleSetColor += color;
						sampleSetColorsAdded ++;
						sampleSetColorWeight += 1.0;
					}
					else
					{
						ON_wString strError(L"Error");
					}
				}
			}
			pointIndex ++;
		}

		if (sampleSetsTaken < m_iMinSamples)
		{
			totalColor += sampleSetColor;
			totalColorsAdded += sampleSetColorsAdded;
			totalColorWeight += sampleSetColorWeight;
		}
		else
		{
			CRhRdkColor oldColor = totalColor;
			if (totalColorWeight > 0.0)
				oldColor /= (float)totalColorWeight;

			CRhRdkColor newColor = totalColor;
			newColor += sampleSetColor;
			if (totalColorWeight + sampleSetColorWeight > 0.0)
				newColor /= (float)(totalColorWeight + sampleSetColorWeight);

			totalColor += sampleSetColor;
			totalColorsAdded += sampleSetColorsAdded;
			totalColorWeight += sampleSetColorWeight;	

			float deltaRed = std::abs(newColor.FRed() - oldColor.FRed());
			float deltaGreen = std::abs(newColor.FGreen() - oldColor.FGreen());
			float deltaBlue = std::abs(newColor.FBlue() - oldColor.FBlue());
			float deltaAlpha = std::abs(newColor.FAlpha() - oldColor.FAlpha());
			float deltaMax = max(max(deltaRed, deltaGreen), max(deltaBlue, deltaAlpha));
			if (deltaMax < 0.01)
				break;
		}
		
		sampleSetsTaken++;

		if (sampleSetsTaken >= m_iMaxSamples)
			break;
	}

	if (totalColorWeight > 0)
		totalColor /= (float)totalColorWeight;

	if (bFailed)
	{
		ON_wString strError(L"Error");
	}

	if (!bFailed && (nullptr != m_pDownsampleCache))
	{
		m_pDownsampleCache[uCell + vCell * uDivision].m_bCached = true;
		m_pDownsampleCache[uCell + vCell * uDivision].m_value = totalColor;
	}

	colOut = totalColor;

	delete pSp;

	return !bFailed;
}

//Implement caching mechanism

IRhRdkTextureEvaluator* CRhRdkResampleTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	//First check the cache.
	const auto f = CRhRdkContent::CRenderCRCFlags::rcrc_ExcludeLinearWorkflow;
	const ON__INT32 crc = RenderCRC(f);

	CResampleTextureEvaluator::m_csCache.Lock();
	for (int i = 0; i < CResampleTextureEvaluator::m_cache.Count(); i++)
	{
		CResampleTextureEvaluator* pE = CResampleTextureEvaluator::m_cache[i];
		if (pE->RenderCRC() == crc)
		{
			pE->AddRef();
			CResampleTextureEvaluator::m_csCache.Unlock();
			return pE;
		}
	}
	CResampleTextureEvaluator::m_csCache.Unlock();

	//This is an ideal time to clean up the cache.
	CResampleTextureEvaluator::CleanUp();

	return new CResampleTextureEvaluator(*this, ef);
}

bool CResampleTextureEvaluator::RegisterChildCallback(IRhRdkTextureEvaluator::IChildCallback* pChildCallback, const CRhRdkTexture& texture, void* pvData)
{
	if (!CRhRdkTextureEvaluator::RegisterChildCallback(pChildCallback, texture, pvData))
		return false;

	const CRhRdkContent* pChild = texture.FindChild(CS_TEX_RESAMPLE_TEXTURE);
	if (nullptr != pChild)
	{
		m_inputChildIndex = pChildCallback->InitializeChild(pChild, pvData);
	}

	return true;
}

unsigned int CRhRdkResampleTexture::BitFlags(void) const
{
	return (__super::BitFlags() & ~(bfTextureSummary | bfLocalTextureMapping | bfGraphDisplay));
}

CRhRdkContent* CRhRdkResampleTexture::MakeCopy(CopyMethods m) const
{
	CRhRdkContent* pCopy = FastMakeCopy(m);

#ifdef _DEBUG
	VerifyCopy(pCopy);
#endif

	return pCopy;
}
