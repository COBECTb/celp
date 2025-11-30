/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		impulse
*
* FUNCTION
*		compute impulse response with direct form filter
*		exclusive of adaptive code book contribution
*
* SYNOPSIS
*		subroutine impulse(l)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	l		int	i	impulse response length
*
*   external
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	NO		int	i
*	FC[]		float	i
*	H[]		float	i/o
*	GAMMA2		float	i
*
***************************************************************************
*
* Global Variables
*
*
*	SPECTRUM VARIABLE:
*	d5	real	auxiliary array
*
****************************************************************************
* CALLED BY
*
*	csub
*
* CALLS
*
*	bwexp	setr	polefilt
*
**************************************************************************/

static void impulse(int l)
{
	float d5[MAXNO + 1], fctemp[MAXNO + 1];

	setr(l, 0.0, H);
	H[0] = 1.0;
	setr(NO + 1, 0.0, d5);
	bwexp(GAMMA2, FC, fctemp, NO);
	polefilt(fctemp, NO, d5, H, l);

}
