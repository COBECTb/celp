/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* Data initialization functions */

/* Include the data files to get the values */
#include "pdelay.h"
#include "codebook.h"
#include "pdencode.h"
#include "pddecode.h"
#include "bitprot.h"

/* Function to initialize data arrays */
static void init_data_arrays(celp_state_t* state) {
    /* Initialize pdelay array */
    float pdelay_data[] = {
#include "pdelay.h"
    };
    for (int i = 0; i < MAXPD; i++) {
        state->pdelay[i] = pdelay_data[i];
    }
    
    /* Initialize x array (codebook) */
    float x_data[] = {
#include "codebook.h"
    };
    for (int i = 0; i < MAXCODE; i++) {
        state->x[i] = x_data[i];
    }
    
    /* Initialize pdencode array */
    int pdencode_data[] = {
#include "pdencode.h"
    };
    for (int i = 0; i < MAXPD; i++) {
        state->pdencode[i] = pdencode_data[i];
    }
    
    /* Initialize pddecode array */
    float pddecode_data[] = {
#include "pddecode.h"
    };
    for (int i = 0; i < MAXPD; i++) {
        state->pddecode[i] = pddecode_data[i];
    }
    
    /* Initialize bitprotect array */
    int bitprotect_data[] = {
#include "bitprot.h"
    };
    for (int i = 0; i < CODELENGTH2; i++) {
        state->bitprotect[i] = bitprotect_data[i];
    }
}