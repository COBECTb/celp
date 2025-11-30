/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:NOexpandtab */

/**************************************************************************
*
* ROUTINE
*		psearch
*
* FUNCTION
*		find pitch VQ parameters
*
* SYNOPSIS
*		subroutine psearch(l)
*
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	l		int	i	dimension of s
*
*   external 
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	IDB		int	i	dimension of d1a and d1b???
*	NO		int	i	filter order  predictor
*	NSEG		int	i	segment counter
*	PINDEX		int	i/o	pitch gain index BB[2]
*	BB		float	i/o	pitch predictor coefficients
*	d1b[]		float	i/o	memory 1/P(z)
*	frame		int	i
*	h[]		float	i/o	impulse response
*	PTYPE[] 	char	i	pitch gain (BB[2]) quantizer type
*	PSTYPE[]	char	i	pitch search type
*	pitch_gain_vid	int	i
*	pitch_qgain_vid int	i
*	pitch_match_vid int	i
*	pitch_ir_vid	int	i
*	fndpp_v0_vid	int	i
*	TAUPTR		int		pitch delay pointer
*	PDELAY		float		pitch delay coding table
*	MINPTR		int		minimum delay pointer
*	PLEVEL1 	int		number of full search pitch delays
*	PLEVEL2 	int		number of delta search pitch delays
*
***************************************************************************
*	
* DESCRIPTION
*
*  Pitch search is performed by closed-loop analysis using a modification
*  of what is commonly called any one of the following: "self-excited",
*  "adaptive code book" or "VQ" method.  This method was found to be
*  superior to the conventional "filtering approach", especially for high
*  pitched speakers.  The filtering and VQ methods are identical except when
*  the delay is less than the frame length.  The pitch delay ranges from MINPTR
*  to maxptr (i.e., 20 to 147 including NOninteger) lags every odd subframe 
*  while even subframes are searched and coded within 2**pbits[1] (i.e., 32)  
*  lags relative to the previous subframe.  The delta search greatly reduces
*  the computational complexity and data rate while causing NO percievable 
*  loss in speech quality.
*
*  The minimum squared prediction error (MSPE) search criteria is modified
*  to check the error at submultiples of the delay to determine if it is
*  within 1 dB of the MSPE.  The submultiple delay is selected if its error
*  satisfies our modified criteria.  This results in a smooth "pitch" delay
*  contour which produces higher quality speech and is crucial to the
*  synthesizer's smoother in the presence of bit errors. 
*	
***************************************************************************
*
* CALLED BY
*
*	csub
*
* CALLS
*
*	 movefr   pitchencode	pgain   delay
*
**************************************************************************
*
* REFRENCES
*
*	Tremain, Thomas E., Joseph P. Campbell, Jr and VaNOy C. Welch,
*       "A 4.8 kbps Code Excited Linear Predictive Coder," Proceedings
*	of the Mobile Satellite Conference, 3-5 May 1988, pp. 491-496.
*
*	Campbell, Joseph P. Jr., VaNOy C. Welch and Thomas E. Tremain,
*       "An Expandable Error-Protected 4800 bps CELP Coder (U.S. Federal
*       Standard 4800 bps Voice Coder)," Proceedings of ICASSP, 1989.
*	(and Proceedings of Speech Tech, 1989.)
*
*       Kroon, Peter and Bishnu Atal, "On Improving the Performance of
*       Pitch Predictions in Speech Coding Systems," IEEE Speech Coding
*	Workshop, September 1989.
*
*       Marques, J.S., et al., "Pitch Prediction with Fractional Delays
*       in CELP Coding," European Conference on Speech Communication and
*	TechNOlogy, September, 1989.
*
**************************************************************************/
#define LEN		30	/* *length of truncated impulse response     */

#define MAXBUFPTR	MMAX + MAXNO + 2  * MAXLP + MAXNP - 1

static const int submult[MAXPD][4] = {
#include "submult.h"		/* *load pitch submultiple delay table         */
};

static void psearch(int l)
{
	int i, m, lag, start;
	int first, bigptr, subptr, topptr, maxptr, bufptr;
	static int oldptr = { 1 };
	float g[MAXPD], match[MAXPD], emax;
	int fraction, neigh, whole, sub, nrange = 0;

	/*    See warning below ----------------  \/ max (MAXL, MAXLP)        */

	float v0[MAXBUFPTR], v0shift[MAXLP], frac;

/*									 */
/* *choose type of pitch delay search:					 */
/*	*two stage hierarchical search of integer and neighboring	 */
/*	*NOninteger delays						 */

	if (strcmp(PSTYPE, "hier") == 0) {
		whole = 1;
		fraction = 0;
		sub = 1;
		neigh = 1;
		nrange = 3;
	}

	/*    *integer only search                                             */

	else if (strcmp(PSTYPE, "intg") == 0) {
		whole = 1;
		fraction = 0;
		sub = 1;
		neigh = 0;
	}

	/*    *full exhaustive search                                          */

	else if (strcmp(PSTYPE, "full") == 0) {
		whole = 1;
		fraction = 1;
		sub = 1;
		neigh = 0;
	} else {
#ifdef CELPDIAG
		fprintf(stderr,
			"psearch: incorrect pitch search type (PSTYPE)");
#endif
		CELP_ERROR(CELP_ERR_PITCH_TYPE);
		return;
	}

	/* *initialize arrays                                                  */

	for (i = 0; i < MAXBUFPTR; i++)
		v0[i] = 0.0;

	for (i = 0; i < MAXLP; i++)
		v0shift[i] = 0.0;

	for (i = 0; i < MAXPD; i++)
		g[i] = match[i] = 0.0;

	if (LEN > l) {
#ifdef CELPDIAG
		fprintf(stderr, "psearch: impulse resp too long\n");
#endif
		CELP_ERROR(CELP_ERR_IMPULSE_LENGTH);
		return;
	}
	bufptr = MMAX + NO + 2 * l + MAXNP - 1;
	if (MAXLP < MAXL) {
#ifdef CELPDIAG
		fprintf(stderr, "psearch: MAXLP < MAXL\n");
#endif
		CELP_ERROR(CELP_ERR_MAXLP);
		return;
	}

	/* *update adaptive code book (pitch memory)                           */

	movefr(IDB, d1b, &v0[bufptr - IDB - l]);

	/* *initial conditions                                                 */

	if (NSEG == 1) {
		BB[2] = 0.0;
		BB[0] = MMIN;
	} else {

		/*          *find allowable pointer range (MINPTR to maxptr)         */

		if ((NSEG % 2) == 0) {

			/* *delta delay coding on even subframes                             */

			MINPTR = oldptr - (PLEVEL2 / 2 - 1);
			maxptr = oldptr + (PLEVEL2 / 2);
			if (MINPTR < 0) {
				MINPTR = 0;
				maxptr = PLEVEL2 - 1;
			}
			if (maxptr > PLEVEL1 - 1) {
				maxptr = PLEVEL1 - 1;
				MINPTR = PLEVEL1 - PLEVEL2;
			}
		} else {

			/* *full range coding on odd subframes                               */

			MINPTR = 0;
			maxptr = PLEVEL1 - 1;
		}

		start = bufptr - l + 1;

		/* *find gain and match score for integer pitch delays               */
		/* *(using end-point correction on unity spaced delays)              */

		if (whole) {
			first = TRUE;
			for (i = MINPTR; i <= maxptr; i++) {
				m = (int)PDELAY[i];
				frac = PDELAY[i] - m;
				if (fabs(frac) < 1.e-4) {
					lag = start - m;
					g[i] =
					    pgain(&v0[lag - 1], l, first, m,
						  LEN, &match[i]);
					first = FALSE;
				} else
					match[i] = 0.0;
			}
		}

		/* *find gain and match score for fractional delays                  */
		/* *(beware of combining this loop with above loop!)                 */
		/* *(could use end-point correction on unity spaced delays)          */

		if (fraction) {
			for (i = MINPTR; i <= maxptr; i++) {
				m = (int)PDELAY[i];
				frac = PDELAY[i] - m;
				if (fabs(frac) >= 1.e-4) {
					delay(v0, start, l, frac, m, v0shift);
					g[i] =
					    pgain(v0shift, l, TRUE, 70, LEN,
						  &match[i]);
				}
			}
		}

		/* *find pointer to top (MSPE) match score (topptr)                  */
		/* *search for best match score (max -error term)                    */

		topptr = MINPTR;
		emax = match[topptr];
		for (i = MINPTR; i <= maxptr; i++) {
			if (match[i] > emax) {
				topptr = i;
				emax = match[topptr];
			}
		}

		/* *for full search (odd) subframes:                                 */
		/* *select shortest delay of 2, 3, or 4 submultiples. if its match   */
		/* *is within 1 dB of MSPE to favor smooth "pitch"                   */

		TAUPTR = topptr;
		if (sub) {
			if ((NSEG % 2) != 0) {

				/* *for each submultiple {2, 3 & 4}                                */

				for (i = 1; i <= submult[topptr][0]; i++) {

					/* *find best neighborhood match for given submultiple         */

					bigptr = submult[topptr][i];
					for (subptr =
					     (mmax(submult[topptr][i] - 8,
						  MINPTR));
					     subptr <=
					     mmin(submult[topptr][i] + 8,
						  maxptr); subptr++) {
						if (match[subptr] >
						    match[bigptr])
							bigptr = subptr;
					}

					/* *select submultiple match if within 1 dB MSPE match         */

					if (match[bigptr] >=
					    0.88 * match[topptr]) {
						TAUPTR = bigptr;
					}
				}
			}
		}

		/* *search TAUPTR's neighboring delays                               */
		/* *(to be used with earlier stages of searching)                    */
		/* *find gain and match score for neighboring delays                 */
		/* *and find best neighborhood match                                 */
		/* *(could use end-point correction on unity spaced delays)          */

		if (neigh) {
			bigptr = TAUPTR;
			for (i = (mmax(TAUPTR - nrange, MINPTR));
			     i <= mmin(TAUPTR + nrange, maxptr); i++) {
				if (i != TAUPTR) {
					m = (int)PDELAY[i];
					frac = PDELAY[i] - m;
					lag = start - m;
					if (fabs(frac) < 1.e-4)
						g[i] =
						    pgain(&v0[lag - 1], l, TRUE,
							  m, LEN, &match[i]);
					else {
						delay(v0, start, l, frac, m,
						      v0shift);
						g[i] =
						    pgain(v0shift, l, TRUE, 70,
							  LEN, &match[i]);
					}
					if (match[i] > match[TAUPTR])
						bigptr = i;
				}
			}
			TAUPTR = bigptr;
		}

		/* *OPTIONAL (may be useful for integer DSPs)                        */
		/* *given chosen pointer to delay (TAUPTR), recompute its            */
		/* *gain to correct errors accumulated in recursions                 */
		/* *and errors due to truncation                                     */

		m = (int)PDELAY[TAUPTR];
		frac = PDELAY[TAUPTR] - m;
		lag = start - m;
		if (fabs(frac) < 1.e-4)

			/* *integer delay:                                                   */

			g[TAUPTR] =
			    pgain(&v0[lag - 1], l, TRUE, m, l, &match[TAUPTR]);
		else
			/* *fractional delay:                                                */

		{
			delay(v0, start, l, frac, m, v0shift);
			g[TAUPTR] =
			    pgain(v0shift, l, TRUE, 70, l, &match[TAUPTR]);
		}

		/* *place pitch parameters in common BB "structure"                  */

		BB[2] = g[TAUPTR];
		BB[0] = PDELAY[TAUPTR];

		/* *save pitch pointer to determine delta delay                      */

		oldptr = TAUPTR;
	}

	/* *pitch quantization BB[2]                                           */

	if (strncmp(PTYPE, "NOne", 4) != 0)
		BB[2] = pitchencode(BB[2], &PINDEX);
	else {
#ifdef CELPDIAG
		fprintf(stderr, "psearch: NO pitch quantization!\n");
#endif
	}
}

#undef LEN
#undef MAXBUFPTR
