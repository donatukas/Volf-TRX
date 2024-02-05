#ifndef SATTILITE_h
#define SATTILITE_h

#include "settings.h"

typedef struct {
	float64_t aodp;
	float64_t aycof;
	float64_t c1;
	float64_t c4;
	float64_t c5;
	float64_t cosio;
	float64_t d2;
	float64_t d3;
	float64_t d4;
	float64_t delmo;
	float64_t omgcof;
	float64_t eta;
	float64_t omgdot;
	float64_t sinio;
	float64_t xnodp;
	float64_t sinmo;
	float64_t t2cof;
	float64_t t3cof;
	float64_t t4cof;
	float64_t t5cof;
	float64_t x1mth2;
	float64_t x3thm1;
	float64_t x7thm1;
	float64_t xmcof;
	float64_t xmdot;
	float64_t xnodcf;
	float64_t xnodot;
	float64_t xlcof;
} SAT_SGSDPStatic;

typedef struct {
	float64_t thgr;
	float64_t xnq;
	float64_t xqncl;
	float64_t omegaq;
	float64_t zmol;
	float64_t zmos;
	float64_t savtsn;
	float64_t ee2;
	float64_t e3;
	float64_t xi2;
	float64_t xl2;
	float64_t xl3;
	float64_t xl4;
	float64_t xgh2;
	float64_t xgh3;
	float64_t xgh4;
	float64_t xh2;
	float64_t xh3;
	float64_t sse;
	float64_t ssi;
	float64_t ssg;
	float64_t xi3;
	float64_t se2;
	float64_t si2;
	float64_t sl2;
	float64_t sgh2;
	float64_t sh2;
	float64_t se3;
	float64_t si3;
	float64_t sl3;
	float64_t sgh3;
	float64_t sh3;
	float64_t sl4;
	float64_t sgh4;
	float64_t ssl;
	float64_t ssh;
	float64_t d3210;
	float64_t d3222;
	float64_t d4410;
	float64_t d4422;
	float64_t d5220;
	float64_t d5232;
	float64_t d5421;
	float64_t d5433;
	float64_t del1;
	float64_t del2;
	float64_t del3;
	float64_t fasx2;
	float64_t fasx4;
	float64_t fasx6;
	float64_t xlamo;
	float64_t xfact;
	float64_t xni;
	float64_t atime;
	float64_t stepp;
	float64_t stepn;
	float64_t step2;
	float64_t preep;
	float64_t pl;
	float64_t sghs;
	float64_t xli;
	float64_t d2201;
	float64_t d2211;
	float64_t sghl;
	float64_t sh1;
	float64_t pinc;
	float64_t pe;
	float64_t shs;
	float64_t zsingl;
	float64_t zcosgl;
	float64_t zsinhl;
	float64_t zcoshl;
	float64_t zsinil;
	float64_t zcosil;
} SAT_DeepStatic;

typedef struct {
	float64_t x;
	float64_t y;
	float64_t z;
	float64_t w;
} SAT_Vector;

typedef struct {
	/* Used by dpinit part of Deep() */
	float64_t eosq;
	float64_t sinio;
	float64_t cosio;
	float64_t betao;
	float64_t aodp;
	float64_t theta2;
	float64_t sing;
	float64_t cosg;
	float64_t betao2;
	float64_t xmdot;
	float64_t omgdot;
	float64_t xnodot;
	float64_t xnodp;

	/* Used by dpsec and dpper parts of Deep() */
	float64_t xll;
	float64_t omgadf;
	float64_t xnode;
	float64_t em;
	float64_t xinc;
	float64_t xn;
	float64_t t;

	/* Used by thetg and Deep() */
	float64_t ds50;
} SAT_DeepArg;

typedef struct {
	// TLE parsed data
	uint32_t catnr;      // Satellite's catalogue number
	char idesg[9];       // International Designator for satellite
	float64_t epoch;     // Epoch time; this is the complete, unconverted epoch.
	uint16_t epoch_year; // Epoch year
	uint16_t epoch_day;  // Epoch day
	float64_t epoch_fod; // Epoch fraction of day
	float64_t xndt2o;    // Satellite's First Time Derivative
	float64_t xndd6o;    // Satellite's Second Time Derivative
	float64_t bstar;     // Satellite's bstar drag term
	uint32_t elset;      // Element Number
	float64_t xincl;     // Satellite's Orbital Inclination (degrees)
	float64_t xnodeo;    // Satellite's RAAN (degrees)
	float64_t eo;        // Satellite's Orbital Eccentricity
	float64_t omegao;    // Satellite's Argument of Perigee (degrees)
	float64_t xmo;       // Satellite's Mean Anomaly of Orbit (degrees)
	float64_t xno;       // Satellite's Mean Motion (rev/day)
	uint32_t revnum;     // Satellite's Revolution number at epoch
	// SAT data
	float64_t jul_utc;
	float64_t jul_epoch;
	float64_t tsince;
	float64_t meanmo;     // mean motion kept in rev/day
	float64_t az;         // Azimuth [deg]
	float64_t el;         // Elevation [deg]
	float64_t range;      // Range [km]
	float64_t range_rate; // Range Rate [km/sec]
	float64_t ssplat;     // SSP latitude [deg]
	float64_t ssplon;     // SSP longitude [deg]
	float64_t alt;        // altitude [km]
	float64_t velo;       // velocity [km/s]
	float64_t ma;         // mean anomaly
	float64_t footprint;  // footprint
	float64_t phase;      // orbit phase
	float64_t orbit;      // orbit number
	uint32_t flags;
	// Misc
	SAT_SGSDPStatic sgps;
	SAT_Vector pos;
	SAT_Vector vel;
	SAT_DeepArg deep_arg;
	SAT_DeepStatic dps;
	float64_t xincl1;
	float64_t xnodeo1;
	float64_t omegao1;
} SAT_Instance;

typedef struct {
	float64_t lat; // Lattitude [rad]
	float64_t lon; // Longitude [rad]
	float64_t alt; // Altitude [km]
	float64_t theta;
} SAT_Geodetic;

typedef struct {
	float64_t az;         // Azimuth [deg]
	float64_t el;         // Elevation [deg]
	float64_t range;      // Range [km]
	float64_t range_rate; // Velocity [km/sec]
} SAT_ObsSet;

#define SAT_de2ra 1.74532925E-2   // Degrees to Radians
#define SAT_pi 3.1415926535898    // Pi
#define SAT_twopi 6.2831853071796 // 2*Pi
#define SAT_pio2 1.5707963267949  // Pi/2
#define SAT_x3pio2 4.71238898     // 3*Pi/2
#define SAT_xmnpda 1.44E3         // Minutes per day
#define SAT_secday 8.6400E4       // Seconds per day
#define SAT_ae 1.0
#define SAT_xke 7.43669161E-2
#define SAT_tothrd 6.6666667E-1 // 2/3
#define SAT_ck2 5.413079E-4
#define SAT_ck4 6.209887E-7
#define SAT_xkmper 6.378135E3 // Earth radius km
#define SAT___s__ 1.012229
#define SAT___f 3.352779E-3
#define SAT_mfactor 7.292115E-5
#define SAT_qoms2t 1.880279E-09
#define SAT_e6a 1.0E-6
#define SAT_xj3 -2.53881E-6 // J3 Harmonic
#define SAT_omega_E 1.0027379
#define SAT_zcosis 9.1744867E-1
#define SAT_zsinis 3.9785416E-1
#define SAT_zsings -9.8088458E-1
#define SAT_zcosgs 1.945905E-1
#define SAT_zns 1.19459E-5
#define SAT_c1ss 2.9864797E-6
#define SAT_zes 1.675E-2
#define SAT_znl 1.5835218E-4
#define SAT_c1l 4.7968065E-7
#define SAT_zel 5.490E-2
#define SAT_root22 1.7891679E-6
#define SAT_root32 3.7393792E-7
#define SAT_root44 7.3636953E-9
#define SAT_root52 1.1428639E-7
#define SAT_root54 2.1765803E-9
#define SAT_thdt 4.3752691E-3
#define SAT_q22 1.7891679E-6
#define SAT_q31 2.1460748E-6
#define SAT_q33 2.2123015E-7
#define SAT_g22 5.7686396
#define SAT_g32 9.5240898E-1
#define SAT_g44 1.8014998
#define SAT_g52 1.0508330
#define SAT_g54 4.4108898

#define SAT_dpinit 1 /* Deep-space initialization code */
#define SAT_dpsec 2  /* Deep-space secular code        */
#define SAT_dpper 3  /* Deep-space periodic code       */

#define SAT_DEEP_SPACE_EPHEM_FLAG 0x000040
#define SAT_SGP4_INITIALIZED_FLAG 0x000002
#define SAT_SDP4_INITIALIZED_FLAG 0x000004
#define SAT_SIMPLE_FLAG 0x000020
#define SAT_LUNAR_TERMS_DONE_FLAG 0x000080
#define SAT_RESONANCE_FLAG 0x000400
#define SAT_SYNCHRONOUS_FLAG 0x000800
#define SAT_DO_LOOP_FLAG 0x000200
#define SAT_EPOCH_RESTART_FLAG 0x001000

extern char SAT_TLE_NAMES[SAT_TLE_MAXCOUNT][SAT_NAME_MAXLEN + 1];
extern SAT_Instance SATTELITE;

extern void SAT_init();
extern void SAT_calc(float64_t t);
extern float64_t SAT_unix2daynum(float64_t sec, float64_t usec);

#endif
