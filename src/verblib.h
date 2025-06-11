/* Reverb Library
 * Verblib version 0.5 - 2022-10-25
 *
 * Philip Bennefall - philip@blastbay.com
 *
 * See the end of this file for licensing terms.
 * This reverb is based on Freeverb, a public domain reverb written by Jezar at Dreampoint.
 *
 * IMPORTANT: The reverb currently only works with 1 or 2 channels, at sample rates of 22050 HZ and above.
 * These restrictions may be lifted in a future version.
 *
 * USAGE
 *
 * This is a single-file library. To use it, do something like the following in one .c file.
 * #define VERBLIB_IMPLEMENTATION
 * #include "verblib.h"
 *
 * You can then #include this file in other parts of the program as you would with any other header file.
 */

#pragma once

/* The maximum sample rate that should be supported, specified as a multiple of 44100. */
#ifndef verblib_max_sample_rate_multiplier
#define verblib_max_sample_rate_multiplier 4
#endif

/* The silence threshold which is used when calculating decay time. */
#ifndef verblib_silence_threshold
#define verblib_silence_threshold 80.0 /* In dB (absolute). */
#endif

/* PUBLIC API */

typedef struct verblib verblib;

/* Initialize a verblib structure.
 *
 * Call this function to initialize the verblib structure.
 * Returns nonzero (true) on success or 0 (false) on failure.
 * The function will only fail if one or more of the parameters are invalid.
 */
int verblib_initialize ( verblib* verb, unsigned long sample_rate, unsigned int channels );

/* Run the reverb.
 *
 * Call this function continuously to generate your output.
 * output_buffer may be the same pointer as input_buffer if in place processing is desired.
 * frames specifies the number of sample frames that should be processed.
 */
void verblib_process ( verblib* verb, const float* input_buffer, float* output_buffer, unsigned long frames );

/* Set the size of the room, between 0.0 and 1.0. */
void verblib_set_room_size ( verblib* verb, float value );

/* Get the size of the room. */
float verblib_get_room_size ( const verblib* verb );

/* Set the amount of damping, between 0.0 and 1.0. */
void verblib_set_damping ( verblib* verb, float value );

/* Get the amount of damping. */
float verblib_get_damping ( const verblib* verb );

/* Set the stereo width of the reverb, between 0.0 and 1.0. */
void verblib_set_width ( verblib* verb, float value );

/* Get the stereo width of the reverb. */
float verblib_get_width ( const verblib* verb );

/* Set the volume of the wet signal, between 0.0 and 1.0. */
void verblib_set_wet ( verblib* verb, float value );

/* Get the volume of the wet signal. */
float verblib_get_wet ( const verblib* verb );

/* Set the volume of the dry signal, between 0.0 and 1.0. */
void verblib_set_dry ( verblib* verb, float value );

/* Get the volume of the dry signal. */
float verblib_get_dry ( const verblib* verb );

/* Set the stereo width of the input signal sent to the reverb, 0.0 or greater.
 * Values less than 1.0 narrow the signal, 1.0 sends the input signal unmodified, values greater than 1.0 widen the signal.
 */
void verblib_set_input_width ( verblib* verb, float value );

/* Get the stereo width of the input signal sent to the reverb. */
float verblib_get_input_width ( const verblib* verb );

/* Set the mode of the reverb, where values below 0.5 mean normal and values above mean frozen. */
void verblib_set_mode ( verblib* verb, float value );

/* Get the mode of the reverb. */
float verblib_get_mode ( const verblib* verb );

/* Get the decay time in sample frames based on the current room size setting. */
/* If freeze mode is active, the decay time is infinite and this function returns 0. */
unsigned long verblib_get_decay_time_in_frames ( const verblib* verb );

/* INTERNAL STRUCTURES */

/* Allpass filter */
typedef struct verblib_allpass verblib_allpass;
struct verblib_allpass
{
    float* buffer;
    float feedback;
    int  bufsize;
    int  bufidx;
};

/* Comb filter */
typedef struct verblib_comb verblib_comb;
struct verblib_comb
{
    float* buffer;
    float feedback;
    float filterstore;
    float damp1;
    float damp2;
    int  bufsize;
    int  bufidx;
};

/* Reverb model tuning values */
#define verblib_numcombs 8
#define verblib_numallpasses 4
#define verblib_muted 0.0f
#define verblib_fixedgain 0.015f
#define verblib_scalewet 3.0f
#define verblib_scaledry 2.0f
#define verblib_scaledamp 0.8f
#define verblib_scaleroom 0.28f
#define verblib_offsetroom 0.7f
#define verblib_initialroom 0.5f
#define verblib_initialdamp 0.25f
#define verblib_initialwet 1.0f/verblib_scalewet
#define verblib_initialdry 0.0f
#define verblib_initialwidth 1.0f
#define verblib_initialinputwidth 0.0f
#define verblib_initialmode 0.0f
#define verblib_freezemode 0.5f
#define verblib_stereospread 23

/*
 * These values assume 44.1KHz sample rate, but will be verblib_scaled appropriately.
 * The values were obtained by listening tests.
 */
#define verblib_combtuningL1 1116
#define verblib_combtuningR1 (1116+verblib_stereospread)
#define verblib_combtuningL2 1188
#define verblib_combtuningR2 (1188+verblib_stereospread)
#define verblib_combtuningL3 1277
#define verblib_combtuningR3 (1277+verblib_stereospread)
#define verblib_combtuningL4 1356
#define verblib_combtuningR4 (1356+verblib_stereospread)
#define verblib_combtuningL5 1422
#define verblib_combtuningR5 (1422+verblib_stereospread)
#define verblib_combtuningL6 1491
#define verblib_combtuningR6 (1491+verblib_stereospread)
#define verblib_combtuningL7 1557
#define verblib_combtuningR7 (1557+verblib_stereospread)
#define verblib_combtuningL8 1617
#define verblib_combtuningR8 (1617+verblib_stereospread)
#define verblib_allpasstuningL1 556
#define verblib_allpasstuningR1 (556+verblib_stereospread)
#define verblib_allpasstuningL2 441
#define verblib_allpasstuningR2 (441+verblib_stereospread)
#define verblib_allpasstuningL3 341
#define verblib_allpasstuningR3 (341+verblib_stereospread)
#define verblib_allpasstuningL4 225
#define verblib_allpasstuningR4 (225+verblib_stereospread)

/* The main reverb structure. This is the structure that you will create an instance of when using the reverb. */
struct verblib
{
    unsigned int channels;
    float gain;
    float roomsize, roomsize1;
    float damp, damp1;
    float wet, wet1, wet2;
    float dry;
    float width;
    float input_width;
    float mode;
    
    /*
     * The following are all declared inline
     * to remove the need for dynamic allocation.
     */
    
    /* Comb filters */
    verblib_comb combL[verblib_numcombs];
    verblib_comb combR[verblib_numcombs];
    
    /* Allpass filters */
    verblib_allpass allpassL[verblib_numallpasses];
    verblib_allpass allpassR[verblib_numallpasses];
    
    /* Buffers for the combs */
    float bufcombL1[verblib_combtuningL1* verblib_max_sample_rate_multiplier];
    float bufcombR1[verblib_combtuningR1* verblib_max_sample_rate_multiplier];
    float bufcombL2[verblib_combtuningL2* verblib_max_sample_rate_multiplier];
    float bufcombR2[verblib_combtuningR2* verblib_max_sample_rate_multiplier];
    float bufcombL3[verblib_combtuningL3* verblib_max_sample_rate_multiplier];
    float bufcombR3[verblib_combtuningR3* verblib_max_sample_rate_multiplier];
    float bufcombL4[verblib_combtuningL4* verblib_max_sample_rate_multiplier];
    float bufcombR4[verblib_combtuningR4* verblib_max_sample_rate_multiplier];
    float bufcombL5[verblib_combtuningL5* verblib_max_sample_rate_multiplier];
    float bufcombR5[verblib_combtuningR5* verblib_max_sample_rate_multiplier];
    float bufcombL6[verblib_combtuningL6* verblib_max_sample_rate_multiplier];
    float bufcombR6[verblib_combtuningR6* verblib_max_sample_rate_multiplier];
    float bufcombL7[verblib_combtuningL7* verblib_max_sample_rate_multiplier];
    float bufcombR7[verblib_combtuningR7* verblib_max_sample_rate_multiplier];
    float bufcombL8[verblib_combtuningL8* verblib_max_sample_rate_multiplier];
    float bufcombR8[verblib_combtuningR8* verblib_max_sample_rate_multiplier];
    
    /* Buffers for the allpasses */
    float bufallpassL1[verblib_allpasstuningL1* verblib_max_sample_rate_multiplier];
    float bufallpassR1[verblib_allpasstuningR1* verblib_max_sample_rate_multiplier];
    float bufallpassL2[verblib_allpasstuningL2* verblib_max_sample_rate_multiplier];
    float bufallpassR2[verblib_allpasstuningR2* verblib_max_sample_rate_multiplier];
    float bufallpassL3[verblib_allpasstuningL3* verblib_max_sample_rate_multiplier];
    float bufallpassR3[verblib_allpasstuningR3* verblib_max_sample_rate_multiplier];
    float bufallpassL4[verblib_allpasstuningL4* verblib_max_sample_rate_multiplier];
    float bufallpassR4[verblib_allpasstuningR4* verblib_max_sample_rate_multiplier];
};
