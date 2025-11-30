/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*

    Definitions for callers of the CELP Codec
    
*/

#ifndef __CELP_H__
#define __CELP_H__

/* Define maximum values */
#define MAXNCSIZE 512
#define MAXL      60
#define MAXCODE   2*(MAXNCSIZE)+MAXL /* shift by 2 overlapped code book */
#define MAXLL     240
#define MAXNO     10
#define MAXLP     60         /* bug, fixed - See MAXPA */
#define MAXNP     3
#define MAXPD     256
#define MAXM2     20
#define MMAX      147
#define MAXPA     MAXLP+MMAX+2+MAXM2

/* Define stream and coding constants */
#define STREAMBITS	144
#define CODELENGTH1	15
#define CODELENGTH2	11
#define PARITYLENGTH	(CODELENGTH1 - CODELENGTH2)
#define SYNDRUN		100

#define CELP_OK 0		/* Decode OK */
#define CELP_TWOERR 1		/* Uncorrectable error in stream bits */
#define CELP_LSPERR 2		/* Error encoding LSPs */
#define CELP_ERR_DCODCBG 3	/* Error decoding cbgain */
#define CELP_ERR_DCODCBI 4	/* Error decoding code book index */
#define CELP_ERR_DELAY	 5	/* Invalid pitch delay */
#define CELP_ERR_GAINDECODE 6	/* Unquantised cbgain in gaincode.c */
#define CELP_ERR_POLEFILT 7	/* Bad coefficients in polefilt.c */
#define CELP_ERR_IMPULSE_LENGTH 8	/* Impulse response too long in psearch.c */
#define CELP_ERR_MAXLP 9	/* MAXLP < MAXL in psearch.c */
#define CELP_ERR_PITCH_TYPE 10	/* Bad pitch search type in psearch.c */
#define CELP_ERR_DCODPG 11	/* Error decoding pitch gain in dcodpg.c */

/* Structure to hold codec state */
typedef struct {
    /* State variables */
    int cbindex, gindex, idb;
    int nseg, pindex, frame, tauptr, minptr, plevel1, plevel2;
    int mxsw;
    
    /* Buffers for audio processing */
    float bb[MAXNP + 1], e0[MAXLP];
    float fc[MAXNO + 1], fcn[MAXNO + 1], fci[MAXNO + 1];
    float h[MAXLP];
    float pdelay[MAXPD];
    float x[MAXCODE];
    int pdencode[MAXPD];
    float pddecode[MAXPD];
    
    /* Pitch delay coding tables */
    int pdtabi[MAXPD];
    
    int cbi[MAXLL / MAXL];
    int findex[MAXNO];
    int lspflag;
    
    float cbg[MAXLL / MAXL], pgs[MAXLL / MAXL];
    float sold[MAXLL], snew[MAXLL], ssub[MAXLL], v[MAXLL];
    float vdecoded[MAXLL], rcn[MAXNO], hamw[MAXLL];
    float dps[MAXPA], newfreq[MAXNO], unqfreq[MAXNO],
        lsp[MAXLL / MAXL][MAXNO];
    float dpps[MAXPA];
    float decodedgain, taus[4];
    
    /* Filter coefficients for 2nd order Butterworth 100 Hz HPF */
    float ahpf[3], bhpf[3];
    
    /* Bit stream */
    short stream[STREAMBITS], savestream[STREAMBITS];
    int pointer, bitpointer, bitsum1, bitsum2;
    
    /* Filter memories */
    float dhpf1[3], dhpf2[3], dss[MAXNO + 1];
    float dp1[MAXNO + 1], dp2[MAXNO + 1], dp3[2];
    float ip, op;
    
    /* Error control coding parameters */
    float syndavg;
    int twoerror;
    
#ifdef PROTECT
    int snrflag;
    int syncBit;
    int codeword[CODELENGTH1], hmatrix[CODELENGTH1];
    int syndrometable[CODELENGTH1], paritybit, protect;
    int syndrome;
    
    /* Bit protection vector */
    int bitprotect[CODELENGTH2];
#endif
    
    int celp_error;		/* Error in encoding or decoding */
    
    /* Configuration parameters */
    int nn;  /* Number of subframes per frame */
    int ll, lp, l;
    float scale;
    const char *ptype, *cbgtype, *pstype;
    int cbbits, cbgbits, ncsize, no;
    const int *pbits;
    const int *sbits;
    float gamma2, prewt, alpha, beta;
} celp_state_t;

/* Functions for codec state management */
extern celp_state_t* celp_create_state(void);
extern void celp_destroy_state(celp_state_t* state);
extern void celp_init_state(celp_state_t* state, int prot);

/* Functions for encoding and decoding */
extern int celp_encode_state(celp_state_t* state, short iarf[240], char packedbits[144 / 8]);
extern int celp_decode_state(celp_state_t* state, char packedbits[144 / 8], short pf[240]);

/* Legacy functions (for backward compatibility) */
extern void celp_init(int prot);
extern int celp_encode(short iarf[240], char packedbits[144 / 8]);
extern int celp_decode(char packedbits[144 / 8], short pf[240]);

#endif
