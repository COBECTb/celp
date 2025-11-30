/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		dcodtau
*
* FUNCTION
*		 Decodes all pitch tau values for a frame
*
*
* SYNOPSIS
*		subroutine dcodtau(taubits,taudelta,bitsum1,bitsum2,bitpointer,
*				   nn,stream,pddecode,pdtabi,taus)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	taubits 	int	i	number of bits for quantization
*	taudelta	int	i	number of bits for even frame delta
*	bitsum1 	int	i	number of bits for odd subframes
*	bitsum2 	int	i	number of bits for even subframes
*	bitpointer	int	i/o	number of bits used
*	nn		int	i	number of subframes/frame
*	stream		short	i	bit stream
*	pddecode	real	i	pitch delay permutation table
*	pdtabi		int	i	pitch delay table index
*       taus            float   o       vector of pitch delays "taus"
*
*   external
*
*	name		type	type	function
*	-------------------------------------------------------------------
*
*	PLEVEL1 	int		number of full search pitch delays
*	PLEVEL2 	int		number of delta search pitch delays
*	PDELAY[]	float		pitch delay coding table
*
***************************************************************************
*
* DESCRIPTION
*
*		This routine converts the pitch delay bits of the 
*	bitstream to actual delay values.  These delays are output
*	to the main routine in taus(i), one delay for each of four
*	subframes.
*
***************************************************************************
*
* CALLED BY
*
*	celp
*
* CALLS
*
*	unpack
*
***************************************************************************
*
* REFERENCES
*
*
**************************************************************************/

static void dcodtau(int taubits, int taudelta, int bitsum1,
		    int bitsum2, int *bitpointer, int nn, short stream[],
		    const float pddecode[], int pdtabi[], float taus[])
{
	int i, pointer, tptr, mxptr, mnptr;
	static int lptr = 0;

	pointer = *bitpointer;
	for (i = 0; i < nn; i++) {
		if (((i + 1) % 2) != 0) {
			unpack(stream, taubits, &tptr, &pointer);
			taus[i] = pddecode[tptr];
			pointer += bitsum1 - taubits;
		} else {
			unpack(stream, taudelta, &tptr, &pointer);
			pointer += bitsum2 - taudelta;
			mnptr = lptr - (PLEVEL2 / 2 - 1);
			mxptr = lptr + (PLEVEL2 / 2);
			if (mnptr < 0)
				mnptr = 0;
			if (mxptr > PLEVEL1 - 1)
				mnptr = PLEVEL1 - PLEVEL2;
			taus[i] = PDELAY[tptr + mnptr];
		}
		lptr = pdtabi[tptr];
	}
	*bitpointer += taubits;
}
