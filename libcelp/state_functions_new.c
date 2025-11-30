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

/* External functions from celp.c */
extern void celp_init(int prot);
extern int celp_encode(short iarf[240], char packedbits[144 / 8]);
extern int celp_decode(char packedbits[144 / 8], short pf[240]);

/* Global state for backward compatibility */
extern celp_state_t global_state_instance;

/* Function to save state from global variables to state structure */
static void save_state_to_global(celp_state_t* state) {
    // This would require access to all global variables in celp.c
    // For now, we'll implement a simpler approach
}

/* Function to restore state from state structure to global variables */
static void restore_state_from_global(celp_state_t* state) {
    // This would require access to all global variables in celp.c
    // For now, we'll implement a simpler approach
}

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

    /* Use the global celp_init function which initializes global variables */
    celp_init(prot);
    
    // Store the protection setting in the state
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
    // We would need to call matrixgen here with state values
    // matrixgen(CODELENGTH1, CODELENGTH2, state->hmatrix, state->syndrometable);
#endif

    /* Generate Hamming windows */
    // We would need to call ham here with state values
    // ham(state->hamw, state->ll);
}

/* Function to encode with state */
int celp_encode_state(celp_state_t* state, short iarf[240], char packedbits[STREAMBITS / 8])
{
    if (!state) return CELP_OK;
    
    // For now, just call the global function
    // In a complete implementation, we would save/restore state
    return celp_encode(iarf, packedbits);
}

/* Function to decode with state */
int celp_decode_state(celp_state_t* state, char packedbits[STREAMBITS / 8], short pf[240])
{
    if (!state) return CELP_OK;
    
    // For now, just call the global function
    // In a complete implementation, we would save/restore state
    return celp_decode(packedbits, pf);
}