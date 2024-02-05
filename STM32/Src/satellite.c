#include "satellite.h"
#include <stdlib.h>

SAT_Instance SATTELITE = {0};

SRAM4 char SAT_TLE_NAMES[SAT_TLE_MAXCOUNT][SAT_NAME_MAXLEN + 1] = {0};

static void SAT_SGP4(SAT_Instance *sat, float64_t tsince);
static void SAT_SDP4(SAT_Instance *sat, float64_t tsince);
static float64_t SAT_Degrees(float64_t arg);
static float64_t SAT_FMod2p(float64_t x);
static float64_t SAT_AcTan(float64_t sinx, float64_t cosx);
static void SAT_Deep(uint8_t ientry, SAT_Instance *sat);
static void SAT_Convert_Sat_State(SAT_Vector *pos, SAT_Vector *vel);
static void SAT_Scale_Vector(float64_t k, SAT_Vector *v);
static float64_t SAT_ThetaG(float64_t epoch, SAT_DeepArg *deep_arg);
static float64_t SAT_Julian_Date_of_Year(uint32_t year);
static float64_t SAT_Modulus(float64_t arg1, float64_t arg2);
static float64_t SAT_Dot(SAT_Vector *v1, SAT_Vector *v2);
static float64_t SAT_ArcSin(float64_t arg);
static int8_t SAT_Sign(float64_t arg);
static void SAT_Calculate_User_PosVel(float64_t _time, SAT_Geodetic *geodetic, SAT_Vector *obs_pos, SAT_Vector *obs_vel);
static void SAT_Calculate_Obs(float64_t _time, SAT_Vector *pos, SAT_Vector *vel, SAT_Geodetic *geodetic, SAT_ObsSet *obs_set);
static void SAT_Calculate_LatLonAlt(float64_t _time, SAT_Vector *pos, SAT_Geodetic *geodetic);
static float64_t SAT_ThetaG_JD(float64_t jd);
static float64_t SAT_Frac(float64_t arg);
static float64_t SAT_Julian_Date_of_Epoch(float64_t epoch);

void SAT_init() {
	char tmp[16];
	char tmp2[16];
	char tmp3[16];

	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line1 + 2, 5);
	SATTELITE.catnr = atol(tmp);

	memset(tmp, 0, sizeof(tmp));
	strncpy(SATTELITE.idesg, TRX.SAT_TLE_Line1 + 9, 8);

	// Replace spaces with 0 before casting, as leading spaces are allowed
	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line1 + 18, 14);
	for (uint8_t i = 0; i < 14; i++) {
		if (tmp[i] == ' ') {
			tmp[i] = '0';
		}
	}
	SATTELITE.epoch = atof(tmp);

	// Now, convert the epoch time into year, day and fraction of day, according to YYDDD.FFFFFFFF
	// Adjust for 2 digit year through 2056
	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line1 + 18, 2);
	SATTELITE.epoch_year = atol(tmp);
	if (SATTELITE.epoch_year > 56) {
		SATTELITE.epoch_year = SATTELITE.epoch_year + 1900;
	} else {
		SATTELITE.epoch_year = SATTELITE.epoch_year + 2000;
	}

	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line1 + 20, 3);
	SATTELITE.epoch_day = atol(tmp);

	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line1 + 23, 9);
	SATTELITE.epoch_fod = atof(tmp);

	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line1 + 33, 10);
	SATTELITE.xndt2o = atof(tmp);

	memset(tmp, 0, sizeof(tmp));
	memset(tmp2, 0, sizeof(tmp));
	memset(tmp3, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line1 + 44, 1);
	strcat(tmp, ".");
	strncpy(tmp2, TRX.SAT_TLE_Line1 + 45, 5);
	strcat(tmp, tmp2);
	strcat(tmp, "E");
	strncpy(tmp3, TRX.SAT_TLE_Line1 + 50, 2);
	strcat(tmp, tmp3);
	SATTELITE.xndd6o = atof(tmp);

	memset(tmp, 0, sizeof(tmp));
	memset(tmp2, 0, sizeof(tmp));
	memset(tmp3, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line1 + 53, 1);
	strcat(tmp, ".");
	strncpy(tmp2, TRX.SAT_TLE_Line1 + 54, 5);
	strcat(tmp, tmp2);
	strcat(tmp, "E");
	strncpy(tmp3, TRX.SAT_TLE_Line1 + 59, 2);
	strcat(tmp, tmp3);
	SATTELITE.bstar = atof(tmp);

	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line1 + 64, 4);
	SATTELITE.elset = atol(tmp);

	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line2 + 8, 8);
	SATTELITE.xincl = atof(tmp);

	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line2 + 17, 8);
	SATTELITE.xnodeo = atof(tmp);

	memset(tmp, 0, sizeof(tmp));
	strcpy(tmp, "0.");
	strncpy(tmp2, TRX.SAT_TLE_Line2 + 26, 7);
	strcat(tmp, tmp2);
	SATTELITE.eo = atof(tmp);

	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line2 + 34, 8);
	SATTELITE.omegao = atof(tmp);

	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line2 + 43, 8);
	SATTELITE.xmo = atof(tmp);

	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line2 + 52, 11);
	SATTELITE.xno = atof(tmp);

	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, TRX.SAT_TLE_Line2 + 63, 5);
	SATTELITE.revnum = atol(tmp);

	// Process ephemeris data

	// Preprocess tle set
	SATTELITE.xnodeo *= SAT_de2ra;
	SATTELITE.omegao *= SAT_de2ra;
	SATTELITE.xmo *= SAT_de2ra;
	SATTELITE.xincl *= SAT_de2ra;
	float64_t temp = SAT_twopi / SAT_xmnpda / SAT_xmnpda;

	/* store mean motion before conversion */
	SATTELITE.meanmo = SATTELITE.xno;
	SATTELITE.xno = SATTELITE.xno * temp * SAT_xmnpda;
	SATTELITE.xndt2o *= temp;
	SATTELITE.xndd6o = SATTELITE.xndd6o * temp / SAT_xmnpda;
	SATTELITE.bstar /= SAT_ae;

	/* Period > 225 minutes is deep space */
	float64_t dd1 = SAT_xke / SATTELITE.xno;
	float64_t dd2 = SAT_tothrd;
	float64_t a1 = pow(dd1, dd2);
	float64_t r1 = cos(SATTELITE.xincl);
	dd1 = 1.0 - SATTELITE.eo * SATTELITE.eo;
	temp = SAT_ck2 * 1.5 * (r1 * r1 * 3.0 - 1.0) / pow(dd1, 1.5);
	float64_t del1 = temp / (a1 * a1);
	float64_t ao = a1 * (1.0 - del1 * (SAT_tothrd * 0.5 + del1 * (del1 * 1.654320987654321 + 1.0)));
	float64_t delo = temp / (ao * ao);
	float64_t xnodp = SATTELITE.xno / (delo + 1.0);

	/* Select a deep-space/near-earth ephemeris */
	if (SAT_twopi / xnodp / SAT_xmnpda >= .15625) {
		SATTELITE.flags |= SAT_DEEP_SPACE_EPHEM_FLAG;
	} else {
		SATTELITE.flags &= ~SAT_DEEP_SPACE_EPHEM_FLAG;
	}

	// Sattelite data
	SATTELITE.jul_epoch = SAT_Julian_Date_of_Epoch(SATTELITE.epoch);

	// Calc data for current timestamp
	SAT_calc(SAT_unix2daynum(getUTCTimestamp(), 0));
}

void SAT_calc(float64_t t) {
	SAT_ObsSet obs_set;
	SAT_Geodetic sat_geodetic;
	SAT_Geodetic obs_geodetic;
	obs_geodetic.lon = atof(TRX.SAT_QTH_Lon) * SAT_de2ra;
	obs_geodetic.lat = atof(TRX.SAT_QTH_Lat) * SAT_de2ra;
	obs_geodetic.alt = atof(TRX.SAT_QTH_Alt) / 1000.0;
	obs_geodetic.theta = 0;

	SATTELITE.jul_utc = t;
	SATTELITE.tsince = (SATTELITE.jul_utc - SATTELITE.jul_epoch) * SAT_xmnpda;

	/* call the norad routines according to the deep-space flag */
	if (SATTELITE.flags & SAT_DEEP_SPACE_EPHEM_FLAG) {
		SAT_SDP4(&SATTELITE, SATTELITE.tsince);
	} else {
		SAT_SGP4(&SATTELITE, SATTELITE.tsince);
	}

	SAT_Convert_Sat_State(&SATTELITE.pos, &SATTELITE.vel);

	/* get the velocity of the satellite */
	SATTELITE.vel.w = sqrt(SATTELITE.vel.x * SATTELITE.vel.x + SATTELITE.vel.y * SATTELITE.vel.y + SATTELITE.vel.z * SATTELITE.vel.z);
	SATTELITE.velo = SATTELITE.vel.w;
	SAT_Calculate_Obs(SATTELITE.jul_utc, &SATTELITE.pos, &SATTELITE.vel, &obs_geodetic, &obs_set);
	SAT_Calculate_LatLonAlt(SATTELITE.jul_utc, &SATTELITE.pos, &sat_geodetic);

	while (sat_geodetic.lon < -SAT_pi) {
		sat_geodetic.lon += SAT_twopi;
	}

	while (sat_geodetic.lon > SAT_pi) {
		sat_geodetic.lon -= SAT_twopi;
	}

	SATTELITE.az = SAT_Degrees(obs_set.az);
	SATTELITE.el = SAT_Degrees(obs_set.el);
	SATTELITE.range = obs_set.range;
	SATTELITE.range_rate = obs_set.range_rate;
	SATTELITE.ssplat = SAT_Degrees(sat_geodetic.lat);
	SATTELITE.ssplon = SAT_Degrees(sat_geodetic.lon);
	SATTELITE.alt = sat_geodetic.alt;
	SATTELITE.ma = SAT_Degrees(SATTELITE.phase);
	SATTELITE.ma *= 256.0 / 360.0;
	SATTELITE.phase = SAT_Degrees(SATTELITE.phase);

	/* same formulas, but the one from predict is nicer */
	SATTELITE.footprint = 12756.33 * acos(SAT_xkmper / (SAT_xkmper + SATTELITE.alt));
	float64_t age = SATTELITE.jul_utc - SATTELITE.jul_epoch;
	SATTELITE.orbit = floor((SATTELITE.xno * SAT_xmnpda / SAT_twopi + age * SATTELITE.bstar * SAT_ae) * age + SATTELITE.xmo / SAT_twopi) + SATTELITE.revnum - 1;

	// some output
	println("SAT currept pos LAT: ", SATTELITE.ssplat, " LON: ", SATTELITE.ssplon, " ALT: ", SATTELITE.alt);
}

/* SGP4 */
/* This function is used to calculate the position and velocity */
/* of near-earth (period < 225 minutes) satellites. tsince is   */
/* time since epoch in minutes, tle is a pointer to a tle_t     */
/* structure with Keplerian orbital elements and pos and vel    */
/* are vector_t structures returning ECI satellite position and */
/* velocity. Use Convert_Sat_State() to convert to km and km/s.*/
static void SAT_SGP4(SAT_Instance *sat, float64_t tsince) {
	/* Initialization */
	if (~sat->flags & SAT_SGP4_INITIALIZED_FLAG) {
		sat->flags |= SAT_SGP4_INITIALIZED_FLAG;

		/* Recover original mean motion (xnodp) and   */
		/* semimajor axis (aodp) from input elements. */
		float64_t a1 = pow(SAT_xke / sat->xno, SAT_tothrd);
		sat->sgps.cosio = cos(sat->xincl);
		float64_t theta2 = sat->sgps.cosio * sat->sgps.cosio;
		sat->sgps.x3thm1 = 3 * theta2 - 1.0;
		float64_t eosq = sat->eo * sat->eo;
		float64_t betao2 = 1.0 - eosq;
		float64_t betao = sqrt(betao2);
		float64_t del1 = 1.5 * SAT_ck2 * sat->sgps.x3thm1 / (a1 * a1 * betao * betao2);
		float64_t ao = a1 * (1 - del1 * (0.5 * SAT_tothrd + del1 * (1 + 134.0 / 81.0 * del1)));
		float64_t delo = 1.5 * SAT_ck2 * sat->sgps.x3thm1 / (ao * ao * betao * betao2);
		sat->sgps.xnodp = sat->xno / (1.0 + delo);
		sat->sgps.aodp = ao / (1.0 - delo);

		/* For perigee less than 220 kilometers, the "simple" flag is set */
		/* and the equations are truncated to linear variation in sqrt a  */
		/* and quadratic variation in mean anomaly.  Also, the c3 term,   */
		/* the delta omega term, and the delta m term are dropped.        */
		if ((sat->sgps.aodp * (1.0 - sat->eo) / SAT_ae) < (220.0 / SAT_xkmper + SAT_ae)) {
			sat->flags |= SAT_SIMPLE_FLAG;
		} else {
			sat->flags &= ~SAT_SIMPLE_FLAG;
		}

		/* For perigee below 156 km, the       */
		/* values of s and qoms2t are altered. */
		float64_t s4 = SAT___s__;
		float64_t qoms24 = SAT_qoms2t;
		float64_t perige = (sat->sgps.aodp * (1 - sat->eo) - SAT_ae) * SAT_xkmper;
		if (perige < 156.0) {
			if (perige <= 98.0) {
				s4 = 20.0;
			} else {
				s4 = perige - 78.0;
			}
			qoms24 = pow((120.0 - s4) * SAT_ae / SAT_xkmper, 4);
			s4 = s4 / SAT_xkmper + SAT_ae;
		}; /* FIXME FIXME: End of if(perige <= 98) NO WAY!!!! */

		float64_t pinvsq = 1.0 / (sat->sgps.aodp * sat->sgps.aodp * betao2 * betao2);
		float64_t tsi = 1.0 / (sat->sgps.aodp - s4);
		sat->sgps.eta = sat->sgps.aodp * sat->eo * tsi;
		float64_t etasq = sat->sgps.eta * sat->sgps.eta;
		float64_t eeta = sat->eo * sat->sgps.eta;
		float64_t psisq = fabs(1.0 - etasq);
		float64_t coef = qoms24 * pow(tsi, 4);
		float64_t coef1 = coef / pow(psisq, 3.5);
		float64_t c2 =
		    coef1 * sat->sgps.xnodp * (sat->sgps.aodp * (1.0 + 1.5 * etasq + eeta * (4.0 + etasq)) + 0.75 * SAT_ck2 * tsi / psisq * sat->sgps.x3thm1 * (8.0 + 3.0 * etasq * (8 + etasq)));
		sat->sgps.c1 = c2 * sat->bstar;
		sat->sgps.sinio = sin(sat->xincl);
		float64_t a3ovk2 = -SAT_xj3 / SAT_ck2 * pow(SAT_ae, 3);
		float64_t c3 = coef * tsi * a3ovk2 * sat->sgps.xnodp * SAT_ae * sat->sgps.sinio / sat->eo;
		sat->sgps.x1mth2 = 1.0 - theta2;
		sat->sgps.c4 =
		    2.0 * sat->sgps.xnodp * coef1 * sat->sgps.aodp * betao2 *
		    (sat->sgps.eta * (2.0 + 0.5 * etasq) + sat->eo * (0.5 + 2.0 * etasq) -
		     2.0 * SAT_ck2 * tsi / (sat->sgps.aodp * psisq) *
		         (-3.0 * sat->sgps.x3thm1 * (1.0 - 2.0 * eeta + etasq * (1.5 - 0.5 * eeta)) + 0.75 * sat->sgps.x1mth2 * (2.0 * etasq - eeta * (1.0 + etasq)) * cos(2.0 * sat->omegao)));
		sat->sgps.c5 = 2.0 * coef1 * sat->sgps.aodp * betao2 * (1.0 + 2.75 * (etasq + eeta) + eeta * etasq);
		float64_t theta4 = theta2 * theta2;
		float64_t temp1 = 3.0 * SAT_ck2 * pinvsq * sat->sgps.xnodp;
		float64_t temp2 = temp1 * SAT_ck2 * pinvsq;
		float64_t temp3 = 1.25 * SAT_ck4 * pinvsq * pinvsq * sat->sgps.xnodp;
		sat->sgps.xmdot = sat->sgps.xnodp + 0.5 * temp1 * betao * sat->sgps.x3thm1 + 0.0625 * temp2 * betao * (13.0 - 78.0 * theta2 + 137.0 * theta4);
		float64_t x1m5th = 1.0 - 5.0 * theta2;
		sat->sgps.omgdot = -0.5 * temp1 * x1m5th + 0.0625 * temp2 * (7.0 - 114.0 * theta2 + 395.0 * theta4) + temp3 * (3.0 - 36.0 * theta2 + 49.0 * theta4);
		float64_t xhdot1 = -temp1 * sat->sgps.cosio;
		sat->sgps.xnodot = xhdot1 + (0.5 * temp2 * (4.0 - 19.0 * theta2) + 2.0 * temp3 * (3.0 - 7.0 * theta2)) * sat->sgps.cosio;
		sat->sgps.omgcof = sat->bstar * c3 * cos(sat->omegao);
		sat->sgps.xmcof = -SAT_tothrd * coef * sat->bstar * SAT_ae / eeta;
		sat->sgps.xnodcf = 3.5 * betao2 * xhdot1 * sat->sgps.c1;
		sat->sgps.t2cof = 1.5 * sat->sgps.c1;
		sat->sgps.xlcof = 0.125 * a3ovk2 * sat->sgps.sinio * (3.0 + 5.0 * sat->sgps.cosio) / (1.0 + sat->sgps.cosio);
		sat->sgps.aycof = 0.25 * a3ovk2 * sat->sgps.sinio;
		sat->sgps.delmo = pow(1.0 + sat->sgps.eta * cos(sat->xmo), 3);
		sat->sgps.sinmo = sin(sat->xmo);
		sat->sgps.x7thm1 = 7.0 * theta2 - 1.0;
		if (~sat->flags & SAT_SIMPLE_FLAG) {
			float64_t c1sq = sat->sgps.c1 * sat->sgps.c1;
			sat->sgps.d2 = 4.0 * sat->sgps.aodp * tsi * c1sq;
			float64_t temp = sat->sgps.d2 * tsi * sat->sgps.c1 / 3.0;
			sat->sgps.d3 = (17.0 * sat->sgps.aodp + s4) * temp;
			sat->sgps.d4 = 0.5 * temp * sat->sgps.aodp * tsi * (221.0 * sat->sgps.aodp + 31.0 * s4) * sat->sgps.c1;
			sat->sgps.t3cof = sat->sgps.d2 + 2.0 * c1sq;
			sat->sgps.t4cof = 0.25 * (3.0 * sat->sgps.d3 + sat->sgps.c1 * (12.0 * sat->sgps.d2 + 10.0 * c1sq));
			sat->sgps.t5cof = 0.2 * (3.0 * sat->sgps.d4 + 12.0 * sat->sgps.c1 * sat->sgps.d3 + 6.0 * sat->sgps.d2 * sat->sgps.d2 + 15.0 * c1sq * (2.0 * sat->sgps.d2 + c1sq));
		}; /* End of if (isFlagClear(SIMPLE_FLAG)) */
	};   /* End of SGP4() initialization */

	/* Update for secular gravity and atmospheric drag. */
	float64_t xmdf = sat->xmo + sat->sgps.xmdot * tsince;
	float64_t omgadf = sat->omegao + sat->sgps.omgdot * tsince;
	float64_t xnoddf = sat->xnodeo + sat->sgps.xnodot * tsince;
	float64_t omega = omgadf;
	float64_t xmp = xmdf;
	float64_t tsq = tsince * tsince;
	float64_t xnode = xnoddf + sat->sgps.xnodcf * tsq;
	float64_t tempa = 1.0 - sat->sgps.c1 * tsince;
	float64_t tempe = sat->bstar * sat->sgps.c4 * tsince;
	float64_t templ = sat->sgps.t2cof * tsq;
	if (~sat->flags & SAT_SIMPLE_FLAG) {
		float64_t delomg = sat->sgps.omgcof * tsince;
		float64_t delm = sat->sgps.xmcof * (pow(1 + sat->sgps.eta * cos(xmdf), 3) - sat->sgps.delmo);
		float64_t temp = delomg + delm;
		xmp = xmdf + temp;
		omega = omgadf - temp;
		float64_t tcube = tsq * tsince;
		float64_t tfour = tsince * tcube;
		tempa = tempa - sat->sgps.d2 * tsq - sat->sgps.d3 * tcube - sat->sgps.d4 * tfour;
		tempe = tempe + sat->bstar * sat->sgps.c5 * (sin(xmp) - sat->sgps.sinmo);
		templ = templ + sat->sgps.t3cof * tcube + tfour * (sat->sgps.t4cof + tsince * sat->sgps.t5cof);
	}; /* End of if (isFlagClear(SIMPLE_FLAG)) */

	float64_t a = sat->sgps.aodp * pow(tempa, 2);
	float64_t e = sat->eo - tempe;
	float64_t xl = xmp + omega + xnode + sat->sgps.xnodp * templ;
	float64_t beta = sqrt(1.0 - (e * e));
	float64_t xn = SAT_xke / pow(a, 1.5);

	/* Long period periodics */
	float64_t axn = e * cos(omega);
	float64_t temp = 1.0 / (a * beta * beta);
	float64_t xll = temp * sat->sgps.xlcof * axn;
	float64_t aynl = temp * sat->sgps.aycof;
	float64_t xlt = xl + xll;
	float64_t ayn = e * sin(omega) + aynl;

	/* Solve Kepler's' Equation */
	float64_t capu = SAT_FMod2p(xlt - xnode);
	float64_t temp2 = capu;

	uint8_t i = 0;
	float64_t sinepw;
	float64_t cosepw;
	float64_t temp3;
	float64_t temp4;
	float64_t temp5;
	float64_t temp6;
	do {
		sinepw = sin(temp2);
		cosepw = cos(temp2);
		temp3 = axn * sinepw;
		temp4 = ayn * cosepw;
		temp5 = axn * cosepw;
		temp6 = ayn * sinepw;
		float64_t epw = (capu - temp4 + temp3 - temp2) / (1.0 - temp5 - temp6) + temp2;
		if (fabs(epw - temp2) <= SAT_e6a) {
			break;
		}
		temp2 = epw;
	} while (i++ < 10);

	/* Short period preliminary quantities */
	float64_t ecose = temp5 + temp6;
	float64_t esine = temp3 - temp4;
	float64_t elsq = axn * axn + ayn * ayn;
	temp = 1.0 - elsq;
	float64_t pl = a * temp;
	float64_t r = a * (1.0 - ecose);
	float64_t temp1 = 1.0 / r;
	float64_t rdot = SAT_xke * sqrt(a) * esine * temp1;
	float64_t rfdot = SAT_xke * sqrt(pl) * temp1;
	temp2 = a * temp1;
	float64_t betal = sqrt(temp);
	temp3 = 1.0 / (1.0 + betal);
	float64_t cosu = temp2 * (cosepw - axn + ayn * esine * temp3);
	float64_t sinu = temp2 * (sinepw - ayn - axn * esine * temp3);
	float64_t u = SAT_AcTan(sinu, cosu);
	float64_t sin2u = 2.0 * sinu * cosu;
	float64_t cos2u = 2.0 * cosu * cosu - 1.0;
	temp = 1.0 / pl;
	temp1 = SAT_ck2 * temp;
	temp2 = temp1 * temp;

	/* Update for short periodics */
	float64_t rk = r * (1.0 - 1.5 * temp2 * betal * sat->sgps.x3thm1) + 0.5 * temp1 * sat->sgps.x1mth2 * cos2u;
	float64_t uk = u - 0.25 * temp2 * sat->sgps.x7thm1 * sin2u;
	float64_t xnodek = xnode + 1.5 * temp2 * sat->sgps.cosio * sin2u;
	float64_t xinck = sat->xincl + 1.5 * temp2 * sat->sgps.cosio * sat->sgps.sinio * cos2u;
	float64_t rdotk = rdot - xn * temp1 * sat->sgps.x1mth2 * sin2u;
	float64_t rfdotk = rfdot + xn * temp1 * (sat->sgps.x1mth2 * cos2u + 1.5 * sat->sgps.x3thm1);

	/* Orientation vectors */
	float64_t sinuk = sin(uk);
	float64_t cosuk = cos(uk);
	float64_t sinik = sin(xinck);
	float64_t cosik = cos(xinck);
	float64_t sinnok = sin(xnodek);
	float64_t cosnok = cos(xnodek);
	float64_t xmx = -sinnok * cosik;
	float64_t xmy = cosnok * cosik;
	float64_t ux = xmx * sinuk + cosnok * cosuk;
	float64_t uy = xmy * sinuk + sinnok * cosuk;
	float64_t uz = sinik * sinuk;
	float64_t vx = xmx * cosuk - cosnok * sinuk;
	float64_t vy = xmy * cosuk - sinnok * sinuk;
	float64_t vz = sinik * cosuk;

	/* Position and velocity */
	sat->pos.x = rk * ux;
	sat->pos.y = rk * uy;
	sat->pos.z = rk * uz;
	sat->vel.x = rdotk * ux + rfdotk * vx;
	sat->vel.y = rdotk * uy + rfdotk * vy;
	sat->vel.z = rdotk * uz + rfdotk * vz;

	sat->phase = xlt - xnode - omgadf + SAT_twopi;
	if (sat->phase < 0) {
		sat->phase += SAT_twopi;
	}
	sat->phase = SAT_FMod2p(sat->phase);

	sat->omegao1 = omega;
	sat->xincl1 = xinck;
	sat->xnodeo1 = xnodek;

} /*SGP4*/

/* SDP4 */
/* This function is used to calculate the position and velocity */
/* of deep-space (period > 225 minutes) satellites. tsince is   */
/* time since epoch in minutes, tle is a pointer to a tle_t     */
/* structure with Keplerian orbital elements and pos and vel    */
/* are vector_t structures returning ECI satellite position and */
/* velocity. Use Convert_Sat_State() to convert to km and km/s. */
static void SAT_SDP4(SAT_Instance *sat, float64_t tsince) {
	/* Initialization */
	if (~sat->flags & SAT_SDP4_INITIALIZED_FLAG) {

		sat->flags |= SAT_SDP4_INITIALIZED_FLAG;

		/* Recover original mean motion (xnodp) and   */
		/* semimajor axis (aodp) from input elements. */
		float64_t a1 = pow(SAT_xke / sat->xno, SAT_tothrd);
		sat->deep_arg.cosio = cos(sat->xincl);
		sat->deep_arg.theta2 = sat->deep_arg.cosio * sat->deep_arg.cosio;
		sat->sgps.x3thm1 = 3.0 * sat->deep_arg.theta2 - 1.0;
		sat->deep_arg.eosq = sat->eo * sat->eo;
		sat->deep_arg.betao2 = 1.0 - sat->deep_arg.eosq;
		sat->deep_arg.betao = sqrt(sat->deep_arg.betao2);
		float64_t del1 = 1.5 * SAT_ck2 * sat->sgps.x3thm1 / (a1 * a1 * sat->deep_arg.betao * sat->deep_arg.betao2);
		float64_t ao = a1 * (1.0 - del1 * (0.5 * SAT_tothrd + del1 * (1.0 + 134.0 / 81.0 * del1)));
		float64_t delo = 1.5 * SAT_ck2 * sat->sgps.x3thm1 / (ao * ao * sat->deep_arg.betao * sat->deep_arg.betao2);
		sat->deep_arg.xnodp = sat->xno / (1.0 + delo);
		sat->deep_arg.aodp = ao / (1.0 - delo);

		/* For perigee below 156 km, the values */
		/* of s and qoms2t are altered.         */
		float64_t s4 = SAT___s__;
		float64_t qoms24 = SAT_qoms2t;
		float64_t perige = (sat->deep_arg.aodp * (1.0 - sat->eo) - SAT_ae) * SAT_xkmper;
		if (perige < 156.0) {
			if (perige <= 98.0) {
				s4 = 20.0;
			} else {
				s4 = perige - 78.0;
			}
			qoms24 = pow((120.0 - s4) * SAT_ae / SAT_xkmper, 4);
			s4 = s4 / SAT_xkmper + SAT_ae;
		}
		float64_t pinvsq = 1.0 / (sat->deep_arg.aodp * sat->deep_arg.aodp * sat->deep_arg.betao2 * sat->deep_arg.betao2);
		sat->deep_arg.sing = sin(sat->omegao);
		sat->deep_arg.cosg = cos(sat->omegao);
		float64_t tsi = 1.0 / (sat->deep_arg.aodp - s4);
		float64_t eta = sat->deep_arg.aodp * sat->eo * tsi;
		float64_t etasq = eta * eta;
		float64_t eeta = sat->eo * eta;
		float64_t psisq = fabs(1.0 - etasq);
		float64_t coef = qoms24 * pow(tsi, 4);
		float64_t coef1 = coef / pow(psisq, 3.5);
		float64_t c2 = coef1 * sat->deep_arg.xnodp *
		               (sat->deep_arg.aodp * (1.0 + 1.5 * etasq + eeta * (4.0 + etasq)) + 0.75 * SAT_ck2 * tsi / psisq * sat->sgps.x3thm1 * (8.0 + 3.0 * etasq * (8.0 + etasq)));
		sat->sgps.c1 = sat->bstar * c2;
		sat->deep_arg.sinio = sin(sat->xincl);
		float64_t a3ovk2 = -SAT_xj3 / SAT_ck2 * pow(SAT_ae, 3);
		sat->sgps.x1mth2 = 1.0 - sat->deep_arg.theta2;
		sat->sgps.c4 =
		    2.0 * sat->deep_arg.xnodp * coef1 * sat->deep_arg.aodp * sat->deep_arg.betao2 *
		    (eta * (2.0 + 0.5 * etasq) + sat->eo * (0.5 + 2.0 * etasq) -
		     2.0 * SAT_ck2 * tsi / (sat->deep_arg.aodp * psisq) *
		         (-3.0 * sat->sgps.x3thm1 * (1.0 - 2.0 * eeta + etasq * (1.5 - 0.5 * eeta)) + 0.75 * sat->sgps.x1mth2 * (2.0 * etasq - eeta * (1.0 + etasq)) * cos(2.0 * sat->omegao)));
		float64_t theta4 = sat->deep_arg.theta2 * sat->deep_arg.theta2;
		float64_t temp1 = 3.0 * SAT_ck2 * pinvsq * sat->deep_arg.xnodp;
		float64_t temp2 = temp1 * SAT_ck2 * pinvsq;
		float64_t temp3 = 1.25 * SAT_ck4 * pinvsq * pinvsq * sat->deep_arg.xnodp;
		sat->deep_arg.xmdot =
		    sat->deep_arg.xnodp + 0.5 * temp1 * sat->deep_arg.betao * sat->sgps.x3thm1 + 0.0625 * temp2 * sat->deep_arg.betao * (13.0 - 78.0 * sat->deep_arg.theta2 + 137.0 * theta4);
		float64_t x1m5th = 1.0 - 5.0 * sat->deep_arg.theta2;
		sat->deep_arg.omgdot = -0.5 * temp1 * x1m5th + 0.0625 * temp2 * (7.0 - 114.0 * sat->deep_arg.theta2 + 395.0 * theta4) + temp3 * (3.0 - 36.0 * sat->deep_arg.theta2 + 49.0 * theta4);
		float64_t xhdot1 = -temp1 * sat->deep_arg.cosio;
		sat->deep_arg.xnodot = xhdot1 + (0.5 * temp2 * (4.0 - 19.0 * sat->deep_arg.theta2) + 2.0 * temp3 * (3.0 - 7.0 * sat->deep_arg.theta2)) * sat->deep_arg.cosio;
		sat->sgps.xnodcf = 3.5 * sat->deep_arg.betao2 * xhdot1 * sat->sgps.c1;
		sat->sgps.t2cof = 1.5 * sat->sgps.c1;
		sat->sgps.xlcof = 0.125 * a3ovk2 * sat->deep_arg.sinio * (3.0 + 5.0 * sat->deep_arg.cosio) / (1.0 + sat->deep_arg.cosio);
		sat->sgps.aycof = 0.25 * a3ovk2 * sat->deep_arg.sinio;
		sat->sgps.x7thm1 = 7.0 * sat->deep_arg.theta2 - 1.0;

		/* initialize Deep() */
		SAT_Deep(SAT_dpinit, sat);
	}; /*End of SDP4() initialization */

	/* Update for secular gravity and atmospheric drag */
	float64_t xmdf = sat->xmo + sat->deep_arg.xmdot * tsince;
	sat->deep_arg.omgadf = sat->omegao + sat->deep_arg.omgdot * tsince;
	float64_t xnoddf = sat->xnodeo + sat->deep_arg.xnodot * tsince;
	float64_t tsq = tsince * tsince;
	sat->deep_arg.xnode = xnoddf + sat->sgps.xnodcf * tsq;
	float64_t tempa = 1.0 - sat->sgps.c1 * tsince;
	float64_t tempe = sat->bstar * sat->sgps.c4 * tsince;
	float64_t templ = sat->sgps.t2cof * tsq;
	sat->deep_arg.xn = sat->deep_arg.xnodp;

	/* Update for deep-space secular effects */
	sat->deep_arg.xll = xmdf;
	sat->deep_arg.t = tsince;

	SAT_Deep(SAT_dpsec, sat);

	xmdf = sat->deep_arg.xll;
	float64_t a = pow(SAT_xke / sat->deep_arg.xn, SAT_tothrd) * tempa * tempa;
	sat->deep_arg.em = sat->deep_arg.em - tempe;
	float64_t xmam = xmdf + sat->deep_arg.xnodp * templ;

	/* Update for deep-space periodic effects */
	sat->deep_arg.xll = xmam;

	SAT_Deep(SAT_dpper, sat);

	xmam = sat->deep_arg.xll;
	float64_t xl = xmam + sat->deep_arg.omgadf + sat->deep_arg.xnode;
	float64_t beta = sqrt(1.0 - sat->deep_arg.em * sat->deep_arg.em);
	sat->deep_arg.xn = SAT_xke / pow(a, 1.5);

	/* Long period periodics */
	float64_t axn = sat->deep_arg.em * cos(sat->deep_arg.omgadf);
	float64_t temp = 1.0 / (a * beta * beta);
	float64_t xll = temp * sat->sgps.xlcof * axn;
	float64_t aynl = temp * sat->sgps.aycof;
	float64_t xlt = xl + xll;
	float64_t ayn = sat->deep_arg.em * sin(sat->deep_arg.omgadf) + aynl;

	/* Solve Kepler's Equation */
	float64_t capu = SAT_FMod2p(xlt - sat->deep_arg.xnode);
	float64_t temp2 = capu;

	uint8_t i = 0;
	float64_t sinepw;
	float64_t cosepw;
	float64_t temp3;
	float64_t temp4;
	float64_t temp5;
	float64_t temp6;
	do {
		sinepw = sin(temp2);
		cosepw = cos(temp2);
		temp3 = axn * sinepw;
		temp4 = ayn * cosepw;
		temp5 = axn * cosepw;
		temp6 = ayn * sinepw;
		float64_t epw = (capu - temp4 + temp3 - temp2) / (1.0 - temp5 - temp6) + temp2;
		if (fabs(epw - temp2) <= SAT_e6a) {
			break;
		}
		temp2 = epw;
	} while (i++ < 10);

	/* Short period preliminary quantities */
	float64_t ecose = temp5 + temp6;
	float64_t esine = temp3 - temp4;
	float64_t elsq = axn * axn + ayn * ayn;
	temp = 1.0 - elsq;
	float64_t pl = a * temp;
	float64_t r = a * (1.0 - ecose);
	float64_t temp1 = 1.0 / r;
	float64_t rdot = SAT_xke * sqrt(a) * esine * temp1;
	float64_t rfdot = SAT_xke * sqrt(pl) * temp1;
	temp2 = a * temp1;
	float64_t betal = sqrt(temp);
	temp3 = 1.0 / (1.0 + betal);
	float64_t cosu = temp2 * (cosepw - axn + ayn * esine * temp3);
	float64_t sinu = temp2 * (sinepw - ayn - axn * esine * temp3);
	float64_t u = SAT_AcTan(sinu, cosu);
	float64_t sin2u = 2.0 * sinu * cosu;
	float64_t cos2u = 2.0 * cosu * cosu - 1.0;
	temp = 1.0 / pl;
	temp1 = SAT_ck2 * temp;
	temp2 = temp1 * temp;

	/* Update for short periodics */
	float64_t rk = r * (1.0 - 1.5 * temp2 * betal * sat->sgps.x3thm1) + 0.5 * temp1 * sat->sgps.x1mth2 * cos2u;
	float64_t uk = u - 0.25 * temp2 * sat->sgps.x7thm1 * sin2u;
	float64_t xnodek = sat->deep_arg.xnode + 1.5 * temp2 * sat->deep_arg.cosio * sin2u;
	float64_t xinck = sat->deep_arg.xinc + 1.5 * temp2 * sat->deep_arg.cosio * sat->deep_arg.sinio * cos2u;
	float64_t rdotk = rdot - sat->deep_arg.xn * temp1 * sat->sgps.x1mth2 * sin2u;
	float64_t rfdotk = rfdot + sat->deep_arg.xn * temp1 * (sat->sgps.x1mth2 * cos2u + 1.5 * sat->sgps.x3thm1);

	/* Orientation vectors */
	float64_t sinuk = sin(uk);
	float64_t cosuk = cos(uk);
	float64_t sinik = sin(xinck);
	float64_t cosik = cos(xinck);
	float64_t sinnok = sin(xnodek);
	float64_t cosnok = cos(xnodek);
	float64_t xmx = -sinnok * cosik;
	float64_t xmy = cosnok * cosik;
	float64_t ux = xmx * sinuk + cosnok * cosuk;
	float64_t uy = xmy * sinuk + sinnok * cosuk;
	float64_t uz = sinik * sinuk;
	float64_t vx = xmx * cosuk - cosnok * sinuk;
	float64_t vy = xmy * cosuk - sinnok * sinuk;
	float64_t vz = sinik * cosuk;

	/* Position and velocity */
	sat->pos.x = rk * ux;
	sat->pos.y = rk * uy;
	sat->pos.z = rk * uz;
	sat->vel.x = rdotk * ux + rfdotk * vx;
	sat->vel.y = rdotk * uy + rfdotk * vy;
	sat->vel.z = rdotk * uz + rfdotk * vz;

	/* Phase in rads */
	sat->phase = xlt - sat->deep_arg.xnode - sat->deep_arg.omgadf + SAT_twopi;
	if (sat->phase < 0.0) {
		sat->phase += SAT_twopi;
	}
	sat->phase = SAT_FMod2p(sat->phase);

	sat->omegao1 = sat->deep_arg.omgadf;
	sat->xincl1 = sat->deep_arg.xinc;
	sat->xnodeo1 = sat->deep_arg.xnode;
} /* SDP4 */

/* This function is used by SDP4 to add lunar and solar */
/* perturbation effects to deep-space orbit objects.    */
static void SAT_Deep(uint8_t ientry, SAT_Instance *sat) {
	switch (ientry) {
	case SAT_dpinit: /* Entrance for deep space initialization */
		sat->dps.thgr = SAT_ThetaG(sat->epoch, &sat->deep_arg);
		float64_t eq = sat->eo;
		sat->dps.xnq = sat->deep_arg.xnodp;
		float64_t aqnv = 1.0 / sat->deep_arg.aodp;
		sat->dps.xqncl = sat->xincl;
		float64_t xmao = sat->xmo;
		float64_t xpidot = sat->deep_arg.omgdot + sat->deep_arg.xnodot;
		float64_t sinq = sin(sat->xnodeo);
		float64_t cosq = cos(sat->xnodeo);
		sat->dps.omegaq = sat->omegao;
		sat->dps.preep = 0;

		/* Initialize lunar solar terms */
		float64_t day = sat->deep_arg.ds50 + 18261.5; /* Days since 1900 Jan 0.5 */
		if (day != sat->dps.preep) {
			sat->dps.preep = day;
			float64_t xnodce = 4.5236020 - 9.2422029E-4 * day;
			float64_t stem = sin(xnodce);
			float64_t ctem = cos(xnodce);
			sat->dps.zcosil = 0.91375164 - 0.03568096 * ctem;
			sat->dps.zsinil = sqrt(1.0 - sat->dps.zcosil * sat->dps.zcosil);
			sat->dps.zsinhl = 0.089683511 * stem / sat->dps.zsinil;
			sat->dps.zcoshl = sqrt(1.0 - sat->dps.zsinhl * sat->dps.zsinhl);
			float64_t c = 4.7199672 + 0.22997150 * day;
			float64_t gam = 5.8351514 + 0.0019443680 * day;
			sat->dps.zmol = SAT_FMod2p(c - gam);
			float64_t zx = 0.39785416 * stem / sat->dps.zsinil;
			float64_t zy = sat->dps.zcoshl * ctem + 0.91744867 * sat->dps.zsinhl * stem;
			zx = SAT_AcTan(zx, zy);
			zx = gam + zx - xnodce;
			sat->dps.zcosgl = cos(zx);
			sat->dps.zsingl = sin(zx);
			sat->dps.zmos = 6.2565837 + 0.017201977 * day;
			sat->dps.zmos = SAT_FMod2p(sat->dps.zmos);
		} /* End if(day != preep) */

		/* Do solar terms */
		sat->dps.savtsn = 1E20;
		float64_t zcosg = SAT_zcosgs;
		float64_t zsing = SAT_zsings;
		float64_t zcosi = SAT_zcosis;
		float64_t zsini = SAT_zsinis;
		float64_t zcosh = cosq;
		float64_t zsinh = sinq;
		float64_t cc = SAT_c1ss;
		float64_t zn = SAT_zns;
		float64_t ze = SAT_zes;
		float64_t zmo = sat->dps.zmos;
		float64_t xnoi = 1.0 / sat->dps.xnq;

		/* Loop breaks when Solar terms are done a second */
		/* time, after Lunar terms are initialized        */
		float64_t se;
		float64_t si;
		float64_t sl;
		float64_t sgh;
		float64_t sh;
		for (;;) {
			/* Solar terms done again after Lunar terms are done */
			float64_t a1 = zcosg * zcosh + zsing * zcosi * zsinh;
			float64_t a3 = -zsing * zcosh + zcosg * zcosi * zsinh;
			float64_t a7 = -zcosg * zsinh + zsing * zcosi * zcosh;
			float64_t a8 = zsing * zsini;
			float64_t a9 = zsing * zsinh + zcosg * zcosi * zcosh;
			float64_t a10 = zcosg * zsini;
			float64_t a2 = sat->deep_arg.cosio * a7 + sat->deep_arg.sinio * a8;
			float64_t a4 = sat->deep_arg.cosio * a9 + sat->deep_arg.sinio * a10;
			float64_t a5 = -sat->deep_arg.sinio * a7 + sat->deep_arg.cosio * a8;
			float64_t a6 = -sat->deep_arg.sinio * a9 + sat->deep_arg.cosio * a10;
			float64_t x1 = a1 * sat->deep_arg.cosg + a2 * sat->deep_arg.sing;
			float64_t x2 = a3 * sat->deep_arg.cosg + a4 * sat->deep_arg.sing;
			float64_t x3 = -a1 * sat->deep_arg.sing + a2 * sat->deep_arg.cosg;
			float64_t x4 = -a3 * sat->deep_arg.sing + a4 * sat->deep_arg.cosg;
			float64_t x5 = a5 * sat->deep_arg.sing;
			float64_t x6 = a6 * sat->deep_arg.sing;
			float64_t x7 = a5 * sat->deep_arg.cosg;
			float64_t x8 = a6 * sat->deep_arg.cosg;
			float64_t z31 = 12 * x1 * x1 - 3 * x3 * x3;
			float64_t z32 = 24 * x1 * x2 - 6 * x3 * x4;
			float64_t z33 = 12 * x2 * x2 - 3 * x4 * x4;
			float64_t z1 = 3 * (a1 * a1 + a2 * a2) + z31 * sat->deep_arg.eosq;
			float64_t z2 = 6 * (a1 * a3 + a2 * a4) + z32 * sat->deep_arg.eosq;
			float64_t z3 = 3 * (a3 * a3 + a4 * a4) + z33 * sat->deep_arg.eosq;
			float64_t z11 = -6 * a1 * a5 + sat->deep_arg.eosq * (-24 * x1 * x7 - 6 * x3 * x5);
			float64_t z12 = -6 * (a1 * a6 + a3 * a5) + sat->deep_arg.eosq * (-24 * (x2 * x7 + x1 * x8) - 6 * (x3 * x6 + x4 * x5));
			float64_t z13 = -6 * a3 * a6 + sat->deep_arg.eosq * (-24 * x2 * x8 - 6 * x4 * x6);
			float64_t z21 = 6 * a2 * a5 + sat->deep_arg.eosq * (24 * x1 * x5 - 6 * x3 * x7);
			float64_t z22 = 6 * (a4 * a5 + a2 * a6) + sat->deep_arg.eosq * (24 * (x2 * x5 + x1 * x6) - 6 * (x4 * x7 + x3 * x8));
			float64_t z23 = 6 * a4 * a6 + sat->deep_arg.eosq * (24 * x2 * x6 - 6 * x4 * x8);
			z1 = z1 + z1 + sat->deep_arg.betao2 * z31;
			z2 = z2 + z2 + sat->deep_arg.betao2 * z32;
			z3 = z3 + z3 + sat->deep_arg.betao2 * z33;
			float64_t s3 = cc * xnoi;
			float64_t s2 = -0.5 * s3 / sat->deep_arg.betao;
			float64_t s4 = s3 * sat->deep_arg.betao;
			float64_t s1 = -15 * eq * s4;
			float64_t s5 = x1 * x3 + x2 * x4;
			float64_t s6 = x2 * x3 + x1 * x4;
			float64_t s7 = x2 * x4 - x1 * x3;
			se = s1 * zn * s5;
			si = s2 * zn * (z11 + z13);
			sl = -zn * s3 * (z1 + z3 - 14 - 6 * sat->deep_arg.eosq);
			sgh = s4 * zn * (z31 + z33 - 6);
			sh = -zn * s2 * (z21 + z23);
			if (sat->dps.xqncl < 5.2359877E-2) {
				sh = 0;
			}
			sat->dps.ee2 = 2 * s1 * s6;
			sat->dps.e3 = 2 * s1 * s7;
			sat->dps.xi2 = 2 * s2 * z12;
			sat->dps.xi3 = 2 * s2 * (z13 - z11);
			sat->dps.xl2 = -2 * s3 * z2;
			sat->dps.xl3 = -2 * s3 * (z3 - z1);
			sat->dps.xl4 = -2 * s3 * (-21 - 9 * sat->deep_arg.eosq) * ze;
			sat->dps.xgh2 = 2 * s4 * z32;
			sat->dps.xgh3 = 2 * s4 * (z33 - z31);
			sat->dps.xgh4 = -18 * s4 * ze;
			sat->dps.xh2 = -2 * s2 * z22;
			sat->dps.xh3 = -2 * s2 * (z23 - z21);

			if (sat->flags & SAT_LUNAR_TERMS_DONE_FLAG) {
				break;
			}

			/* Do lunar terms */
			sat->dps.sse = se;
			sat->dps.ssi = si;
			sat->dps.ssl = sl;
			sat->dps.ssh = sh / sat->deep_arg.sinio;
			sat->dps.ssg = sgh - sat->deep_arg.cosio * sat->dps.ssh;
			sat->dps.se2 = sat->dps.ee2;
			sat->dps.si2 = sat->dps.xi2;
			sat->dps.sl2 = sat->dps.xl2;
			sat->dps.sgh2 = sat->dps.xgh2;
			sat->dps.sh2 = sat->dps.xh2;
			sat->dps.se3 = sat->dps.e3;
			sat->dps.si3 = sat->dps.xi3;
			sat->dps.sl3 = sat->dps.xl3;
			sat->dps.sgh3 = sat->dps.xgh3;
			sat->dps.sh3 = sat->dps.xh3;
			sat->dps.sl4 = sat->dps.xl4;
			sat->dps.sgh4 = sat->dps.xgh4;
			zcosg = sat->dps.zcosgl;
			zsing = sat->dps.zsingl;
			zcosi = sat->dps.zcosil;
			zsini = sat->dps.zsinil;
			zcosh = sat->dps.zcoshl * cosq + sat->dps.zsinhl * sinq;
			zsinh = sinq * sat->dps.zcoshl - cosq * sat->dps.zsinhl;
			zn = SAT_znl;
			cc = SAT_c1l;
			ze = SAT_zel;
			zmo = sat->dps.zmol;
			sat->flags |= SAT_LUNAR_TERMS_DONE_FLAG;
		} /* End of for(;;) */

		sat->dps.sse = sat->dps.sse + se;
		sat->dps.ssi = sat->dps.ssi + si;
		sat->dps.ssl = sat->dps.ssl + sl;
		sat->dps.ssg = sat->dps.ssg + sgh - sat->deep_arg.cosio / sat->deep_arg.sinio * sh;
		sat->dps.ssh = sat->dps.ssh + sh / sat->deep_arg.sinio;

		/* Geopotential resonance initialization for 12 hour orbits */
		sat->flags &= ~SAT_RESONANCE_FLAG;
		sat->flags &= ~SAT_SYNCHRONOUS_FLAG;

		float64_t bfact;
		if (!((sat->dps.xnq < 0.0052359877) && (sat->dps.xnq > 0.0034906585))) {
			if ((sat->dps.xnq < 0.00826) || (sat->dps.xnq > 0.00924)) {
				return;
			}
			if (eq < 0.5) {
				return;
			}
			sat->flags |= SAT_RESONANCE_FLAG;
			float64_t eoc = eq * sat->deep_arg.eosq;
			float64_t g201 = -0.306 - (eq - 0.64) * 0.440;
			float64_t g211;
			float64_t g310;
			float64_t g322;
			float64_t g410;
			float64_t g422;
			float64_t g520;
			if (eq <= 0.65) {
				g211 = 3.616 - 13.247 * eq + 16.290 * sat->deep_arg.eosq;
				g310 = -19.302 + 117.390 * eq - 228.419 * sat->deep_arg.eosq + 156.591 * eoc;
				g322 = -18.9068 + 109.7927 * eq - 214.6334 * sat->deep_arg.eosq + 146.5816 * eoc;
				g410 = -41.122 + 242.694 * eq - 471.094 * sat->deep_arg.eosq + 313.953 * eoc;
				g422 = -146.407 + 841.880 * eq - 1629.014 * sat->deep_arg.eosq + 1083.435 * eoc;
				g520 = -532.114 + 3017.977 * eq - 5740 * sat->deep_arg.eosq + 3708.276 * eoc;
			} else {
				g211 = -72.099 + 331.819 * eq - 508.738 * sat->deep_arg.eosq + 266.724 * eoc;
				g310 = -346.844 + 1582.851 * eq - 2415.925 * sat->deep_arg.eosq + 1246.113 * eoc;
				g322 = -342.585 + 1554.908 * eq - 2366.899 * sat->deep_arg.eosq + 1215.972 * eoc;
				g410 = -1052.797 + 4758.686 * eq - 7193.992 * sat->deep_arg.eosq + 3651.957 * eoc;
				g422 = -3581.69 + 16178.11 * eq - 24462.77 * sat->deep_arg.eosq + 12422.52 * eoc;
				if (eq <= 0.715) {
					g520 = 1464.74 - 4664.75 * eq + 3763.64 * sat->deep_arg.eosq;
				} else {
					g520 = -5149.66 + 29936.92 * eq - 54087.36 * sat->deep_arg.eosq + 31324.56 * eoc;
				}
			} /* End if (eq <= 0.65) */

			float64_t g533;
			float64_t g521;
			float64_t g532;
			if (eq < 0.7) {
				g533 = -919.2277 + 4988.61 * eq - 9064.77 * sat->deep_arg.eosq + 5542.21 * eoc;
				g521 = -822.71072 + 4568.6173 * eq - 8491.4146 * sat->deep_arg.eosq + 5337.524 * eoc;
				g532 = -853.666 + 4690.25 * eq - 8624.77 * sat->deep_arg.eosq + 5341.4 * eoc;
			} else {
				g533 = -37995.78 + 161616.52 * eq - 229838.2 * sat->deep_arg.eosq + 109377.94 * eoc;
				g521 = -51752.104 + 218913.95 * eq - 309468.16 * sat->deep_arg.eosq + 146349.42 * eoc;
				g532 = -40023.88 + 170470.89 * eq - 242699.48 * sat->deep_arg.eosq + 115605.82 * eoc;
			} /* End if (eq <= 0.7) */

			float64_t sini2 = sat->deep_arg.sinio * sat->deep_arg.sinio;
			float64_t f220 = 0.75 * (1 + 2 * sat->deep_arg.cosio + sat->deep_arg.theta2);
			float64_t f221 = 1.5 * sini2;
			float64_t f321 = 1.875 * sat->deep_arg.sinio * (1 - 2 * sat->deep_arg.cosio - 3 * sat->deep_arg.theta2);
			float64_t f322 = -1.875 * sat->deep_arg.sinio * (1 + 2 * sat->deep_arg.cosio - 3 * sat->deep_arg.theta2);
			float64_t f441 = 35 * sini2 * f220;
			float64_t f442 = 39.3750 * sini2 * sini2;
			float64_t f522 =
			    9.84375 * sat->deep_arg.sinio * (sini2 * (1 - 2 * sat->deep_arg.cosio - 5 * sat->deep_arg.theta2) + 0.33333333 * (-2 + 4 * sat->deep_arg.cosio + 6 * sat->deep_arg.theta2));
			float64_t f523 =
			    sat->deep_arg.sinio * (4.92187512 * sini2 * (-2 - 4 * sat->deep_arg.cosio + 10 * sat->deep_arg.theta2) + 6.56250012 * (1 + 2 * sat->deep_arg.cosio - 3 * sat->deep_arg.theta2));
			float64_t f542 = 29.53125 * sat->deep_arg.sinio * (2 - 8 * sat->deep_arg.cosio + sat->deep_arg.theta2 * (-12 + 8 * sat->deep_arg.cosio + 10 * sat->deep_arg.theta2));
			float64_t f543 = 29.53125 * sat->deep_arg.sinio * (-2 - 8 * sat->deep_arg.cosio + sat->deep_arg.theta2 * (12 + 8 * sat->deep_arg.cosio - 10 * sat->deep_arg.theta2));
			float64_t xno2 = sat->dps.xnq * sat->dps.xnq;
			float64_t ainv2 = aqnv * aqnv;
			float64_t temp1 = 3 * xno2 * ainv2;
			float64_t temp = temp1 * SAT_root22;
			sat->dps.d2201 = temp * f220 * g201;
			sat->dps.d2211 = temp * f221 * g211;
			temp1 = temp1 * aqnv;
			temp = temp1 * SAT_root32;
			sat->dps.d3210 = temp * f321 * g310;
			sat->dps.d3222 = temp * f322 * g322;
			temp1 = temp1 * aqnv;
			temp = 2 * temp1 * SAT_root44;
			sat->dps.d4410 = temp * f441 * g410;
			sat->dps.d4422 = temp * f442 * g422;
			temp1 = temp1 * aqnv;
			temp = temp1 * SAT_root52;
			sat->dps.d5220 = temp * f522 * g520;
			sat->dps.d5232 = temp * f523 * g532;
			temp = 2 * temp1 * SAT_root54;
			sat->dps.d5421 = temp * f542 * g521;
			sat->dps.d5433 = temp * f543 * g533;
			sat->dps.xlamo = xmao + sat->xnodeo + sat->xnodeo - sat->dps.thgr - sat->dps.thgr;
			bfact = sat->deep_arg.xmdot + sat->deep_arg.xnodot + sat->deep_arg.xnodot - SAT_thdt - SAT_thdt;
			bfact = bfact + sat->dps.ssl + sat->dps.ssh + sat->dps.ssh;
		} else {
			sat->flags |= SAT_RESONANCE_FLAG;
			sat->flags |= SAT_SYNCHRONOUS_FLAG;
			/* Synchronous resonance terms initialization */
			float64_t g200 = 1 + sat->deep_arg.eosq * (-2.5 + 0.8125 * sat->deep_arg.eosq);
			float64_t g310 = 1 + 2 * sat->deep_arg.eosq;
			float64_t g300 = 1 + sat->deep_arg.eosq * (-6 + 6.60937 * sat->deep_arg.eosq);
			float64_t f220 = 0.75 * (1 + sat->deep_arg.cosio) * (1 + sat->deep_arg.cosio);
			float64_t f311 = 0.9375 * sat->deep_arg.sinio * sat->deep_arg.sinio * (1 + 3 * sat->deep_arg.cosio) - 0.75 * (1 + sat->deep_arg.cosio);
			float64_t f330 = 1 + sat->deep_arg.cosio;
			f330 = 1.875 * f330 * f330 * f330;
			sat->dps.del1 = 3 * sat->dps.xnq * sat->dps.xnq * aqnv * aqnv;
			sat->dps.del2 = 2 * sat->dps.del1 * f220 * g200 * SAT_q22;
			sat->dps.del3 = 3 * sat->dps.del1 * f330 * g300 * SAT_q33 * aqnv;
			sat->dps.del1 = sat->dps.del1 * f311 * g310 * SAT_q31 * aqnv;
			sat->dps.fasx2 = 0.13130908;
			sat->dps.fasx4 = 2.8843198;
			sat->dps.fasx6 = 0.37448087;
			sat->dps.xlamo = xmao + sat->xnodeo + sat->omegao - sat->dps.thgr;
			bfact = sat->deep_arg.xmdot + xpidot - SAT_thdt;
			bfact = bfact + sat->dps.ssl + sat->dps.ssg + sat->dps.ssh;
		} /* End if( !(xnq < 0.0052359877) && (xnq > 0.0034906585) ) */

		sat->dps.xfact = bfact - sat->dps.xnq;

		/* Initialize integrator */
		sat->dps.xli = sat->dps.xlamo;
		sat->dps.xni = sat->dps.xnq;
		sat->dps.atime = 0;
		sat->dps.stepp = 720;
		sat->dps.stepn = -720;
		sat->dps.step2 = 259200;
		/* End case SAT_dpinit: */
		return;

	case SAT_dpsec: /* Entrance for deep space secular effects */
		sat->deep_arg.xll = sat->deep_arg.xll + sat->dps.ssl * sat->deep_arg.t;
		sat->deep_arg.omgadf = sat->deep_arg.omgadf + sat->dps.ssg * sat->deep_arg.t;
		sat->deep_arg.xnode = sat->deep_arg.xnode + sat->dps.ssh * sat->deep_arg.t;
		sat->deep_arg.em = sat->eo + sat->dps.sse * sat->deep_arg.t;
		sat->deep_arg.xinc = sat->xincl + sat->dps.ssi * sat->deep_arg.t;
		if (sat->deep_arg.xinc < 0) {
			sat->deep_arg.xinc = -sat->deep_arg.xinc;
			sat->deep_arg.xnode = sat->deep_arg.xnode + SAT_pi;
			sat->deep_arg.omgadf = sat->deep_arg.omgadf - SAT_pi;
		}
		if (~sat->flags & SAT_RESONANCE_FLAG) {
			return;
		}

		float64_t delt;
		float64_t ft;
		float64_t xldot;
		float64_t xndot;
		float64_t xnddt;
		do {
			if ((sat->dps.atime == 0) || ((sat->deep_arg.t >= 0) && (sat->dps.atime < 0)) || ((sat->deep_arg.t < 0) && (sat->dps.atime >= 0))) {
				/* Epoch restart */
				if (sat->deep_arg.t >= 0) {
					delt = sat->dps.stepp;
				} else {
					delt = sat->dps.stepn;
				}

				sat->dps.atime = 0;
				sat->dps.xni = sat->dps.xnq;
				sat->dps.xli = sat->dps.xlamo;
			} else {
				if (fabs(sat->deep_arg.t) >= fabs(sat->dps.atime)) {
					if (sat->deep_arg.t > 0) {
						delt = sat->dps.stepp;
					} else {
						delt = sat->dps.stepn;
					}
				}
			}

			do {
				if (fabs(sat->deep_arg.t - sat->dps.atime) >= sat->dps.stepp) {
					sat->flags |= SAT_DO_LOOP_FLAG;
					sat->flags &= ~SAT_EPOCH_RESTART_FLAG;
				} else {
					ft = sat->deep_arg.t - sat->dps.atime;
					sat->flags &= ~SAT_DO_LOOP_FLAG;
				}

				if (fabs(sat->deep_arg.t) < fabs(sat->dps.atime)) {
					if (sat->deep_arg.t >= 0) {
						delt = sat->dps.stepn;
					} else {
						delt = sat->dps.stepp;
					}
					sat->flags |= (SAT_DO_LOOP_FLAG | SAT_EPOCH_RESTART_FLAG);
				}

				/* Dot terms calculated */
				if (sat->flags & SAT_SYNCHRONOUS_FLAG) {
					xndot = sat->dps.del1 * sin(sat->dps.xli - sat->dps.fasx2) + sat->dps.del2 * sin(2 * (sat->dps.xli - sat->dps.fasx4)) + sat->dps.del3 * sin(3 * (sat->dps.xli - sat->dps.fasx6));
					xnddt = sat->dps.del1 * cos(sat->dps.xli - sat->dps.fasx2) + 2 * sat->dps.del2 * cos(2 * (sat->dps.xli - sat->dps.fasx4)) +
					        3 * sat->dps.del3 * cos(3 * (sat->dps.xli - sat->dps.fasx6));
				} else {
					float64_t xomi = sat->dps.omegaq + sat->deep_arg.omgdot * sat->dps.atime;
					float64_t x2omi = xomi + xomi;
					float64_t x2li = sat->dps.xli + sat->dps.xli;
					xndot = sat->dps.d2201 * sin(x2omi + sat->dps.xli - SAT_g22) + sat->dps.d2211 * sin(sat->dps.xli - SAT_g22) + sat->dps.d3210 * sin(xomi + sat->dps.xli - SAT_g32) +
					        sat->dps.d3222 * sin(-xomi + sat->dps.xli - SAT_g32) + sat->dps.d4410 * sin(x2omi + x2li - SAT_g44) + sat->dps.d4422 * sin(x2li - SAT_g44) +
					        sat->dps.d5220 * sin(xomi + sat->dps.xli - SAT_g52) + sat->dps.d5232 * sin(-xomi + sat->dps.xli - SAT_g52) + sat->dps.d5421 * sin(xomi + x2li - SAT_g54) +
					        sat->dps.d5433 * sin(-xomi + x2li - SAT_g54);
					xnddt = sat->dps.d2201 * cos(x2omi + sat->dps.xli - SAT_g22) + sat->dps.d2211 * cos(sat->dps.xli - SAT_g22) + sat->dps.d3210 * cos(xomi + sat->dps.xli - SAT_g32) +
					        sat->dps.d3222 * cos(-xomi + sat->dps.xli - SAT_g32) + sat->dps.d5220 * cos(xomi + sat->dps.xli - SAT_g52) + sat->dps.d5232 * cos(-xomi + sat->dps.xli - SAT_g52) +
					        2 * (sat->dps.d4410 * cos(x2omi + x2li - SAT_g44) + sat->dps.d4422 * cos(x2li - SAT_g44) + sat->dps.d5421 * cos(xomi + x2li - SAT_g54) +
					             sat->dps.d5433 * cos(-xomi + x2li - SAT_g54));
				} /* End of if (isFlagSet(SYNCHRONOUS_FLAG)) */

				xldot = sat->dps.xni + sat->dps.xfact;
				xnddt = xnddt * xldot;

				if (sat->flags & SAT_DO_LOOP_FLAG) {
					sat->dps.xli = sat->dps.xli + xldot * delt + xndot * sat->dps.step2;
					sat->dps.xni = sat->dps.xni + xndot * delt + xnddt * sat->dps.step2;
					sat->dps.atime = sat->dps.atime + delt;
				}
			} while ((sat->flags & SAT_DO_LOOP_FLAG) && (~sat->flags & SAT_EPOCH_RESTART_FLAG));
		} while ((sat->flags & SAT_DO_LOOP_FLAG) && (sat->flags & SAT_EPOCH_RESTART_FLAG));

		sat->deep_arg.xn = sat->dps.xni + xndot * ft + xnddt * ft * ft * 0.5;
		float64_t xl = sat->dps.xli + xldot * ft + xndot * ft * ft * 0.5;
		float64_t temp = -sat->deep_arg.xnode + sat->dps.thgr + sat->deep_arg.t * SAT_thdt;

		if (~sat->flags & SAT_SYNCHRONOUS_FLAG) {
			sat->deep_arg.xll = xl + temp + temp;
		} else {
			sat->deep_arg.xll = xl - sat->deep_arg.omgadf + temp;
		}

		return;
		/* End case dpsec: */

		float64_t sinis;
		float64_t cosis;
	case SAT_dpper: /* Entrance for lunar-solar periodics */
		sinis = sin(sat->deep_arg.xinc);
		cosis = cos(sat->deep_arg.xinc);
		if (fabs(sat->dps.savtsn - sat->deep_arg.t) >= 30) {
			sat->dps.savtsn = sat->deep_arg.t;
			float64_t zm = sat->dps.zmos + SAT_zns * sat->deep_arg.t;
			float64_t zf = zm + 2 * SAT_zes * sin(zm);
			float64_t sinzf = sin(zf);
			float64_t f2 = 0.5 * sinzf * sinzf - 0.25;
			float64_t f3 = -0.5 * sinzf * cos(zf);
			float64_t ses = sat->dps.se2 * f2 + sat->dps.se3 * f3;
			float64_t sis = sat->dps.si2 * f2 + sat->dps.si3 * f3;
			float64_t sls = sat->dps.sl2 * f2 + sat->dps.sl3 * f3 + sat->dps.sl4 * sinzf;
			sat->dps.sghs = sat->dps.sgh2 * f2 + sat->dps.sgh3 * f3 + sat->dps.sgh4 * sinzf;
			sat->dps.shs = sat->dps.sh2 * f2 + sat->dps.sh3 * f3;
			zm = sat->dps.zmol + SAT_znl * sat->deep_arg.t;
			zf = zm + 2 * SAT_zel * sin(zm);
			sinzf = sin(zf);
			f2 = 0.5 * sinzf * sinzf - 0.25;
			f3 = -0.5 * sinzf * cos(zf);
			float64_t sel = sat->dps.ee2 * f2 + sat->dps.e3 * f3;
			float64_t sil = sat->dps.xi2 * f2 + sat->dps.xi3 * f3;
			float64_t sll = sat->dps.xl2 * f2 + sat->dps.xl3 * f3 + sat->dps.xl4 * sinzf;
			sat->dps.sghl = sat->dps.xgh2 * f2 + sat->dps.xgh3 * f3 + sat->dps.xgh4 * sinzf;
			sat->dps.sh1 = sat->dps.xh2 * f2 + sat->dps.xh3 * f3;
			sat->dps.pe = ses + sel;
			sat->dps.pinc = sis + sil;
			sat->dps.pl = sls + sll;
		}

		float64_t pgh = sat->dps.sghs + sat->dps.sghl;
		float64_t ph = sat->dps.shs + sat->dps.sh1;
		sat->deep_arg.xinc = sat->deep_arg.xinc + sat->dps.pinc;
		sat->deep_arg.em = sat->deep_arg.em + sat->dps.pe;

		if (sat->dps.xqncl >= 0.2) {
			/* Apply periodics directly */
			ph = ph / sat->deep_arg.sinio;
			pgh = pgh - sat->deep_arg.cosio * ph;
			sat->deep_arg.omgadf = sat->deep_arg.omgadf + pgh;
			sat->deep_arg.xnode = sat->deep_arg.xnode + ph;
			sat->deep_arg.xll = sat->deep_arg.xll + sat->dps.pl;
		} else {
			/* Apply periodics with Lyddane modification */
			float64_t sinok = sin(sat->deep_arg.xnode);
			float64_t cosok = cos(sat->deep_arg.xnode);
			float64_t alfdp = sinis * sinok;
			float64_t betdp = sinis * cosok;
			float64_t dalf = ph * cosok + sat->dps.pinc * cosis * sinok;
			float64_t dbet = -ph * sinok + sat->dps.pinc * cosis * cosok;
			alfdp = alfdp + dalf;
			betdp = betdp + dbet;
			sat->deep_arg.xnode = SAT_FMod2p(sat->deep_arg.xnode);
			float64_t xls = sat->deep_arg.xll + sat->deep_arg.omgadf + cosis * sat->deep_arg.xnode;
			float64_t dls = sat->dps.pl + pgh - sat->dps.pinc * sat->deep_arg.xnode * sinis;
			xls = xls + dls;
			float64_t xnoh = sat->deep_arg.xnode;
			sat->deep_arg.xnode = SAT_AcTan(alfdp, betdp);

			/* This is a patch to Lyddane modification */
			/* suggested by Rob Matson. */
			if (fabs(xnoh - sat->deep_arg.xnode) > SAT_pi) {
				if (sat->deep_arg.xnode < xnoh) {
					sat->deep_arg.xnode += SAT_twopi;
				} else {
					sat->deep_arg.xnode -= SAT_twopi;
				}
			}

			sat->deep_arg.xll = sat->deep_arg.xll + sat->dps.pl;
			sat->deep_arg.omgadf = xls - sat->deep_arg.xll - cos(sat->deep_arg.xinc) * sat->deep_arg.xnode;
		} /* End case dpper: */
		return;

	} /* End switch(ientry) */

} /* End of Deep() */

/* Returns angle in degrees from arg in rads */
static float64_t SAT_Degrees(float64_t arg) { return arg / SAT_de2ra; }

/* Returns mod 2pi of argument */
static float64_t SAT_FMod2p(float64_t x) {
	float64_t ret_val = x;
	int32_t i = (int)(ret_val / SAT_twopi);
	ret_val -= (float64_t)i * SAT_twopi;

	if (ret_val < 0) {
		ret_val += SAT_twopi;
	}

	return ret_val;
}

/* Four-quadrant arctan function */
static float64_t SAT_AcTan(float64_t sinx, float64_t cosx) {
	if (cosx == 0) {
		if (sinx > 0) {
			return SAT_pio2;
		} else {
			return SAT_x3pio2;
		}
	} else {
		if (cosx > 0) {
			if (sinx > 0) {
				return atan(sinx / cosx);
			} else {
				return SAT_twopi + atan(sinx / cosx);
			}
		} else {
			return SAT_pi + atan(sinx / cosx);
		}
	}
}

/* Converts the satellite's position and velocity  */
/* vectors from normalised values to km and km/sec */
static void SAT_Convert_Sat_State(SAT_Vector *pos, SAT_Vector *vel) {
	SAT_Scale_Vector(SAT_xkmper, pos);
	SAT_Scale_Vector(SAT_xkmper * SAT_xmnpda / SAT_secday, vel);
}

/* Multiplies the vector v1 by the scalar k */
static void SAT_Scale_Vector(float64_t k, SAT_Vector *v) {
	v->x *= k;
	v->y *= k;
	v->z *= k;

	v->w = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

/* The function ThetaG calculates the Greenwich Mean Sidereal Time */
/* for an epoch specified in the format used in the NORAD two-line */
/* element sets. It has now been adapted for dates beyond the year */
/* 1999, as described above. The function ThetaG_JD provides the   */
/* same calculation except that it is based on an input in the     */
/* form of a Julian Date. */
static float64_t SAT_ThetaG(float64_t epoch, SAT_DeepArg *deep_arg) {
	/* Reference:  The 1992 Astronomical Almanac, page B6. */
	// double year,day,UT,jd,TU,GMST,_ThetaG;

	/* Modification to support Y2K */
	/* Valid 1957 through 2056     */
	float64_t year = 0;
	float64_t day = modf(epoch * 1E-3, &year) * 1E3;

	if (year < 57) {
		year += 2000;
	} else {
		year += 1900;
	}
	/* End modification */

	float64_t UT = fmod(day, day);
	float64_t jd = SAT_Julian_Date_of_Year(year) + day;
	float64_t TU = (jd - 2451545.0) / 36525;
	float64_t GMST = 24110.54841 + TU * (8640184.812866 + TU * (0.093104 - TU * 6.2E-6));
	GMST = SAT_Modulus(GMST + SAT_secday * SAT_omega_E * UT, SAT_secday);
	deep_arg->ds50 = jd - 2433281.5 + UT;

	return SAT_FMod2p(6.3003880987 * deep_arg->ds50 + 1.72944494);
}

/* The function Julian_Date_of_Year calculates the Julian Date  */
/* of Day 0.0 of {year}. This function is used to calculate the */
/* Julian Date of any date by using Julian_Date_of_Year, DOY,   */
/* and Fraction_of_Day. */
static float64_t SAT_Julian_Date_of_Year(uint32_t year) {
	/* Astronomical Formulae for Calculators, Jean Meeus, */
	/* pages 23-25. Calculate Julian Date of 0.0 Jan year */
	year = year - 1;
	float64_t i = (int)(year / 100);
	float64_t A = i;
	i = (int)(A / 4);
	float64_t B = (int)(2 - A + i);
	i = (int)(365.25 * year);
	i += (int)(30.6001 * 14);
	float64_t jdoy = i + 1720994.5 + B;

	return jdoy;
}

/* Returns arg1 mod arg2 */
static float64_t SAT_Modulus(float64_t arg1, float64_t arg2) {
	float64_t ret_val = arg1;
	int32_t i = (int)(ret_val / arg2);
	ret_val -= (float64_t)i * arg2;

	if (ret_val < 0) {
		ret_val += arg2;
	}

	return ret_val;
}

/* The procedures Calculate_Obs and Calculate_RADec calculate         */
/* the *topocentric* coordinates of the object with ECI position,     */
/* {pos}, and velocity, {vel}, from location {geodetic} at {time}.    */
/* The {obs_set} returned for Calculate_Obs consists of azimuth,      */
/* elevation, range, and range rate (in that order) with units of     */
/* radians, radians, kilometers, and kilometers/second, respectively. */
/* The WGS '72 geoid is used and the effect of atmospheric refraction */
/* (under standard temperature and pressure) is incorporated into the */
/* elevation calculation; the effect of atmospheric refraction on     */
/* range and range rate has not yet been quantified.                  */

/* The {obs_set} for Calculate_RADec consists of right ascension and  */
/* declination (in that order) in radians.  Again, calculations are   */
/* based on *topocentric* position using the WGS '72 geoid and        */
/* incorporating atmospheric refraction.                              */
static void SAT_Calculate_Obs(float64_t _time, SAT_Vector *pos, SAT_Vector *vel, SAT_Geodetic *geodetic, SAT_ObsSet *obs_set) {
	SAT_Vector obs_pos;
	SAT_Vector obs_vel;
	SAT_Vector range;
	SAT_Vector rgvel;

	SAT_Calculate_User_PosVel(_time, geodetic, &obs_pos, &obs_vel);

	range.x = pos->x - obs_pos.x;
	range.y = pos->y - obs_pos.y;
	range.z = pos->z - obs_pos.z;

	rgvel.x = vel->x - obs_vel.x;
	rgvel.y = vel->y - obs_vel.y;
	rgvel.z = vel->z - obs_vel.z;

	range.w = sqrt(range.x * range.x + range.y * range.y + range.z * range.z);

	float64_t sin_lat = sin(geodetic->lat);
	float64_t cos_lat = cos(geodetic->lat);
	float64_t sin_theta = sin(geodetic->theta);
	float64_t cos_theta = cos(geodetic->theta);
	float64_t top_s = sin_lat * cos_theta * range.x + sin_lat * sin_theta * range.y - cos_lat * range.z;
	float64_t top_e = -sin_theta * range.x + cos_theta * range.y;
	float64_t top_z = cos_lat * cos_theta * range.x + cos_lat * sin_theta * range.y + sin_lat * range.z;
	float64_t azim = atan(-top_e / top_s); /*Azimuth*/
	if (top_s > 0) {
		azim = azim + SAT_pi;
	}
	if (azim < 0) {
		azim = azim + SAT_twopi;
	}
	float64_t el = SAT_ArcSin(top_z / range.w);
	obs_set->az = azim;       /* Azimuth (radians)  */
	obs_set->el = el;         /* Elevation (radians)*/
	obs_set->range = range.w; /* Range (kilometers) */

	/* Range Rate (kilometers/second)*/
	obs_set->range_rate = SAT_Dot(&range, &rgvel) / range.w;

	/* Corrections for atmospheric refraction */
	/* Reference:  Astronomical Algorithms by Jean Meeus, pp. 101-104    */
	/* Correction is meaningless when apparent elevation is below horizon */
	//	obs_set->el = obs_set->el + Radians((1.02/tan(Radians(Degrees(el)+
	//							      10.3/(Degrees(el)+5.11))))/60);
	if (obs_set->el < 0) {
		obs_set->el = el; /*Reset to true elevation*/
	}
}

/* Returns the dot product of two vectors */
static float64_t SAT_Dot(SAT_Vector *v1, SAT_Vector *v2) { return (v1->x * v2->x + v1->y * v2->y + v1->z * v2->z); }

/* Returns the arcsine of the argument */
static float64_t SAT_ArcSin(float64_t arg) {
	if (fabs(arg) >= 1) {
		return (SAT_Sign(arg) * SAT_pio2);
	} else {
		return (atan(arg / sqrt(1 - arg * arg)));
	}
}

/* Returns sign of a float */
static int8_t SAT_Sign(float64_t arg) {
	if (arg > 0) {
		return 1;
	} else if (arg < 0) {
		return -1;
	} else {
		return 0;
	}
}

/* Procedure Calculate_User_PosVel passes the user's geodetic position */
/* and the time of interest and returns the ECI position and velocity  */
/* of the observer. The velocity calculation assumes the geodetic      */
/* position is stationary relative to the earth's surface.             */
static void SAT_Calculate_User_PosVel(float64_t _time, SAT_Geodetic *geodetic, SAT_Vector *obs_pos, SAT_Vector *obs_vel) {
	/* Reference:  The 1992 Astronomical Almanac, page K11. */

	float64_t sinGeodeticLat = sin(geodetic->lat); /* Only run sin($geodetic->lat) once */

	geodetic->theta = SAT_FMod2p(SAT_ThetaG_JD(_time) + geodetic->lon); /*LMST*/
	float64_t c = 1 / sqrt(1 + SAT___f * (SAT___f - 2) * sinGeodeticLat * sinGeodeticLat);
	float64_t sq = (1 - SAT___f) * (1 - SAT___f) * c;
	float64_t achcp = (SAT_xkmper * c + geodetic->alt) * cos(geodetic->lat);
	obs_pos->x = achcp * cos(geodetic->theta); /*kilometers*/
	obs_pos->y = achcp * sin(geodetic->theta);
	obs_pos->z = (SAT_xkmper * sq + geodetic->alt) * sinGeodeticLat;
	obs_vel->x = -SAT_mfactor * obs_pos->y; /*kilometers/second*/
	obs_vel->y = SAT_mfactor * obs_pos->x;
	obs_vel->z = 0;
	obs_pos->w = sqrt(obs_pos->x * obs_pos->x + obs_pos->y * obs_pos->y + obs_pos->z * obs_pos->z);
	obs_vel->w = sqrt(obs_vel->x * obs_vel->x + obs_vel->y * obs_vel->y + obs_vel->z * obs_vel->z);
}

/* See the ThetaG doc block above */
static float64_t SAT_ThetaG_JD(float64_t jd) {
	/* Reference:  The 1992 Astronomical Almanac, page B6. */
	float64_t UT = SAT_Frac(jd + 0.5);
	jd = jd - UT;
	float64_t TU = (jd - 2451545.0) / 36525;
	float64_t GMST = 24110.54841 + TU * (8640184.812866 + TU * (0.093104 - TU * 6.2E-6));
	GMST = SAT_Modulus(GMST + SAT_secday * SAT_omega_E * UT, SAT_secday);

	return SAT_twopi * GMST / SAT_secday;
}

/* Returns fractional part of double argument */
static float64_t SAT_Frac(float64_t arg) { return arg - floor(arg); }

/* Procedure Calculate_LatLonAlt will calculate the geodetic  */
/* position of an object given its ECI position pos and time. */
/* It is intended to be used to determine the ground track of */
/* a satellite.  The calculations  assume the earth to be an  */
/* oblate spheroid as defined in WGS '72.                     */
static void SAT_Calculate_LatLonAlt(float64_t _time, SAT_Vector *pos, SAT_Geodetic *geodetic) {
	/* Reference:  The 1992 Astronomical Almanac, page K12. */

	geodetic->theta = SAT_AcTan(pos->y, pos->x);                        /*radians*/
	geodetic->lon = SAT_FMod2p(geodetic->theta - SAT_ThetaG_JD(_time)); /*radians*/
	float64_t r = sqrt((pos->x * pos->x) + (pos->y * pos->y));
	float64_t e2 = SAT___f * (2 - SAT___f);
	geodetic->lat = SAT_AcTan(pos->z, r); /*radians*/

	float64_t phi;
	float64_t c;
	float64_t sinPhi;
	do {
		phi = geodetic->lat;
		sinPhi = sin(phi);
		c = 1 / sqrt(1 - e2 * (sinPhi * sinPhi));
		geodetic->lat = SAT_AcTan(pos->z + SAT_xkmper * c * e2 * sinPhi, r);
	} while (fabs(geodetic->lat - phi) >= 1E-10);

	geodetic->alt = r / cos(geodetic->lat) - SAT_xkmper * c; /*kilometers*/

	if (geodetic->lat > SAT_pio2) {
		geodetic->lat -= SAT_twopi;
	}
}

/**
 * Converts a standard unix timestamp and optional
 * milliseconds to a daynum
 *
 * @param int $sec  Seconds from the unix epoch
 * @param int $usec Optional milliseconds
 *
 * @return float
 */
float64_t SAT_unix2daynum(float64_t sec, float64_t usec) {
	float64_t time = (((sec + usec) / 86400.0) - 3651.0);
	return time + 2444238.5;
}

/* The function Julian_Date_of_Epoch returns the Julian Date of     */
/* an epoch specified in the format used in the NORAD two-line      */
/* element sets. It has been modified to support dates beyond       */
/* the year 1999 assuming that two-digit years in the range 00-56   */
/* correspond to 2000-2056. Until the two-line element set format   */
/* is changed, it is only valid for dates through 2056 December 31. */
static float64_t SAT_Julian_Date_of_Epoch(float64_t epoch) {
	float64_t year = 0;

	/* Modification to support Y2K */
	/* Valid 1957 through 2056     */
	float64_t day = modf(epoch * 1E-3, &year) * 1E3;
	if (year < 57) {
		year = year + 2000;
	} else {
		year = year + 1900;
	}
	/* End modification */

	return SAT_Julian_Date_of_Year(year) + day;
}
