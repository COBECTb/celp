/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#include <stdio.h>
#include "context.h"

/* Global context for backward compatibility */
celp_state_t global_celp_state;

/* Flag to determine which context to use */
int use_state_context = 0;

/* Current context pointer */
static celp_state_t* current_context = NULL;

/* Function to set the current context */
void set_celp_context(celp_state_t* state)
{
    current_context = state;
    if (state) {
        use_state_context = 1;
    } else {
        use_state_context = 0;
    }
}

/* Function to get the current context */
celp_state_t* get_celp_context(void)
{
    if (current_context) {
        return current_context;
    } else {
        return &global_celp_state;
    }
}