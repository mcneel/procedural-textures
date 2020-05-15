
#include "stdafx.h"
#include "RhRdkPhysicalSkyTexture.h"
#include "RhRdkTextureUtilities.h"
#include "RhRcmBasicEnvironment.h"

const wchar_t* EnglishPhysicalSkyString(e_PSS_StringId id)
{
	switch(id)
	{
	case pss_date:				return L"Date and Time";
	case pss_use_doc_sun:		return L"Use document sun";
	case pss_show_sun:			return L"Show sun";
	case pss_sun_size:			return L"Sun size";
	case pss_coronal_color:		return L"Sun coronal color";
	case pss_sun_brightness:	return L"Sun brightness";
	case pss_atmos_den:			return L"Atmospheric density";
	case pss_light_scat:		return L"Light scattering";
	case pss_part_scat:			return L"Particle scattering (haze)";
	case pss_light_wave:		return L"Light wavelengths";
	case pss_exposure:			return L"Exposure";
	case pss_latlong:			return L"Earth Location";
	}

	RHRDK_ASSERT_EXPLAIN(false, "EnglishPhysicalSkyString() failed");

	return L"";
}

const wchar_t* LocalPhysicalSkyString(e_PSS_StringId id)
{
	switch(id)
	{
	case pss_date:				return RhLocalizeString( L"Date and Time", 35083);
	case pss_use_doc_sun:		return RhLocalizeString( L"Use document sun", 35084);
	case pss_show_sun:			return RhLocalizeString( L"Show sun", 35085);
	case pss_sun_size:			return RhLocalizeString( L"Sun size", 35086);
	case pss_coronal_color:		return RhLocalizeString( L"Sun coronal color", 35087);
	case pss_sun_brightness:	return RhLocalizeString( L"Sun brightness", 35088);
	case pss_atmos_den:			return RhLocalizeString( L"Atmospheric density", 35089);
	case pss_light_scat:		return RhLocalizeString( L"Light scattering", 35090);
	case pss_part_scat:			return RhLocalizeString( L"Particle scattering (haze)", 35091);
	case pss_light_wave:		return RhLocalizeString( L"Light wavelengths", 35092);
	case pss_exposure:			return RhLocalizeString( L"Exposure", 35093);
	case pss_latlong:			return RhLocalizeString( L"Earth Location", 35252);
	}

	RHRDK_ASSERT_EXPLAIN(false, "LocalPhysicalSkyString() failed");

	return L"";
}

class PhysicalSkyEval : public CRhRdkTextureEvaluator
{
public:
	PhysicalSkyEval(IRhRdkTextureEvaluator::CEvalFlags ef) : CRhRdkTextureEvaluator(ef)
	{
		UseEarthSettings();
	}

protected:
	// GLSL lookalikes...
	#define dot(a, b)     ON_DotProduct( a, b )
	#define vec3          ON_3fVector

	void DeleteThis(void) { delete this; }

	static __forceinline float __scale(float fCos)
	{
		float x = 1.0f - fCos;
		return 0.25f * exp(-0.00287f + x*(0.459f + x*(3.83f + x*(-6.80f + x*5.25f))));
	}

	__forceinline vec3  __HDR(const vec3& LDR) const
	{
		float  fExposure = -m_fExposure;

		return vec3(1.0f, 1.0f, 1.0f) - vec3( exp( fExposure * LDR.x ), exp( fExposure * LDR.y ), exp( fExposure * LDR.z ) );
	}

	// another GLSL lookalike...although this one needs to be a true function.
	static vec3 normalize(vec3  v) { v.Unitize(); return v; }

	bool GetColor(const ON_3dPoint& uvw, const ON_3dVector& duvwdx, const ON_3dVector& duvwdy, CRhRdkColor& colOut, void* pvData) const
	{
		vec3 vSkyDir;
		vec3 vLightDir( -m_SunDir);

		//This gives you the view direction into space.
		EquirectToWorld(float(uvw.x), float(uvw.y), vSkyDir);

		// Need to swap and flip y and z so that projection matches OpenGL's...this is so that
		// the same code in the shader can be used here pretty much interchangeably.
		float tmp = vSkyDir.y;
		vSkyDir.y = vSkyDir.z;
		vSkyDir.z = -tmp;

		colOut = CRhRdkColor::black;

		//////////////////////////////////////
		// BEGIN: Physical Sky algorithm...
		if ( vSkyDir.z < -0.15f )
			return true;

		const int   nSamples = 3;
		const float fSamples = float(nSamples);

		float g  = m_fAtmosphericDensity / 10000.0f - 1.0f;
		float g2 = g*g;

		float  fInnerRadius = 1.0f;
		float  fOuterRadius = fInnerRadius + 0.025f;
		float  OR2 = fOuterRadius*fOuterRadius;
		float  fKrESun = m_fRayleighScattering*m_fSunBrightness;
		float  fKmESun = m_fMieScattering*m_fSunBrightness;
		float  fKr4PI  = float(m_fRayleighScattering*4.0f*ON_PI);
		float  fKm4PI  = float(m_fMieScattering*4.0f*ON_PI);
		float  fScale  = 1.0f / (fOuterRadius - fInnerRadius);
		float  fScaleDepth = 0.25f;
		float  fScaleOverScaleDepth = fScale / fScaleDepth;
		vec3   vGroundPos = vec3(0.0, 0.0, fInnerRadius+1.0e-6f);
		float  IR2 = vGroundPos.z*vGroundPos.z;

		// Precomputed Mie constants...
		float  M1 = 1.5f * ((1.0f - g2) / (2.0f + g2));
		float  M2 = 1.0f + g2;
		float  M3 = 2.0f * g;
		float  M4 = m_fSunSize/10.0f;
		vec3   M5 = fKmESun * m_SunColor;
		vec3   RScatter = (m_InvWavelengths * fKrESun);
		vec3   MScatter = (m_InvWavelengths * fKr4PI) + vec3( fKm4PI, fKm4PI, fKm4PI );

		// Calculate the distance 't' from the ground to the outer atmosphere... basically
		// do a quick ray:sphere intersection (with shortcuts) to get 't'...
		float B = -vGroundPos.z * vSkyDir.z; //dot( -vGroundPos, vSkyDir );
		float D = IR2 - B * B; 
		float q = sqrt( OR2 - D );
		float t = B + q;

		// Get the ray from the ground to the vertex (position on the "virtual" sky dome)
		vec3  vSkyPos = vGroundPos + vSkyDir * t;
		vec3  vRay    = normalize(vSkyPos - vGroundPos);

		// Calculate the ray's starting position, then calculate its scattering offset
		float Depth       = exp( -fScaleOverScaleDepth * 1.0e-6f );
		float StartAngle  = dot( vRay, normalize( vGroundPos ) );
		float StartOffset = Depth * __scale( StartAngle );

		// Initialize the scattering variables
		float  SampleLength = t / fSamples;
		float  ScaledLength = SampleLength * fScale;
		vec3   vSampleRay   = vRay * SampleLength;
		vec3   vSamplePoint = vGroundPos + vSampleRay * 0.5f;

		// Now iterate along the sample ray and accumulate scattering mie and rayleigh values...
		vec3   Color  = vec3(0.0, 0.0, 0.0);
		float  Height = float(vSamplePoint.Length());

		for (int i = 0; i < nSamples; i++)
		{
			vec3 vPoint = vSamplePoint;

			vPoint.Unitize();

			float  Depth2      = exp( fScaleOverScaleDepth * (fInnerRadius - Height) );
			float  LightAngle  = dot( vLightDir, vPoint );
			float  CameraAngle = dot( vRay, vPoint );
			float  Scatter     = StartOffset + Depth2 * (__scale(LightAngle) - __scale(CameraAngle));
			vec3   vAttenuate  = vec3( exp( -Scatter * MScatter.x ), exp( -Scatter * MScatter.y ), exp( -Scatter * MScatter.z ) );

			Color  += vAttenuate * (Depth2 * ScaledLength);
			Height += SampleLength;
			vSamplePoint += vSampleRay;
		}

		// Finally, compute sun and sky colors based on mie and rayleigh values...
		float fCos = dot( vLightDir, -vRay );
		float fMie = M1 * (1.0f + fCos*fCos) / pow( M2 - M3*fCos, M4 );
		//vec3  Sun  = __HDR( vec3( Color.x * M5.x, Color.y * M5.y, Color.z * M5.z ) * fMie );
		vec3  Sky  = __HDR( vec3( Color.x * RScatter.x, Color.y * RScatter.y, Color.z * RScatter.z ) );

		vec3  SkyColor = Sky;

		if ( m_bShowSun )
			SkyColor += __HDR( vec3( Color.x * M5.x, Color.y * M5.y, Color.z * M5.z ) * fMie );

		// END: Physical Sky algorithm...
		//////////////////////////////////////

		colOut.Set( SkyColor.x, SkyColor.y, SkyColor.z );

		return true;
	}

#undef dot
#undef vec3

	//bool RegisterChildCallback(IChildCallback* pChildCallback, const CRhRdkTexture& texture, void* pvData) { return false; }
	//const IChildCallback* ChildCallback(void) const { return nullptr; }

public:
	void  UseEarthSettings(void);

	void  SetSunDirection(const ON_3dVector& dir) { m_SunDir = dir;                     }
	void  SetShowSun(bool show)                   { m_bShowSun = show;                  }
	void  SetDensity(float  atmDensity)           { m_fAtmosphericDensity = atmDensity; }
	void  SetRayleighScattering(float  RScat)     { m_fRayleighScattering = RScat;      }
	void  SetMieScattering(float MScat)           { m_fMieScattering      = MScat;      }
	void  SetWavelengths(const ON_3dVector& wl)   { m_Wavelengths         = wl;
													m_InvWavelengths.x = 1.0f/powf( m_Wavelengths.x, 4.0f );
													m_InvWavelengths.y = 1.0f/powf( m_Wavelengths.y, 4.0f );
													m_InvWavelengths.z = 1.0f/powf( m_Wavelengths.z, 4.0f );
												  }
	void  SetSunBrightness(float  sb)             { m_fSunBrightness      = sb;         }
	void  SetSunSize(float ss)                    { m_fSunSize            = ss;         }
	void  SetSunColor(const ON_Color& sc)         { m_SunColor            = ON_3fVector((float)sc.FractionRed(), 
																						(float)sc.FractionGreen(), 
																						(float)sc.FractionBlue()) ; }
	void  SetExposure(float fExp)                 { m_fExposure = fExp; }

private:
	ON_3dVector   m_SunDir;
	bool          m_bShowSun;
	float         m_fAtmosphericDensity;
	float         m_fRayleighScattering;
	float         m_fMieScattering;
	ON_3fVector   m_Wavelengths;
	ON_3fVector   m_InvWavelengths;
	float         m_fSunBrightness;
	float         m_fSunSize;
	float         m_fExposure;
	ON_3fVector   m_SunColor;

	int           m_Iterations;
	ON_3fVector   m_AvgSun;
};

/////////////////////////////////////////////////////////////////////////////
//
void PhysicalSkyEval::UseEarthSettings(void)
{
	m_fAtmosphericDensity = 50.0f;
	m_fRayleighScattering = 0.00250f;
	m_fMieScattering      = 0.00200f;
	m_Wavelengths         = ON_3fVector( 0.650f, 0.570f, 0.475f );
	m_fSunBrightness      = 18.0f;
	m_fSunSize            = 15.0f;
	m_fExposure           = 1.0f;
	m_SunColor            = ON_3fVector( 1.0f, 1.0f, 0.874509f );

	m_InvWavelengths.x = 1.0f/powf( m_Wavelengths.x, 4.0f );
	m_InvWavelengths.y = 1.0f/powf( m_Wavelengths.y, 4.0f );
	m_InvWavelengths.z = 1.0f/powf( m_Wavelengths.z, 4.0f );
}

IRhRdkTextureEvaluator* CRhRdkPhysicalSkyTexture::NewTextureEvaluator(IRhRdkTextureEvaluator::CEvalFlags ef) const
{
	// You shouldn't use the document sun from within the evaluator, so get all of the data out here.

	ON_3dVector sunDirection(0.0, 0.0, 0.0);

	if (!m_bUseDocumentSun)
	{
		CRhRdkSun sun;

		time_t time = m_time;
		tm* pTime = ::localtime(&time);

		const double dLatitude = m_vLatLong.Value().AsVector2d().x;
		const double dLongitude= m_vLatLong.Value().AsVector2d().y;

		sun.Sun().SetLatitude(dLatitude);
		sun.Sun().SetLongitude(dLongitude);

		const double dTimeZone = CRhRdkCity::FindNearest(dLatitude, dLongitude).TimeZone();

		double dHours = DecimalHoursFromHMS(pTime->tm_hour, pTime->tm_min, pTime->tm_sec) - dTimeZone;

		if (dHours > 24.0)
			dHours -= 24.0;
		else
		if (dHours < 0.0)
			dHours += 24.0;

		sun.Sun().SetUTCDateTime(pTime->tm_year+1900, pTime->tm_mon+1, pTime->tm_mday, dHours);
		sunDirection = sun.Sun().Light().Direction();
	}
	else
	{
		// The content should at least be _associated_ with a document.
		const auto* pRdkDoc = RdkDocumentAssoc();
		if (nullptr != pRdkDoc)
		{
			sunDirection = pRdkDoc->Sun().Light().Direction();
		}
		else
		{
			// Not associated with a document, so just use a dummy sun.
			CRhRdkSun dummySun;
			sunDirection = dummySun.Sun().Light().Direction();
		}
	}

	// Jeff: Not exactly sure why this needs to be done... it seems the doc sun is off by -90 degrees...

	// John Croudy: This may be caused by the doc sun's North being 90 by default. See the RDK Document Sun tab.

	// Jeff: That may be true, but the pipeline code doesn't have to do this type of thing when it uses the sun's
	//       ON_Light and direction...so perhaps this is getting done by the RDK before handing it off to the pipeline.

	ON_Xform xf;
	xf.Rotation(-ON_PI / 2.0, ON_3dVector(0, 0, 1), ON_3dPoint::Origin);
	sunDirection = xf * sunDirection;

	PhysicalSkyEval* pPSEval = new PhysicalSkyEval(ef);
	pPSEval->SetSunDirection(sunDirection);
	pPSEval->SetShowSun(m_bShowSun);
	pPSEval->SetSunBrightness(m_fSunBrightness);
	pPSEval->SetSunSize(m_fSunSize);
	pPSEval->SetDensity(m_fAtmosphericDensity);
	pPSEval->SetRayleighScattering((float)m_fLightScattering / 1000.0f);
	pPSEval->SetMieScattering((float)m_fHaze / 1000.0f);
	pPSEval->SetWavelengths(m_Wavelengths);
	pPSEval->SetSunColor(m_SunColor);
	pPSEval->SetExposure(m_fExposure);

	return pPSEval;
}

void CRhRdkPhysicalSkyEnvironment::SimulateEnvironment(CRhRdkSimulatedEnvironment& env, CRhRdkTexture::TextureGeneration tg, int iSize) const
{
	CRhRdkPhysicalSkyTexture* pTexture = new CRhRdkPhysicalSkyTexture(*this);
	pTexture->Initialize();

	CRhRdkBasicEnvironment* pEnvironment = new CRhRdkBasicEnvironment;
	pEnvironment->Initialize();
	pEnvironment->SetChild(pTexture, SS_BASIC_ENV_TEXTURE_CHILD_SLOT_NAME);
	pEnvironment->SetProjection(CRhRdkBasicEnvironment::BackgroundProjections::Spherical);

	static_cast<CRhRdkEnvironment*>(pEnvironment)->SimulateEnvironment(env, tg, iSize);

	// Gets rid of the texture at the same time.
	pEnvironment->Uninitialize();
	delete pEnvironment;
}

CRhRdkTexture::EnvironmentMappingModes CRhRdkPhysicalSkyTexture::AutomaticEnvironmentMappingMode(void) const
{
	return CRhRdkTexture::EnvironmentMappingModes::Spherical;
}

void* CRhRdkPhysicalSkyTexture::EVF(const wchar_t* wszFunc, void* pvData)
{
	if (_wcsicmp(L"AutomaticEnvironmentMappingMode", wszFunc) == 0)
	{
		RHRDK_ASSERT_EVF_DEPRECATED_RETURN("AutomaticEnvironmentMappingMode");
		const auto pemm = reinterpret_cast<EnvironmentMappingModes*>(pvData);
		if (nullptr != pemm) *pemm = AutomaticEnvironmentMappingMode();
		return this;
	}

	return CRhRdkTexture::EVF(wszFunc, pvData);
}

ON_wString CRhRdkPhysicalSkyTexture::TypeName(void) const				
{ 
	return RhLocalizeString( L"Physical Sky Texture", 35147); 
}

ON_wString CRhRdkPhysicalSkyTexture::TypeDescription(void) const		
{ 
	return RhLocalizeString( L"Generates an equirectangular physical sky texture", 35148); 
}

ON_wString CRhRdkPhysicalSkyEnvironment::TypeName(void) const				
{ 
	return RhLocalizeString( L"Physical Sky Environment", 35149); 
}

ON_wString CRhRdkPhysicalSkyEnvironment::TypeDescription(void) const		
{ 
	return RhLocalizeString( L"Generates an equirectangular physical sky environment", 35150); 
}

ON_wString CRhRdkPhysicalSkyBase_AddUISections_LocalString()
{
	return RhLocalizeString( L"Physical sky parameters", 35151);
}

ON_wString CRhRdkPhysicalSkyBase_AddUISections_EnglishString()
{
	return L"Physical sky parameters";
}

CRhRdkVariant CRhRdkPhysicalSkyTexture::GetParameter(const wchar_t* wszParam) const
{
	if (_wcsicmp(L"max-simulated-height", wszParam) == 0)
	{
		return 512;
	}

	return CRhRdkPhysicalSkyBase<CRhRdkTexture>::GetParameter(wszParam);
}
