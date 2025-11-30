/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*                                                                         *
*	CELP Voice Coder with State Management                              *
*	Version 3.2c	                                                  *
*                                                                         *
***************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "ccsub.h"
#include "ophtools.h"

#define PROTECT

#define TRUE		1
#define FALSE		0

#include "celplib.h"
#include "celp.h"

#define STREAMBITS	144
#define CODELENGTH1	15
#define CODELENGTH2	11
#define PARITYLENGTH	(CODELENGTH1 - CODELENGTH2)
#define SYNDRUN		100
#define OMEGA		0.994127	/* Bandwidth expansion for LPC analysis (15 Hz) */
#define ALPHA		0.8	/* Bandwidth expansion for postfilter */
#define BETA		0.5	/* Bandwidth expansion for postfilter */

#define mmax(A,B)        ((A)>(B)?(A):(B))
#define mmin(A,B)        ((A)<(B)?(A):(B))


/* Function to create a new codec state */
celp_state_t* celp_create_state(void)
{
    celp_state_t* state = (celp_state_t*)calloc(1, sizeof(celp_state_t));
    if (!state) return NULL;
    
    /* Initialize constant values */
    state->cbgbits = 5;
    state->ncsize = 512;
    state->no = 10;
    state->mxsw = 1;
    state->gamma2 = 0.8f;
    state->prewt = 0.0f;
    state->scale = 1.0;
    state->ll = 240;
    state->lp = 60;
    state->l = 60;
    state->alpha = 0.8;
    state->beta = 0.5;
    state->ptype = "max2";
    state->cbgtype = "log";
    state->pstype = "hier";
    state->cbbits = 9;
    
    // Initialize filter coefficients
    state->ahpf[0] = 0.946;
    state->ahpf[1] = -1.892;
    state->ahpf[2] = 0.946;
    state->bhpf[0] = 1.0;
    state->bhpf[1] = -1.889033;
    state->bhpf[2] = 0.8948743;
    
    return state;
}

/* Function to destroy codec state */
void celp_destroy_state(celp_state_t* state)
{
    if (state) {
        free(state);
    }
}

/* Function to initialize codec state */
void celp_init_state(celp_state_t* state, int prot)
{
    if (!state) return;

#ifdef PROTECT
    state->protect = prot;		/* Set bit error recovery mode */
#endif

    /* Number of codewords/LPC frame */
    state->nn = state->ll / state->l;

    /* Dimension of d1a and d1b */
    state->idb = MMAX + MAXNP - 1 + state->l;
    state->plevel1 = 1 << (8); // pbits[0] = 8

    /* Levels of delta tau */
    state->plevel2 = 1 << (6); // pbits[1] = 6

    /* number of bits per subframe */
    state->bitsum1 = state->cbbits + state->cbgbits + 8 + 5; // cbbits + cbgbits + pbits[0] + pbits[2]
    state->bitsum2 = state->cbbits + state->cbgbits + 6 + 5; // cbbits + cbgbits + pbits[1] + pbits[2]

#ifdef PROTECT
    state->twoerror = FALSE;
    state->snrflag = FALSE;
    state->lspflag = TRUE;
#endif

    /* Initialise arrays */
    for (int i = 0; i < MAXLP; i++) {
        state->h[i] = state->e0[i] = 0.0;
    }
    for (int i = 0; i < MAXLL; i++) {
        state->sold[i] = 0.0;
    }
    for (int i = 0; i < STREAMBITS; i++) {
        state->stream[i] = state->savestream[i] = 0;
    }

    /* Start nseg at 0 to do pitch on odd segments.
       (nseg is incremented before csub). */
    state->nseg = 0;

    /* Generate matrix for error control coding */
#ifdef PROTECT
    matrixgen(CODELENGTH1, CODELENGTH2, state->hmatrix, state->syndrometable);
#endif

    /* Generate Hamming windows */
    ham(state->hamw, state->ll);
}

/* Function to encode with state */
int celp_encode_state(celp_state_t* state, short iarf[240], char packedbits[STREAMBITS / 8])
{
    if (!state) return CELP_OK;
    
    int i, pointer;
    int i1, i2, i3;

    state->frame++;
    pointer = 0;
    state->celp_error = CELP_OK;

    /*  Scale and convert to real speech.
        The ssub buffer used for subframe CELP analysis is 1/2 a
        frame behind the snew buffer and 1/2 a frame ahead of the 
        sold buffer.  */
    for (i = 0; i < state->ll; i++) {
        state->snew[i] = mmax(-32768., mmin(state->scale * iarf[i], 32767.));
    }

    /* High pass filter snew. */
    zerofilt(state->ahpf, 2, state->dhpf1, state->snew, state->ll);
    polefilt(state->bhpf, 2, state->dhpf2, state->snew, state->ll);
    if (state->celp_error) {
        return state->celp_error;
    }

    /* Make ssub vector from snew and sold. */
    for (i = 0; i < state->ll / 2; i++) {
        state->ssub[i] = state->sold[i + state->ll / 2];
        state->ssub[i + state->ll / 2] = state->snew[i];
    }

    autohf(state->snew, state->hamw, state->ll, state->no, OMEGA, state->fcn, state->rcn);

    /* Pc -> lsp (new). */
    pctolsp2(state->fcn, state->no, state->newfreq, &state->lspflag);
    if (state->lspflag) {
#ifdef CELPDIAG
        fprintf(stderr, "celp: Bad \\\\\"new\\\" lsp at frame: %d\\n", state->frame);
        fprintf(stderr, "lsp: ");
        for (i = 0; i < state->no; i++) {
            fprintf(stderr, "%9.5f", state->newfreq[i]);
        }
        fprintf(stderr, "\\npc: ");
        for (i = 0; i < state->no + 1; i++) {
            fprintf(stderr, "%9.5f", state->fcn[i]);
        }
        fprintf(stderr, "\\nrc: ");
        for (i = 0; i < state->no; i++) {
            fprintf(stderr, "%9.5f", state->rcn[i]);
        }
        fprintf(stderr, "\\n");
#endif
        return CELP_LSPERR;
    }

    /* Save unquantized lsp. */
    for (i = 0; i < state->no; i++) {
        state->unqfreq[i] = state->newfreq[i];
    }

    /* Quantize lsp's. */
    lsp34(state->newfreq, state->no, (const int[]){ 3, 4, 4, 4, 4, 3, 3, 3, 3, 3 }, state->findex);

    /* Pack lsp indices in bit stream array. */
    for (i = 0; i < state->no; i++) {
        pack(state->findex[i], (const int[]){ 3, 4, 4, 4, 4, 3, 3, 3, 3, 3 }[i], state->stream, &pointer);
    }

    /* Linearly interpolate LSP's for each subframe. */
    intanaly(state->newfreq, state->nn, state->lsp);

    /* For each subframe, search stochastic & adaptive code books. */
    int k = 0;
    for (int i = 0; i < state->nn; i++) {
        lsptopc(&state->lsp[i][0], state->fci);
        for (int j = 0; j < state->no + 1; j++) {
            state->fc[j] = state->fci[j];
        }
        state->nseg++;

        /* Code book & pitch searches. */
        csub(&state->ssub[k], &state->v[k], state->l, state->lp);

        /* Pitch quantization tau. */

        /* Pack parameter indices in bit stream array. */
        if (((i + 1) % 2) != 0) {
            packtau(state->tauptr - state->minptr, 8, (const int*)state->pdencode, state->stream,
                    &pointer);  // pbits[0] = 8
        } else {
            pack(state->tauptr - state->minptr, 6, state->stream, &pointer);  // pbits[1] = 6
        }

        pack(state->pindex, 5, state->stream, &pointer);  // pbits[2] = 5
        state->cbindex--;
        pack(state->cbindex, state->cbbits, state->stream, &pointer);
        pack(state->gindex, state->cbgbits, state->stream, &pointer);

        /* Decode parameters for analysis by synthesis. */
        state->cbindex++;

        k += state->l;
    }

    /* Bit error protection
       Extract bits to protect from stream array. */
#ifdef PROTECT
    if (state->protect) {
        for (i = 0; i < CODELENGTH2; i++) {
            state->codeword[i] = state->stream[(const int[]){ 1, 3, 4, 5, 7, 8, 9, 10, 12, 13, 14, 15, 17, 18, 19 }[i] - 1]; // bitprotect array values
        }

        /* Hamming encode. */
        encodeham(CODELENGTH1, CODELENGTH2, state->hmatrix, &state->paritybit,
                  state->codeword);

        /* Pack future bit. */
        pack(0, 1, state->stream, &pointer);

        /* Pack parity bits. */
        for (i = 0; i < PARITYLENGTH; i++) {
            pack(state->codeword[CODELENGTH2 + i], 1, state->stream, &pointer);
        }

        /* Toggle and pack the sync bit. */
        state->syncBit = state->syncBit ^ 1;
        pack(state->syncBit, 1, state->stream, &pointer);
    }
#endif

    /*  At this time \\\"stream\\\" contains the CELP encoded bitstream.  The
        stream array consists of one bit per int element.  */
    i2 = 0x80;
    i3 = 0;
    memzero(packedbits, STREAMBITS / 8);
    for (i1 = 0; i1 < STREAMBITS; i1++) {
        packedbits[i3] |= (state->stream[i1] ? i2 : 0);
        i2 >>= 1;
        if (i2 == 0) {
            i2 = 0x80;
            i3++;
        }
    }

    /* Shift new speech buffer into old speech buffer

        sold                snew
        |-------------------|-------------------| snew
        |-------------------|
        ssub                                      */
    for (i = 0; i < state->ll; i++) {
        state->sold[i] = state->snew[i];
    }
    return state->celp_error;
}

/* Function to decode with state */
int celp_decode_state(celp_state_t* state, char packedbits[STREAMBITS / 8], short pf[240])
{
    if (!state) return CELP_OK;
    
    int i1, i2, i3;

    i2 = 0x80;
    i3 = 0;
    for (i1 = 0; i1 < STREAMBITS; i1++) {
        state->stream[i1] = (packedbits[i3] & i2) ? 1 : 0;
        i2 >>= 1;
        if (i2 == 0) {
            i2 = 0x80;
            i3++;
        }
    }

    state->frame++;
    int pointer = -1;
    state->celp_error = CELP_OK;

    /* Unpack parity bits. */
#ifdef PROTECT
    if (state->protect) {
        pointer = pointer - PARITYLENGTH - 2;

        pointer = 138;

        for (int i = 0; i < PARITYLENGTH; i++) {
            unpack(state->stream, 1, &state->codeword[CODELENGTH2 + i], &pointer);
        }

        /* Extract code word from stream array. */
        for (int i = 0; i < CODELENGTH2; i++) {
            state->codeword[i] = state->stream[(const int[]){ 1, 3, 4, 5, 7, 8, 9, 10, 12, 13, 14, 15, 17, 18, 19 }[i] - 1]; // bitprotect array values
        }

        /* Repack Bisnu bit (remains constant for now). */
        state->codeword[10] = 0;

        /* Hamming decode. */
        decodeham(CODELENGTH1, state->hmatrix, state->syndrometable, state->paritybit,
                  state->codeword, &state->twoerror, &state->syndrome);

        /* Disable parity check (if parity not used). */
        state->twoerror = FALSE;

        /* Bit error rate estimator (running avg of bad syndromes). */
        if (state->syndrome != 0) {
            state->syndrome = 1;
        }
        state->syndavg =
            (1.0 - (1.0 / SYNDRUN)) * state->syndavg +
            (1.0 / SYNDRUN) * (float)state->syndrome;

        /* Repack protected bits. */
        for (int i = 0; i < CODELENGTH2; i++) {
            state->stream[(const int[]){ 1, 3, 4, 5, 7, 8, 9, 10, 12, 13, 14, 15, 17, 18, 19 }[i] - 1] = state->codeword[i]; // bitprotect array values
        }

        /* Frame repeat if two errors detected in code word. */
        if (state->twoerror) {
#ifdef CELPDIAG
            fprintf(stderr,
                    "celp: two errors have occured in frame %d\\n",
                    state->frame);
#endif
            return CELP_TWOERR;
        }
    }
#endif		/* PROTECT */

    pointer = -1;

    /* Unpack data stream. */
    for (int i = 0; i < state->no; i++) {
        unpack(state->stream, (const int[]){ 3, 4, 4, 4, 4, 3, 3, 3, 3, 3 }[i], &state->findex[i], &pointer);
    }

    /* Decode lsp's. */
    lspdecode34(state->findex, state->no, state->newfreq);

    /* Interpolate spectrum lsp's for nn subframes. */
    intsynth(state->newfreq, state->nn, state->lsp, state->twoerror, state->syndavg);

    /* Decode all code book and pitch parameters. */
    state->bitpointer = pointer;
    dcodtau(8, 6, state->bitsum1, state->bitsum2, &state->bitpointer, state->nn, state->stream,
            state->pddecode, state->pdtabi, state->taus);  // pbits[0] = 8, pbits[1] = 6
    dcodpg(5, state->bitsum1, state->bitsum2, &state->bitpointer, state->nn, state->stream, state->pgs);  // pbits[2] = 5
    if (state->celp_error) {
        return state->celp_error;
    }
    dcodcbi(state->cbbits, state->bitsum1, state->bitsum2, &state->bitpointer, state->nn, state->stream, state->cbi);
    if (state->celp_error) {
        return state->celp_error;
    }
    dcodcbg(state->cbgbits, state->bitsum1, state->bitsum2, &state->bitpointer, state->nn, state->stream, state->cbg);
    if (state->celp_error) {
        return state->celp_error;
    }

    /* *** synthesize each subframe                                      */
    state->nseg -= state->nn;

    int k = 0;
    for (int i = 0; i < state->nn; i++) {
        state->nseg++;

        /* Decode values for subframe. */
        state->cbindex = state->cbi[i];
        state->decodedgain = state->cbg[i];
#ifdef PROTECT
        if (state->protect) {
            smoothcbgain(&state->decodedgain, state->twoerror, state->syndavg, state->cbg,
                         i + 1);
        }
#endif

        /* Code book synthesis. */
        vdecode(state->decodedgain, state->l, &state->vdecoded[k]);

#ifdef PROTECT
        if (state->protect) {
            smoothtau(&state->taus[i], state->twoerror, state->syndavg, state->taus[2], i + 1);
        }
#endif
        state->bb[0] = state->taus[i];
        state->bb[2] = state->pgs[i];
#ifdef PROTECT
        if (state->protect) {
            smoothpgain(&state->bb[2], state->twoerror, state->syndavg, state->pgs, i + 1);
        }
#endif

        /* Pitch synthesis. */
        pitchvq(&state->vdecoded[k], state->l, state->dps, state->idb, state->bb, "long");

        /* Pitch pre-filter (synthesis). */
        if (state->prewt != 0.0) {
            prefilt(&state->vdecoded[k], state->l, state->dpps);
        }

        /* Convert lsp's to pc's. */
        lsptopc(&state->lsp[i][0], state->fci);

        /* LPC synthesis. */
        polefilt(state->fci, state->no, state->dss, &state->vdecoded[k], state->l);
        if (state->celp_error) {
            return state->celp_error;
        }

        /* Post-filtering */
        postfilt(&state->vdecoded[k], state->l, state->alpha, state->beta, &state->ip, &state->op, state->dp1, state->dp2, state->dp3);

        /* Test for output speech clipping. */
        while (clip(&state->vdecoded[k], state->l)) {

            /* Frame repeat & recall synthesizer?
               or scale vdecoded? */
#ifdef CELPDIAG
            fprintf(stderr, "celp: Clipping detected at frame %d\\n",
                    state->frame);
#endif
            for (int j = 0; j < state->l; j++) {
                state->vdecoded[k + j] = 0.05 * state->vdecoded[k + j];
            }
        }

        /* Write postfiltered output speech disk files. */
        for (int j = 0; j < state->l; j++) {
            pf[k + j] =
                round(mmax
                      (-32768.0, mmin(32767.0, state->vdecoded[k + j])));
        }

        k += state->l;
    }
    return state->celp_error;
}

/* Legacy functions for backward compatibility */
static celp_state_t global_state_instance;
static celp_state_t* global_state = &global_state_instance;

void celp_init(int prot)
{
    celp_init_state(global_state, prot);
}

int celp_encode(short iarf[240], char packedbits[144 / 8])
{
    return celp_encode_state(global_state, iarf, packedbits);
}

int celp_decode(char packedbits[144 / 8], short pf[240])
{
    return celp_decode_state(global_state, packedbits, pf);
}