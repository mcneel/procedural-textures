
#include "stdafx.h"
#include "RhRdkDotTexture.h"
#include "RhRdkTextureUtilities.h"
#include "RhRcm.h"

using namespace std;

CRhRdkDotTexture::CRhRdkDotTexture()
	:
	m_samplerType       (*this, FS_TEX_DOT_SAMPLER_TYPE                 , RhLocalizeString( L"Sampler Type", 33698)			, L"Sampler Type"),
	m_fallOffType       (*this, FS_TEX_DOT_FALL_OFF_TYPE                , RhLocalizeString( L"Fall-off Type", 33699)		, L"Fall-off Type"),
	m_compositionType   (*this, FS_TEX_DOT_COMPOSITION_TYPE             , RhLocalizeString( L"Composition Type", 33700)		, L"Composition Type"),
	m_sampleAreaSize    (*this, FS_TEX_DOT_SAMPLE_AREA_SIZE             , RhLocalizeString( L"Pattern Size", 33701)			, L"Pattern Size"),
	m_numberOfDots      (*this, FS_TEX_DOT_BEST_CANDIDATE_NUMBER_OF_DOTS, RhLocalizeString( L"# of Dots", 33702)			, L"# of Dots"),
	m_jitter            (*this, FS_TEX_DOT_STRATIFIED_JITTER            , RhLocalizeString( L"Jitter", 33703)				, L"Jitter"),
	m_gridSize          (*this, FS_TEX_DOT_STRATIFIED_GRID_SIZE         , RhLocalizeString( L"Grid Size", 33704)			, L"Grid Size"),
	m_dotProbability    (*this, FS_TEX_DOT_STRATIFIED_DOT_PROBABILITY   , RhLocalizeString( L"Dot Probability", 33705)		, L"Dot Probability"),
	m_radius            (*this, FS_TEX_DOT_RADIUS                       , RhLocalizeString( L"Radius", 33706)				, L"Radius"),
	m_radiusVariance    (*this, FS_TEX_DOT_RADIUS_VARY                  , RhLocalizeString( L"Radius Variance", 33707)		, L"Radius Variance"),
	m_amplitude         (*this, FS_TEX_DOT_AMPLITUDE                    , RhLocalizeString( L"Amplitude", 33708)			, L"Amplitude"),
	m_amplitudeVariance (*this, FS_TEX_DOT_AMPLITUDE_VARY               , RhLocalizeString( L"Amplitude Variance", 33709)	, L"Amplitude Variance"),
	m_hueVariance       (*this, FS_TEX_DOT_HUE_VARY                     , RhLocalizeString( L"Hue Variance", 33710)			, L"Hue Variance"),
	m_saturationVariance(*this, FS_TEX_DOT_SATURATION_VARY              , RhLocalizeString( L"Saturation Variance", 33711)	, L"Saturation Variance"),
	m_bRings            (*this, FS_TEX_DOT_RINGS                        , RhLocalizeString( L"Rings", 33712)				, L"Rings"),
	m_ringRadius        (*this, FS_TEX_DOT_RING_RADIUS                  , RhLocalizeString( L"Ring Radius", 33713)			, L"Ring Radius"),
	m_ringRadiusVariance(*this, FS_TEX_DOT_RING_RADIUS_VARY             , RhLocalizeString( L"Ring Radius Variance", 33714)	, L"Ring Radius Variance")
{
	m_sampleAreaSize = 5;
	m_numberOfDots = 1;
	m_jitter = 0.0;
	m_gridSize = 1;
	m_dotProbability = 1.0;
	m_radius = 0.1;
	m_radiusVariance = 0.0;
	m_amplitude = 1.0;
	m_amplitudeVariance = 0.0;
	m_hueVariance = 0.0;
	m_saturationVariance = 0.0;
	m_bRings = false;
	m_ringRadius = 0.25;
	m_ringRadiusVariance = 0.0;
	m_samplerType     = int(RhRdkDotSamplerType::BestCandidate);
	m_fallOffType     = int(RhRdkDotFallOffType::Flat);
	m_compositionType = int(RhRdkDotCompositionType::Standard);

	m_sampleAreaSize    .SetLimits(1, 20);
	m_numberOfDots      .SetLimits(1, 100);
	m_jitter            .SetLimits(0.0, 1.0);
	m_gridSize          .SetLimits(1, 10);
	m_dotProbability    .SetLimits(0.0, 1.0);
	m_radius            .SetLimits(0.0, 1.0);
	m_radiusVariance    .SetLimits(0.0, 1.0);
	m_amplitude         .SetLimits(0.0, 1.0);
	m_amplitudeVariance .SetLimits(0.0, 1.0);
	m_hueVariance       .SetLimits(0.0, 1.0);
	m_saturationVariance.SetLimits(0.0, 1.0);
	m_ringRadius        .SetLimits(0.0, 1.0);
	m_ringRadiusVariance.SetLimits(0.0, 1.0);

	m_amplitudeVariance .SetConversion(CRhRdkContentField::conv_percentile);
	m_hueVariance       .SetConversion(CRhRdkContentField::conv_percentile);
	m_saturationVariance.SetConversion(CRhRdkContentField::conv_percentile);
	m_ringRadiusVariance.SetConversion(CRhRdkContentField::conv_percentile);
	m_radiusVariance    .SetConversion(CRhRdkContentField::conv_percentile);
	m_ringRadius        .SetConversion(CRhRdkContentField::conv_one_minus|CRhRdkContentField::conv_percentile);
}

RhRdkDotSamplerType CRhRdkDotTexture::SamplerType(void) const
{
	return RhRdkDotSamplerType(int(m_samplerType));
}

void CRhRdkDotTexture::SetSamplerType(RhRdkDotSamplerType type)
{
	m_samplerType.SetValue(int(type));
}

RhRdkDotFallOffType CRhRdkDotTexture::FallOffType(void) const
{
	return RhRdkDotFallOffType(int(m_fallOffType));
}

void CRhRdkDotTexture::SetFallOffType(RhRdkDotFallOffType type)
{
	m_fallOffType.SetValue(int(type));
}

RhRdkDotCompositionType CRhRdkDotTexture::CompositionType(void) const
{
	return RhRdkDotCompositionType(int(m_compositionType));
}

void CRhRdkDotTexture::SetCompositionType(RhRdkDotCompositionType type)
{
	m_compositionType.SetValue(int(type));
}

int CRhRdkDotTexture::NumberOfDots(void) const
{
	return m_numberOfDots;
}

void CRhRdkDotTexture::SetNumberOfDots(int i)
{
	m_numberOfDots.SetValue(i);
}

double CRhRdkDotTexture::DotProbability(void) const
{
	return m_dotProbability;
}

void CRhRdkDotTexture::SetDotProbability(double d)
{
	m_dotProbability.SetValue(d);
}

double CRhRdkDotTexture::Jitter(void) const
{
	return m_jitter;
}

void CRhRdkDotTexture::SetJitter(double d)
{
	m_jitter.SetValue(d);
}

int CRhRdkDotTexture::GridSize(void) const
{
	return m_gridSize;
}

void CRhRdkDotTexture::SetGridSize(int i)
{
	m_gridSize.SetValue(i);
}

int CRhRdkDotTexture::PatternSize(void) const
{
	return m_sampleAreaSize;
}

void CRhRdkDotTexture::SetPatternSize(int i)
{
	m_sampleAreaSize.SetValue(i);
}

double CRhRdkDotTexture::Radius(void) const
{
	return m_radius;
}

void CRhRdkDotTexture::SetRadius(double d)
{
	m_radius.SetValue(d);
}

double CRhRdkDotTexture::RadiusVariance(void) const
{
	return m_radiusVariance;
}

void CRhRdkDotTexture::SetRadiusVariance(double d)
{
	m_radiusVariance.SetValue(d);
}

bool CRhRdkDotTexture::Rings(void) const
{
	return m_bRings;
}

void CRhRdkDotTexture::SetRings(bool b)
{
	m_bRings.SetValue(b);
}

double CRhRdkDotTexture::RingRadius(void) const
{
	return m_ringRadius;
}

void CRhRdkDotTexture::SetRingRadius(double d)
{
	m_ringRadius.SetValue(d);
}

double CRhRdkDotTexture::RingRadiusVariance(void) const
{
	return m_ringRadiusVariance;
}

void CRhRdkDotTexture::SetRingRadiusVariance(double d)
{
	m_ringRadiusVariance.SetValue(d);
}

double CRhRdkDotTexture::Amplitude(void) const
{
	return m_amplitude;
}

void CRhRdkDotTexture::SetAmplitude(double d)
{
	m_amplitude.SetValue(d);
}

double CRhRdkDotTexture::AmplitudeVariance(void) const
{
	return m_amplitudeVariance;
}

void CRhRdkDotTexture::SetAmplitudeVariance(double d)
{
	m_amplitudeVariance.SetValue(d);
}

double CRhRdkDotTexture::HueVariance(void) const
{
	return m_hueVariance;
}

void CRhRdkDotTexture::SetHueVariance(double d)
{
	m_hueVariance.SetValue(d);
}

double CRhRdkDotTexture::SaturationVariance(void) const
{
	return m_saturationVariance;
}

void CRhRdkDotTexture::SetSaturationVariance(double d)
{
	m_saturationVariance.SetValue(d);
}

UUID CRhRdkDotTexture::RenderEngineId(void) const
{
	return uuidUniversalRenderEngine;
}

UUID CRhRdkDotTexture::PlugInId(void) const
{
	return rdk.RhinoPlugInUuid();
}

UUID CRhRdkDotTexture::TypeId(void) const
{
	return uuidAdvancedDotTextureType;
}

ON_wString CRhRdkDotTexture::InternalName(void) const
{
	return L"rdk-dots-texture";
}

ON_wString CRhRdkDotTexture::TypeName(void) const
{
	return RhLocalizeString( L"Dots Texture", 23098);
}

ON_wString CRhRdkDotTexture::TypeDescription(void) const
{
	return RhLocalizeString( L"Dots Texture.", 23099);
}

const wchar_t* CRhRdkDotTexture::Category(void) const
{
	return RDK_CAT_PROCEDURAL_2D;
}

bool CRhRdkDotTexture::IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& f, const wchar_t* wszChildSlotName) const
{
	return f.IsKind(CRhRdkContent::Kinds::Texture);
}

// Optimizing stuff

class CDot
{
public:
	CDot(const ON_2dPoint& ptCenter = ON_2dPoint(0.0, 0.0), double radius = 0.0, int value = 0);
	ON_2dPoint m_ptCenter;
	double m_radius;
	int m_value;
};

CDot::CDot(const ON_2dPoint& ptCenter, double radius, int value)
{
	m_ptCenter.x = ptCenter.x;
	m_ptCenter.y = ptCenter.y;
	m_radius = radius;
	m_value = value;
}

class CBBox
{
public:
	// Important! xMin must be less than or equal to xMax and the same for yMin and yMax.
	CBBox(double xMin = 0.0, double yMin = 0.0, double xMax = 1.0, double yMax = 1.0);
	CBBox(const ON_2dPoint& ptCenter, double radius);
	bool Inside(const ON_2dPoint& point) const;
	void Expand(const ON_2dPoint& point);
	void Expand(const CBBox& bbox);
	void ShrinkTo(const ON_2dPoint& point);
	double XMin() const { return m_xMin; }
	double XMax() const { return m_xMax; }
	double YMin() const { return m_yMin; }
	double YMax() const { return m_yMax; }

private:
	double m_xMin;
	double m_yMin;
	double m_xMax;
	double m_yMax;
};

CBBox::CBBox(double xMin, double yMin, double xMax, double yMax)
{
	m_xMin = xMin;
	m_yMin = yMin;
	m_xMax = xMax;
	m_yMax = yMax;
}

CBBox::CBBox(const ON_2dPoint& ptCenter, double radius)
{
	m_xMin = ptCenter.x - radius;
	m_xMax = ptCenter.x + radius;
	m_yMin = ptCenter.y - radius;
	m_yMax = ptCenter.y + radius;
}

bool CBBox::Inside(const ON_2dPoint &point) const
{
	if (point.x < m_xMin)
		return false;
	if (point.x > m_xMax)
		return false;
	if (point.y < m_yMin)
		return false;
	if (point.y > m_yMax)
		return false;

	return true;
}

void CBBox::Expand(const ON_2dPoint& point)
{
	if (point.x < m_xMin)
		m_xMin = point.x;
	else
	if (point.x > m_xMax)
		m_xMax = point.x;

	if (point.y < m_yMin)
		m_yMin = point.y;
	else
	if (point.y > m_yMax)
		m_yMax = point.y;
}

void CBBox::Expand(const CBBox& bbox)
{
	if (m_xMin > bbox.m_xMin)
		m_xMin = bbox.m_xMin;

	if (m_xMax < bbox.m_xMax)
		m_xMax = bbox.m_xMax;

	if (m_yMin > bbox.m_yMin)
		m_yMin = bbox.m_yMin;

	if (m_yMax < bbox.m_yMax)
		m_yMax = bbox.m_yMax;
}

void CBBox::ShrinkTo(const ON_2dPoint& point)
{
	m_xMin = m_xMax = point.x;
	m_yMin = m_yMax = point.y;
}

class CDotSTNode
{
public:
	CDotSTNode(const CDot& dot1, const CDot& dot2, CDotSTNode* pChild1 = nullptr, CDotSTNode* pChild2 = nullptr);
	~CDotSTNode();

	void InsertDot(const CDot& dot);
	const CDotSTNode* Child1() const { return m_pChild1; }
	const CDotSTNode* Child2() const { return m_pChild2; }
	const CDot& Dot1() const { return m_dot1; }
	const CDot& Dot2() const { return m_dot2; }
	const CBBox& BBox1() const { return m_bbox1; }
	const CBBox& BBox2() const { return m_bbox2; }
	void GetListOfIntersectingDots(const ON_2dPoint& point, ON_SimpleArray<int>& aResultValues, ON_SimpleArray<double>& aResultSquaredDistances) const;

private:
	void FindIntersecting(const ON_2dPoint& point, ON_SimpleArray<int>& aResultValues, ON_SimpleArray<double>& aResultSquaredDistances) const;
	CDot m_dot1;
	CDot m_dot2;
	CDotSTNode* m_pChild1;
	CDotSTNode* m_pChild2;
	CBBox m_bbox1;
	CBBox m_bbox2;
};

CDotSTNode::CDotSTNode(const CDot& dot1, const CDot& dot2, CDotSTNode* pChild1, CDotSTNode* pChild2)
	:
	m_bbox1(dot1.m_ptCenter, dot1.m_radius),
	m_bbox2(dot2.m_ptCenter, dot2.m_radius)
{
	m_dot1 = dot1;
	m_dot2 = dot2;
	m_pChild1 = pChild1;
	m_pChild2 = pChild2;
}

CDotSTNode::~CDotSTNode()
{
	SAFE_DELETE(m_pChild1);
	SAFE_DELETE(m_pChild2);
}

void CDotSTNode::InsertDot(const CDot& dot)
{
	const double sqDistToChild1 = (dot.m_ptCenter - m_dot1.m_ptCenter).LengthSquared();
	const double sqDistToChild2 = (dot.m_ptCenter - m_dot2.m_ptCenter).LengthSquared();

	if (sqDistToChild1 < sqDistToChild2)
	{
		m_bbox1.Expand(CBBox(dot.m_ptCenter, dot.m_radius));

		if (nullptr != m_pChild1)
			m_pChild1->InsertDot(dot);
		else
			m_pChild1 = new CDotSTNode(m_dot1, dot);
	}
	else
	{
		m_bbox2.Expand(CBBox(dot.m_ptCenter, dot.m_radius));

		if (nullptr != m_pChild2)
			m_pChild2->InsertDot(dot);
		else
			m_pChild2 = new CDotSTNode(m_dot2, dot);
	}
}

void CDotSTNode::GetListOfIntersectingDots(const ON_2dPoint& point, ON_SimpleArray<int>& aResultValues, ON_SimpleArray<double>& aResultSquaredDistances) const
{
	double x = point.x;
	double y = point.y;

	NormalizeUV(x, y);

	// Jussi, 7-Feb-2012: Sample 9 times with (-1/0/+1,-1/0/+1) offsets to find all possible intersections.
	for (double tx = -1.0; tx <= 1.0; tx += 1.0)
	{
		for (double ty = -1.0; ty <= 1.0; ty += 1.0)
		{
			FindIntersecting(ON_2dPoint(x + tx, y + ty), aResultValues, aResultSquaredDistances);
		}
	}
}

void CDotSTNode::FindIntersecting(const ON_2dPoint& point, ON_SimpleArray<int>& aResultValues, ON_SimpleArray<double>& aResultSquaredDistances) const
{
	if (nullptr == m_pChild2)
	{
		const double sqDistToChild2 = (m_dot2.m_ptCenter - point).LengthSquared();
		if (sqDistToChild2 <= m_dot2.m_radius * m_dot2.m_radius)
		{
			aResultValues.Append(m_dot2.m_value);
			aResultSquaredDistances.Append(sqDistToChild2);
		}
	}
	else
	{
		if (m_bbox2.Inside(point))
			m_pChild2->FindIntersecting(point, aResultValues, aResultSquaredDistances);
	}

	if (nullptr == m_pChild1)
	{
		const double sqDistToChild1 = (m_dot1.m_ptCenter - point).LengthSquared();
		if (sqDistToChild1 <= m_dot1.m_radius * m_dot1.m_radius)
		{
			aResultValues.Append(m_dot1.m_value);
			aResultSquaredDistances.Append(sqDistToChild1);
		}
	}
	else
	{
		if (m_bbox1.Inside(point))
			m_pChild1->FindIntersecting(point, aResultValues, aResultSquaredDistances);
	}
}

class CDotSearchTree
{
public:
	~CDotSearchTree() { SAFE_DELETE(m_pRootNode); SAFE_DELETE(m_pSingleDot); }

	void AddDot(double x, double y, double radius, int value);
	void IntersectingDots(double x, double y, ON_SimpleArray<int>& aResultValues, ON_SimpleArray<double>& aResultSquaredDistances) const;

private:
	int m_dotCount = 0;
	CDot* m_pSingleDot = nullptr;
	CDotSTNode* m_pRootNode = nullptr;
};

void CDotSearchTree::AddDot(double x, double y, double radius, int value)
{
	if (m_dotCount > 1 && m_pRootNode != nullptr)
	{
		m_pRootNode->InsertDot(CDot(ON_2dPoint(x, y), radius, value));
		m_dotCount++;
	}
	else
	{
		if (m_dotCount == 0 && m_pSingleDot == nullptr)
		{
			m_pSingleDot = new CDot(ON_2dPoint(x, y), radius, value);
			m_dotCount++;
		}
		else
		{
			if (m_dotCount == 1 && m_pSingleDot != nullptr && m_pRootNode == nullptr)
			{
				m_pRootNode = new CDotSTNode(*m_pSingleDot, CDot(ON_2dPoint(x, y), radius, value));
				m_dotCount++;
			}
		}
	}
}

void CDotSearchTree::IntersectingDots(double x, double y, ON_SimpleArray<int> &aResultValues, ON_SimpleArray<double> &aResultSquaredDistances) const
{
	if (m_dotCount > 1 && m_pRootNode != nullptr)
	{
		m_pRootNode->GetListOfIntersectingDots(ON_2dPoint(x, y), aResultValues, aResultSquaredDistances);
	}
	else
	if (m_dotCount == 1 && m_pSingleDot != nullptr)
	{
		// Jussi, 7-Feb-2012: Test 9 times with (-1/0/+1,-1/0/+1) offsets to find all possible intersections.
		for (double tx = -1.0; tx <= 1.0; tx += 1.0)
		{
			for (double ty = -1.0; ty <= 1.0; ty += 1.0)
			{
				const double dx = x + tx - m_pSingleDot->m_ptCenter.x; 
				const double dy = y + ty - m_pSingleDot->m_ptCenter.y;
				const double sqDist = dx * dx + dy * dy;
				if (sqDist <= m_pSingleDot->m_radius * m_pSingleDot->m_radius)
				{
					aResultValues.Append(m_pSingleDot->m_value);
					aResultSquaredDistances.Append(sqDist);
				}
			}
		}
	}
}

// CRhRdkDotTexture::Evaluator

class CRhRdkDotTexture::Evaluator : public CRhRdkTwoColorEvaluator
{
public:
	Evaluator(const CRhRdkDotTexture& texture, CEvalFlags ef);
	virtual ~Evaluator();

	virtual CRhRdkColor GetColorSample(const ON_3dPoint& uvw, const ON_3dVector& duvwdx,  const ON_3dVector& duvwdy, void* pvData) const;

private:
	mutable bool m_bInitialized = false;
	mutable CCriticalSection m_cs;
	bool _Initialize(void) const;

	RhRdkDotSamplerType m_samplerType;
	RhRdkDotFallOffType m_fallOffType;
	RhRdkDotCompositionType m_compositionType;

	class CDotData
	{
	public:
		bool m_bInitialized = false;
		double m_dRadius = 0.0;
		double m_dRingRadius = 0.0;
		double m_dAmplitude = 0.0;
		double m_dHueAdjust = 0.0;
		double m_dSaturationFactor = 0.0;
	};

	int m_sampleAreaSize;
	int m_numberOfDots;
	double m_jitter;
	int m_gridSize;
	double m_dotProbability;
	double m_radius;
	double m_radiusVariance;
	double m_amplitude;
	double m_amplitudeVariance;
	double m_hueVariance;
	double m_saturationVariance;
	bool m_bRings;
	double m_ringRadius;
	double m_ringRadiusVariance;

	mutable CLBPRh2dStratifiedSamplePattern* m_pStratifiedSamplePattern = nullptr;
	mutable ON_SimpleArray<int> m_aIgnoreDots;
	mutable CDotSearchTree m_searchTree;
	mutable CDotData* m_pDotData = nullptr;
	mutable int m_iDotDataCount = 0;
};

CRhRdkDotTexture::Evaluator::Evaluator(const CRhRdkDotTexture& texture, CEvalFlags ef)
	:
	CRhRdkTwoColorEvaluator(texture, ef)
{
	m_samplerType        = texture.SamplerType();
	m_fallOffType        = texture.FallOffType();
	m_compositionType    = texture.CompositionType();
	m_sampleAreaSize     = texture.PatternSize();
	m_numberOfDots       = texture.NumberOfDots();
	m_gridSize           = texture.GridSize();
	m_jitter             = texture.Jitter();
	m_dotProbability     = texture.DotProbability();
	m_radius             = texture.Radius();
	m_radiusVariance     = texture.RadiusVariance();
	m_amplitude          = texture.Amplitude();
	m_amplitudeVariance  = texture.AmplitudeVariance();
	m_hueVariance        = texture.HueVariance();
	m_saturationVariance = texture.SaturationVariance();
	m_bRings             = texture.Rings();
	m_ringRadius         = texture.RingRadius() * 0.5;
	m_ringRadiusVariance = texture.RingRadiusVariance();
}

CRhRdkDotTexture::Evaluator::~Evaluator()
{
	SAFE_DELETE(m_pStratifiedSamplePattern);
	delete [] m_pDotData;
}

CRhRdkColor CRhRdkDotTexture::Evaluator::GetColorSample(const ON_3dPoint& uvwOriginal, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, void* pvData) const
{
	bool bFailed = false;

	ON_3dVector uvw = LocalMappingTransform() * uvwOriginal / (double)m_sampleAreaSize;

	const ON_3dVector d1 = LocalMappingTransform() * duvwdx;
	const ON_3dVector d2 = LocalMappingTransform() * duvwdx;

	const CRhRdkColor originalBackgroundColor = OutputColor(1, uvwOriginal, duvwdx, duvwdy, pvData);
	const CRhRdkColor originalDotColor = OutputColor(2, uvwOriginal, duvwdx, duvwdy, pvData);

	m_cs.Lock();
	if (!m_bInitialized)
	{
		_Initialize();
	}
	m_cs.Unlock();

	CRhRdkColor colOut;

	if (m_bInitialized)
	{
		ON_SimpleArray<int> aPointIndexes;
		ON_SimpleArray<double> aSquaredDistances;
		m_searchTree.IntersectingDots(uvw.x, uvw.y, aPointIndexes, aSquaredDistances);

		CRhRdkColor color(originalBackgroundColor);
		int colorsAdded = 0;
		int index = 0;

		double standardCompositionValue = 0.0;
		while (!bFailed && (index < aPointIndexes.Count()))
		{
			const int pointIndex = aPointIndexes[index];

			RHRDK_ASSERT((pointIndex >= 0) && (pointIndex < m_iDotDataCount))

			const CDotData& dotData = m_pDotData[pointIndex];

			const double radius = dotData.m_dRadius;

			// Calculate virtual radius and virtual distance
			double virtualRadius = radius;
			double virtualSquaredDistance = aSquaredDistances[index];
			double virtualDistance = sqrt(virtualSquaredDistance);

			if (m_bRings && m_ringRadius != 1.0)
			{
				const double ringRadius = dotData.m_dRingRadius;
				virtualRadius = radius * ringRadius;
				const double actualRingRadius = radius * (1.0 - ringRadius);
				virtualDistance = std::abs(actualRingRadius - virtualDistance);
				virtualSquaredDistance = virtualDistance * virtualDistance;
			}

			if (virtualSquaredDistance < virtualRadius * virtualRadius)
			{
				const double amplitude = dotData.m_dAmplitude;
				const double hueAdjust = dotData.m_dHueAdjust;
				const double saturationFactor = dotData.m_dSaturationFactor;

				double value = 0.0;
				switch (m_fallOffType)
				{
					case RhRdkDotFallOffType::Flat:
						value = amplitude;
						break;

					case RhRdkDotFallOffType::Linear:
						value = amplitude * (1.0 - sqrt(virtualSquaredDistance) / virtualRadius);
						break;

					case RhRdkDotFallOffType::Cubic:
						{
							const double param = sqrt(virtualSquaredDistance) / virtualRadius;
							const double alpha = 1.0;
							const double a = 2.0 * alpha;
							const double b = -3.0 * alpha;
							const double c = 0.0;
							const double d = alpha;
							value = amplitude * (((a * param + b) * param + c) * param + d);
						}
						break;

					case RhRdkDotFallOffType::Elliptic:
						{
							const double xSquared = virtualSquaredDistance / (virtualRadius * virtualRadius);
							value = amplitude * sqrt(1.0 - xSquared);
						}
						break;
					default:
						value = amplitude;
						break;
				}

				float h = 0.0;
				float s = 0.0;
				float b = 0.0;
				CRhRdkColor dotColor(originalDotColor);

				// Jussi, 12-1-2011: Hue variance was broken due to invalid use of hsb2rgb.
				// Hue value passed into hsb2rgb must in degrees and in the range of [0,360).
				if (!rgb2hsb(originalDotColor, h, s, b))
				{
					bFailed = true;
				}
				else
				{
					float shiftedHue = fmodf(h + (float)hueAdjust, 360.0f);

					if (shiftedHue < 0)
						shiftedHue += 360.0f;

					RHRDK_ASSERT(0.0f <= shiftedHue && shiftedHue < 360.0f);

					if (!hsb2rgb(shiftedHue, s * (float)saturationFactor, b, dotColor))
						bFailed = true;
				}

				switch (m_compositionType)
				{
					case RhRdkDotCompositionType::Maximum:
						{
						CRhRdkColor startColor = originalBackgroundColor;
						CRhRdkColor endColor = dotColor;
						startColor.MultiplyBy( 1.0f - float(value), true);
						endColor.MultiplyBy(float(value), true);
						startColor += endColor;
						color.Set(max(color.FRed()  , startColor.FRed()),
								  max(color.FGreen(), startColor.FGreen()),
								  max(color.FBlue() , startColor.FBlue()),
								  max(color.FAlpha(), startColor.FAlpha()));
						}
						break;

					case RhRdkDotCompositionType::Addition:
						dotColor.MultiplyBy(float(value), true);
						color += dotColor;
						break;

					case RhRdkDotCompositionType::Subtraction:
						dotColor.MultiplyBy(float(value), true);
						color -= dotColor;
						break;

					case RhRdkDotCompositionType::Multiplication:
						{
							CRhRdkColor startColor = originalBackgroundColor;
							CRhRdkColor endColor = dotColor;
							startColor.MultiplyBy(1.0f - float(value), true);
							endColor.MultiplyBy(float(value), true);
							endColor += startColor;
							color *= endColor;
						}
						break;

					case RhRdkDotCompositionType::Average:
						{
							CRhRdkColor startColor = originalBackgroundColor;
							CRhRdkColor endColor = dotColor;
							startColor.MultiplyBy(1.0f - float(value), true);
							endColor.MultiplyBy(float(value), true);
							color += startColor;
							color += endColor;
						}
						break;

					case RhRdkDotCompositionType::Standard:
						standardCompositionValue += value;
						color = dotColor;
						break;

					default:
						break;
				}

				colorsAdded++;
			}

			index++;
		}

		if (m_compositionType == RhRdkDotCompositionType::Average)
		{
			color /= (float)(colorsAdded == 0 ? 1 : colorsAdded);
		}
		else
		if (m_compositionType == RhRdkDotCompositionType::Standard)
		{
			//const double compVal = std::max(0.0, std::min(1.0, standardCompositionValue));
			const float compVal = (float)standardCompositionValue;
			const float compValNeg = 1.f - compVal;
			const float alpha = color.FAlpha();

			//Alpha values must be multiplied here - the float * color operator doesn't do that.
			//http://mcneel.myjetbrains.com/youtrack/issue/RH-31318
			color = compValNeg * originalBackgroundColor + compVal * color;
			color.SetAlpha(compValNeg * originalBackgroundColor.FAlpha() + compVal * alpha);
		}

		color.Clamp();

		colOut = color;
	}

	if (bFailed)
		colOut.Set(1.0, 0.0, 0.0);

	return colOut;
}

bool CRhRdkDotTexture::Evaluator::_Initialize(void) const
{
	if (m_bInitialized)
		return true;

	bool bDeleteSamplePatternAfterUse = false;

	ILBPRh2dSamplePattern* pPattern = nullptr;

	if (m_samplerType == RhRdkDotSamplerType::Stratified)
	{
		const int actualGridSize = m_gridSize * m_sampleAreaSize;
		m_pStratifiedSamplePattern = new CLBPRh2dStratifiedSamplePattern(actualGridSize, m_jitter);
		
		CLBPMersenneTwister randomizer(m_gridSize);
		for (int k = 0; k < actualGridSize * actualGridSize; k++)
		{
			const double value = double(randomizer.RandomFloat1());
			if (value > m_dotProbability)
				m_aIgnoreDots.Append(k);
		}

		m_aIgnoreDots.QuickSort(ON_CompareIncreasing);

		pPattern = m_pStratifiedSamplePattern;
	}
	else
//	if (m_samplerType == RhRdkDotSamplerType::BestCandidate)
	{
		pPattern = rdk.BestCandidateSamplePatternCreator2d().NewPattern(m_numberOfDots * m_sampleAreaSize * m_sampleAreaSize);
		bDeleteSamplePatternAfterUse = true;
	}

	if (nullptr == pPattern)
		return false;

	m_iDotDataCount = pPattern->PointCount();

	m_pDotData = new CDotData[m_iDotDataCount];

	const float minValue = -1.0;
	const float maxValue = +1.0;
	const double minAmplitude = m_amplitude * (1.0 - 0.5 * m_amplitudeVariance);
	const double maxAmplitude = m_amplitude * (1.0 + 1.0 * m_amplitudeVariance);
	const double minHueAdjust = -180.0 * m_hueVariance;
	const double maxHueAdjust = 180.0 * m_hueVariance;
	const double minSaturationFactor = (1.0 - 0.5 * m_saturationVariance);
	const double maxSaturationFactor = (1.0 + 1.0 * m_saturationVariance);

	// Jussi, 7-Feb-2012: Limit radius and ring radius boundaries.
	const double minRadius = max(0.0, m_radius * (1.0 - 0.5 * m_radiusVariance)) / double(m_sampleAreaSize);
	const double maxRadius = min(1.0, m_radius * (1.0 + 1.0 * m_radiusVariance)) / double(m_sampleAreaSize);
	const double minRingRadius = max(0.0, m_ringRadius * (1.0 - 0.5 * m_ringRadiusVariance));
	const double maxRingRadius = min(1.0, m_ringRadius * (1.0 + 1.0 * m_ringRadiusVariance));

	for (int i = 0; i < m_iDotDataCount; i++)
	{
		if (-1 == m_aIgnoreDots.BinarySearch(&i, ON_CompareIncreasing))
		{
			ON_2dPoint point;
			if (pPattern->GetPoint(i, point))
			{
				CDotData& dotData = m_pDotData[i];

				CLBPMersenneTwister randomizer(i * 97);
				const double radiusNoise = randomizer.RandomFloat2();
				const double radiusRandValue = (radiusNoise - minValue) / (maxValue - minValue);
				const double radius = minRadius + (maxRadius - minRadius) * radiusRandValue;
				dotData.m_dRadius = radius;
				m_searchTree.AddDot(point.x, point.y, radius, i);

				if (m_bRings && m_ringRadius != 1.0)
				{
					const double ringRadiusNoise = randomizer.RandomFloat2();
					const double ringRadiusRandValue = (ringRadiusNoise - minValue) / (maxValue - minValue);
					dotData.m_dRingRadius = minRingRadius + (maxRingRadius - minRingRadius) * ringRadiusRandValue;
				}

				const double amplitudeNoise = randomizer.RandomFloat2();
				const double amplitudeRandValue = (amplitudeNoise - minValue) / (maxValue - minValue);
				dotData.m_dAmplitude = minAmplitude + (maxAmplitude - minAmplitude) * amplitudeRandValue;

				const double hueNoise = randomizer.RandomFloat2();
				const double hueRandValue = (hueNoise - minValue) / (maxValue - minValue);
				dotData.m_dHueAdjust = minHueAdjust + (maxHueAdjust - minHueAdjust) * hueRandValue;

				const double saturationNoise = randomizer.RandomFloat2();
				const double saturationRandValue = (saturationNoise - minValue) / (maxValue - minValue);
				dotData.m_dSaturationFactor = minSaturationFactor + (maxSaturationFactor - minSaturationFactor) * saturationRandValue;
			}
		}
	}

	if (bDeleteSamplePatternAfterUse)
		delete pPattern;

	m_bInitialized = true;

	return true;
}

IRhRdkTextureEvaluator* CRhRdkDotTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	return new Evaluator(*this, ef);
}
