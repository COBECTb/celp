/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#ifndef CONTEXT_H
#define CONTEXT_H

#include "celp.h"

/* Global context for backward compatibility */
extern celp_state_t global_celp_state;

/* Flag to determine which context to use */
extern int use_state_context;

/* Function to set the current context */
void set_celp_context(celp_state_t* state);

/* Function to get the current context */
celp_state_t* get_celp_context(void);

/* Helper macros to access state variables */
#define CBINDEX (get_celp_context()->cbindex)
#define GINDEX (get_celp_context()->gindex)
#define IDB (get_celp_context()->idb)
#define NSEG (get_celp_context()->nseg)
#define PINDEX (get_celp_context()->pindex)
#define FRAME (get_celp_context()->frame)
#define TAUPTR (get_celp_context()->tauptr)
#define MINPTR (get_celp_context()->minptr)
#define PLEVEL1 (get_celp_context()->plevel1)
#define PLEVEL2 (get_celp_context()->plevel2)
#define MXPW (get_celp_context()->mxsw)

#define BB (get_celp_context()->bb)
#define E0 (get_celp_context()->e0)
#define FC (get_celp_context()->fc)
#define FCN (get_celp_context()->fcn)
#define FCI (get_celp_context()->fci)
#define H (get_celp_context()->h)
#define PDELAY (get_celp_context()->pdelay)
#define X (get_celp_context()->x)
#define PDENCODE (get_celp_context()->pdencode)
#define PDDECODE (get_celp_context()->pddecode)

#define PDTABI (get_celp_context()->pdtabi)
#define CBI (get_celp_context()->cbi)
#define FINDEX (get_celp_context()->findex)
#define LSPFLAG (get_celp_context()->lspflag)

#define CBG (get_celp_context()->cbg)
#define PGS (get_celp_context()->pgs)
#define SOLD (get_celp_context()->sold)
#define SNEW (get_celp_context()->snew)
#define SSUB (get_celp_context()->ssub)
#define V (get_celp_context()->v)
#define VDECODED (get_celp_context()->vdecoded)
#define RCN (get_celp_context()->rcn)
#define HAMW (get_celp_context()->hamw)
#define DPS (get_celp_context()->dps)
#define NEWFREQ (get_celp_context()->newfreq)
#define UNQFREQ (get_celp_context()->unqfreq)
#define LSP (get_celp_context()->lsp)
#define DPPS (get_celp_context()->dpps)
#define DECODEDGAIN (get_celp_context()->decodedgain)
#define TAUS (get_celp_context()->taus)

#define AHPF (get_celp_context()->ahpf)
#define BHPF (get_celp_context()->bhpf)

#define STREAM (get_celp_context()->stream)
#define SAVESTREAM (get_celp_context()->savestream)
#define POINTER (get_celp_context()->pointer)
#define BITPOINTER (get_celp_context()->bitpointer)
#define BITSUM1 (get_celp_context()->bitsum1)
#define BITSUM2 (get_celp_context()->bitsum2)

#define DHPF1 (get_celp_context()->dhpf1)
#define DHPF2 (get_celp_context()->dhpf2)
#define DSS (get_celp_context()->dss)
#define DP1 (get_celp_context()->dp1)
#define DP2 (get_celp_context()->dp2)
#define DP3 (get_celp_context()->dp3)
#define IP (get_celp_context()->ip)
#define OP (get_celp_context()->op)

#define SYNDAVG (get_celp_context()->syndavg)
#define TWOERROR (get_celp_context()->twoerror)

#ifdef PROTECT
#define SNRFLAG (get_celp_context()->snrflag)
#define SYNCBIT (get_celp_context()->syncBit)
#define CODEWORD (get_celp_context()->codeword)
#define HMATRIX (get_celp_context()->hmatrix)
#define SYNDROMETABLE (get_celp_context()->syndrometable)
#define PARITYBIT (get_celp_context()->paritybit)
#define PROTECT_VAR (get_celp_context()->protect)
#define SYNDROME (get_celp_context()->syndrome)
#define BITPROTECT (get_celp_context()->bitprotect)
#endif

#define CELP_ERROR_VAR (get_celp_context()->celp_error)

#define LL (get_celp_context()->ll)
#define LP (get_celp_context()->lp)
#define L (get_celp_context()->l)
#define SCALE (get_celp_context()->scale)
#define PTYPE (get_celp_context()->ptype)
#define CBGTYPE (get_celp_context()->cbgtype)
#define PSTYPE (get_celp_context()->pstype)
#define CBBITS (get_celp_context()->cbbits)
#define CBGBITS (get_celp_context()->cbgbits)
#define NCSIZE (get_celp_context()->ncsize)
#define NO (get_celp_context()->no)
#define GAMMA2 (get_celp_context()->gamma2)
#define PREWT (get_celp_context()->prewt)
#define ALPHA (get_celp_context()->alpha)
#define BETA (get_celp_context()->beta)

#endif /* CONTEXT_H */