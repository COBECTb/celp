//
//  main.c
//  melpe
//
//  Created by Aleksandr Strokov on 30.07.17.
//  Copyright © 2017 Aleksandr Strokov. All rights reserved.
//

#include "libcelp/celp.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


//#define DEBUG_MELP

#ifdef DEBUG_MELP
#define FSpeed(code, text) \
{ \
struct timeval FSpeed_MACRO_tmv; \
gettimeofday(&FSpeed_MACRO_tmv, 0); \
long FSpeed_MACRO_sec = FSpeed_MACRO_tmv.tv_sec; \
long FSpeed_MACRO_usec = FSpeed_MACRO_tmv.tv_usec; \
code; \
gettimeofday(&FSpeed_MACRO_tmv, 0); \
FSpeed_MACRO_sec = FSpeed_MACRO_tmv.tv_sec - FSpeed_MACRO_sec; \
if(FSpeed_MACRO_usec > FSpeed_MACRO_tmv.tv_usec) \
{ \
FSpeed_MACRO_sec--; \
FSpeed_MACRO_usec = 1000000 - FSpeed_MACRO_usec + FSpeed_MACRO_tmv.tv_usec; \
} \
else FSpeed_MACRO_usec = FSpeed_MACRO_tmv.tv_usec - FSpeed_MACRO_usec; \
fprintf(stderr,"pid: %i time frame %u.%u\n",text,FSpeed_MACRO_sec,FSpeed_MACRO_usec); \
}
#else
#define FSpeed(code, text) code;
#endif
int main(int argc, char **argv)
{
	if(argc<4)
	{
		fprintf(stderr,"Ussage: celp <mode> infile outfile");
		exit(1);
	}
	
	FILE *fp_in, *fp_out;
	if(argv[2][0]!='-')
	{
		/* Open input, output, and parameter files */
		if (( fp_in = fopen(argv[2],"rb")) == NULL )
		{
			fprintf(stderr,"  ERROR: cannot read file %s.\n",argv[2]);
			exit(1);
		}
	}
	else
	{
		fp_in = stdin;
	}
	if(argv[3][0]!='-')
	{
		if (( fp_out = fopen(argv[3],"wb")) == NULL ) {
			fprintf(stderr,"  ERROR: cannot write file %s.\n",argv[3]);
			exit(1);
		}
	}
	else
	{
		fp_out = stdout;
	}
	
	/* Create and initialize codec state */
	celp_state_t* state = celp_create_state();
	if (!state) {
		fprintf(stderr, "ERROR: cannot create codec state.\n");
		exit(1);
	}
	celp_init_state(state, 0);
	
	int length, frame, eof_reached=0;
	short speech[240];
	unsigned char buf[144/8];
	if(argv[1][0]=='e') //encode
	{
		while (1)
		{
			FSpeed(
				   length=fread(speech,sizeof(short),240,fp_in);
				   if(length<240)
					break; // Нулячить хвост
				   celp_encode_state(state, speech, buf);
				   fwrite(buf,sizeof(char),144/8,fp_out);
			
				   //тут надо брать в 11 байте бит и хз что делать с ним. лишний байт это ппц
				   if (fp_out == stdout) fflush(stdout);
				   if (fp_in == stdin) fflush(stdin);
			,getpid());
		}
	}
	else if(argv[1][0]=='d') //decode
	{
		while (1)
		{
			FSpeed(
				   length=fread(buf,sizeof(char),144/8,fp_in);
				   if(length<144/8)
				   break; // Exit if not enough data for a full frame
				   int result = celp_decode_state(state, buf, speech);
				   if(result != 0) {
					   fprintf(stderr, "CELP decode error: %d\n", result);
					   // Only continue if it's not a critical error
					   if (result == CELP_TWOERR || result == CELP_LSPERR) {
						   break; // Critical error, stop decoding
					   }
				   }
				   fwrite(speech,sizeof(short),240,fp_out);
				   if (fp_out == stdout) fflush(stdout);
				   if (fp_in == stdin) fflush(stdin);
			,getpid());
		}
	}
	else
	{
		while (1)
		{
			FSpeed(
				   length=fread(speech,sizeof(short),240,fp_in);
				   if(length<240)
				   break; // Нулячить хвост
				   celp_encode_state(state, speech, buf);
				   int result = celp_decode_state(state, buf, speech);
				   if(result != 0) {
					   fprintf(stderr, "CELP decode error: %d\n", result);
				   }
				   fwrite(speech,sizeof(short),240,fp_out);
				   if (fp_out == stdout) fflush(stdout);
				   if (fp_in == stdin) fflush(stdin);
			,getpid());
		}
	}
        celp_destroy_state(state);
	exit(0);
}
