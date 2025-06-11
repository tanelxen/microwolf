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

#include "verblib.h"

#include <stddef.h>
#include <math.h>

#ifdef _MSC_VER
#define VERBLIB_INLINE __forceinline
#else
#ifdef __GNUC__
#define VERBLIB_INLINE inline __attribute__((always_inline))
#else
#define VERBLIB_INLINE inline
#endif
#endif

#define verblib_max(x, y)                (((x) > (y)) ? (x) : (y))

#define undenormalise(sample) sample+=1.0f; sample-=1.0f;

/* Allpass filter */
static void verblib_allpass_initialize ( verblib_allpass* allpass, float* buf, int size )
{
    allpass->buffer = buf;
    allpass->bufsize = size;
    allpass->bufidx = 0;
}

static VERBLIB_INLINE float verblib_allpass_process ( verblib_allpass* allpass, float input )
{
    float output;
    float bufout;
    
    bufout = allpass->buffer[allpass->bufidx];
    undenormalise ( bufout );
    
    output = -input + bufout;
    allpass->buffer[allpass->bufidx] = input + ( bufout * allpass->feedback );
    
    if ( ++allpass->bufidx >= allpass->bufsize )
    {
        allpass->bufidx = 0;
    }
    
    return output;
}

static void verblib_allpass_mute ( verblib_allpass* allpass )
{
    int i;
    for ( i = 0; i < allpass->bufsize; i++ )
    {
        allpass->buffer[i] = 0.0f;
    }
}

/* Comb filter */
static void verblib_comb_initialize ( verblib_comb* comb, float* buf, int size )
{
    comb->buffer = buf;
    comb->bufsize = size;
    comb->filterstore = 0.0f;
    comb->bufidx = 0;
}

static void verblib_comb_mute ( verblib_comb* comb )
{
    int i;
    for ( i = 0; i < comb->bufsize; i++ )
    {
        comb->buffer[i] = 0.0f;
    }
}

static void verblib_comb_set_damp ( verblib_comb* comb, float val )
{
    comb->damp1 = val;
    comb->damp2 = 1.0f - val;
}

static VERBLIB_INLINE float verblib_comb_process ( verblib_comb* comb, float input )
{
    float output;
    
    output = comb->buffer[comb->bufidx];
//    undenormalise ( output );
    
    comb->filterstore = ( output * comb->damp2 ) + ( comb->filterstore * comb->damp1 );
//    undenormalise ( comb->filterstore );
    
    comb->buffer[comb->bufidx] = input + ( comb->filterstore * comb->feedback );
    
    if ( ++comb->bufidx >= comb->bufsize )
    {
        comb->bufidx = 0;
    }
    
    return output;
}

static void verblib_update ( verblib* verb )
{
    /* Recalculate internal values after parameter change. */
    
    int i;
    
    verb->wet1 = verb->wet * ( verb->width / 2.0f + 0.5f );
    verb->wet2 = verb->wet * ( ( 1.0f - verb->width ) / 2.0f );
    
    if ( verb->mode >= verblib_freezemode )
    {
        verb->roomsize1 = 1.0f;
        verb->damp1 = 0.0f;
        verb->gain = verblib_muted;
    }
    else
    {
        verb->roomsize1 = verb->roomsize;
        verb->damp1 = verb->damp;
        verb->gain = verblib_fixedgain;
    }
    
    for ( i = 0; i < verblib_numcombs; i++ )
    {
        
        verb->combL[i].feedback = verb->roomsize1;
        verb->combR[i].feedback = verb->roomsize1;
        verblib_comb_set_damp ( &verb->combL[i], verb->damp1 );
        verblib_comb_set_damp ( &verb->combR[i], verb->damp1 );
    }
    
}

static void verblib_mute ( verblib* verb )
{
    int i;
    if ( verblib_get_mode ( verb ) >= verblib_freezemode )
    {
        return;
    }
    
    for ( i = 0; i < verblib_numcombs; i++ )
    {
        verblib_comb_mute ( &verb->combL[i] );
        verblib_comb_mute ( &verb->combR[i] );
    }
    for ( i = 0; i < verblib_numallpasses; i++ )
    {
        verblib_allpass_mute ( &verb->allpassL[i] );
        verblib_allpass_mute ( &verb->allpassR[i] );
    }
}

static int verblib_get_verblib_scaled_buffer_size ( unsigned long sample_rate, unsigned long value )
{
    long double result = ( long double ) sample_rate;
    result /= 44100.0;
    result = ( ( long double ) value ) * result;
    if ( result < 1.0 )
    {
        result = 1.0;
    }
    return ( int ) result;
}

int verblib_initialize ( verblib* verb, unsigned long sample_rate, unsigned int channels )
{
    int i;
    
    if ( channels != 1 && channels != 2 )
    {
        return 0;    /* Currently supports only 1 or 2 channels. */
    }
    if ( sample_rate < 22050 )
    {
        return 0;    /* The minimum supported sample rate is 22050 HZ. */
    }
    else if ( sample_rate > 44100 * verblib_max_sample_rate_multiplier )
    {
        return 0; /* The sample rate is too high. */
    }
    
    verb->channels = channels;
    
    /* Tie the components to their buffers. */
    verblib_comb_initialize ( &verb->combL[0], verb->bufcombL1, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningL1 ) );
    verblib_comb_initialize ( &verb->combR[0], verb->bufcombR1, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningR1 ) );
    verblib_comb_initialize ( &verb->combL[1], verb->bufcombL2, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningL2 ) );
    verblib_comb_initialize ( &verb->combR[1], verb->bufcombR2, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningR2 ) );
    verblib_comb_initialize ( &verb->combL[2], verb->bufcombL3, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningL3 ) );
    verblib_comb_initialize ( &verb->combR[2], verb->bufcombR3, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningR3 ) );
    verblib_comb_initialize ( &verb->combL[3], verb->bufcombL4, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningL4 ) );
    verblib_comb_initialize ( &verb->combR[3], verb->bufcombR4, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningR4 ) );
    verblib_comb_initialize ( &verb->combL[4], verb->bufcombL5, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningL5 ) );
    verblib_comb_initialize ( &verb->combR[4], verb->bufcombR5, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningR5 ) );
    verblib_comb_initialize ( &verb->combL[5], verb->bufcombL6, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningL6 ) );
    verblib_comb_initialize ( &verb->combR[5], verb->bufcombR6, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningR6 ) );
    verblib_comb_initialize ( &verb->combL[6], verb->bufcombL7, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningL7 ) );
    verblib_comb_initialize ( &verb->combR[6], verb->bufcombR7, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningR7 ) );
    verblib_comb_initialize ( &verb->combL[7], verb->bufcombL8, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningL8 ) );
    verblib_comb_initialize ( &verb->combR[7], verb->bufcombR8, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_combtuningR8 ) );
    
    verblib_allpass_initialize ( &verb->allpassL[0], verb->bufallpassL1, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_allpasstuningL1 ) );
    verblib_allpass_initialize ( &verb->allpassR[0], verb->bufallpassR1, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_allpasstuningR1 ) );
    verblib_allpass_initialize ( &verb->allpassL[1], verb->bufallpassL2, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_allpasstuningL2 ) );
    verblib_allpass_initialize ( &verb->allpassR[1], verb->bufallpassR2, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_allpasstuningR2 ) );
    verblib_allpass_initialize ( &verb->allpassL[2], verb->bufallpassL3, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_allpasstuningL3 ) );
    verblib_allpass_initialize ( &verb->allpassR[2], verb->bufallpassR3, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_allpasstuningR3 ) );
    verblib_allpass_initialize ( &verb->allpassL[3], verb->bufallpassL4, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_allpasstuningL4 ) );
    verblib_allpass_initialize ( &verb->allpassR[3], verb->bufallpassR4, verblib_get_verblib_scaled_buffer_size ( sample_rate, verblib_allpasstuningR4 ) );
    
    /* Set default values. */
    for ( i = 0; i < verblib_numallpasses; i++ )
    {
        verb->allpassL[i].feedback = 0.5f;
        verb->allpassR[i].feedback = 0.5f;
    }
    
    verblib_set_wet ( verb, verblib_initialwet );
    verblib_set_room_size ( verb, verblib_initialroom );
    verblib_set_dry ( verb, verblib_initialdry );
    verblib_set_damping ( verb, verblib_initialdamp );
    verblib_set_width ( verb, verblib_initialwidth );
    verblib_set_input_width ( verb, verblib_initialinputwidth );
    verblib_set_mode ( verb, verblib_initialmode );
    
    /* The buffers will be full of rubbish - so we MUST mute them. */
    verblib_mute ( verb );
    
    return 1;
}

void verblib_process ( verblib* verb, const float* input_buffer, float* output_buffer, unsigned long frames )
{
    int i;
    float outL, outR, input;
    
    if ( verb->channels == 1 )
    {
        while ( frames-- > 0 )
        {
            outL = 0.0f;
            input = ( input_buffer[0] * 2.0f ) * verb->gain;
            
            /* Accumulate comb filters in parallel. */
            for ( i = 0; i < verblib_numcombs; i++ )
            {
                outL += verblib_comb_process ( &verb->combL[i], input );
            }
            
            /* Feed through allpasses in series. */
            for ( i = 0; i < verblib_numallpasses; i++ )
            {
                outL = verblib_allpass_process ( &verb->allpassL[i], outL );
            }
            
            /* Calculate output REPLACING anything already there. */
            output_buffer[0] = outL * verb->wet1 + input_buffer[0] * verb->dry;
            
            /* Increment sample pointers. */
            ++input_buffer;
            ++output_buffer;
        }
    }
    else if ( verb->channels == 2 )
    {
        if ( verb->input_width > 0.0f ) /* Stereo input is widened or narrowed. */
        {
            
            /*
             * The stereo mid/side code is derived from:
             * https://www.musicdsp.org/en/latest/Effects/256-stereo-width-control-obtained-via-transfromation-matrix.html
             * The description of the code on the above page says:
             *
             * This work is hereby placed in the public domain for all purposes, including
             * use in commercial applications.
             */
            
            const float tmp = 1 / verblib_max ( 1 + verb->input_width, 2 );
            const float coef_mid = 1 * tmp;
            const float coef_side = verb->input_width * tmp;
            while ( frames-- > 0 )
            {
                const float mid = ( input_buffer[0] + input_buffer[1] ) * coef_mid;
                const float side = ( input_buffer[1] - input_buffer[0] ) * coef_side;
                const float input_left = ( mid - side ) * ( verb->gain * 2.0f );
                const float input_right = ( mid + side ) * ( verb->gain * 2.0f );
                
                outL = outR = 0.0f;
                
                /* Accumulate comb filters in parallel. */
                for ( i = 0; i < verblib_numcombs; i++ )
                {
                    outL += verblib_comb_process ( &verb->combL[i], input_left );
                    outR += verblib_comb_process ( &verb->combR[i], input_right );
                }
                
                /* Feed through allpasses in series. */
                for ( i = 0; i < verblib_numallpasses; i++ )
                {
                    outL = verblib_allpass_process ( &verb->allpassL[i], outL );
                    outR = verblib_allpass_process ( &verb->allpassR[i], outR );
                }
                
                /* Calculate output REPLACING anything already there. */
                output_buffer[0] = outL * verb->wet1 + outR * verb->wet2;// + input_buffer[0] * verb->dry;
                output_buffer[1] = outR * verb->wet1 + outL * verb->wet2;// + input_buffer[1] * verb->dry;
                
                /* Increment sample pointers. */
                input_buffer += 2;
                output_buffer += 2;
            }
        }
        else /* Stereo input is summed to mono. */
        {
            while ( frames-- > 0 )
            {
                outL = outR = 0.0f;
                input = ( input_buffer[0] + input_buffer[1] ) * verb->gain;
                
                /* Accumulate comb filters in parallel. */
                for ( i = 0; i < verblib_numcombs; i++ )
                {
                    outL += verblib_comb_process ( &verb->combL[i], input );
                    outR += verblib_comb_process ( &verb->combR[i], input );
                }
                
                /* Feed through allpasses in series. */
                for ( i = 0; i < verblib_numallpasses; i++ )
                {
                    outL = verblib_allpass_process ( &verb->allpassL[i], outL );
                    outR = verblib_allpass_process ( &verb->allpassR[i], outR );
                }
                
                /* Calculate output REPLACING anything already there. */
                output_buffer[0] = outL * verb->wet1 + outR * verb->wet2 + input_buffer[0] * verb->dry;
                output_buffer[1] = outR * verb->wet1 + outL * verb->wet2 + input_buffer[1] * verb->dry;
                
                /* Increment sample pointers. */
                input_buffer += 2;
                output_buffer += 2;
            }
        }
    }
}

void verblib_set_room_size ( verblib* verb, float value )
{
    verb->roomsize = ( value * verblib_scaleroom ) + verblib_offsetroom;
    verblib_update ( verb );
}

float verblib_get_room_size ( const verblib* verb )
{
    return ( verb->roomsize - verblib_offsetroom ) / verblib_scaleroom;
}

void verblib_set_damping ( verblib* verb, float value )
{
    verb->damp = value * verblib_scaledamp;
    verblib_update ( verb );
}

float verblib_get_damping ( const verblib* verb )
{
    return verb->damp / verblib_scaledamp;
}

void verblib_set_wet ( verblib* verb, float value )
{
    verb->wet = value * verblib_scalewet;
    verblib_update ( verb );
}

float verblib_get_wet ( const verblib* verb )
{
    return verb->wet / verblib_scalewet;
}

void verblib_set_dry ( verblib* verb, float value )
{
    verb->dry = value * verblib_scaledry;
}

float verblib_get_dry ( const verblib* verb )
{
    return verb->dry / verblib_scaledry;
}

void verblib_set_width ( verblib* verb, float value )
{
    verb->width = value;
    verblib_update ( verb );
}

float verblib_get_width ( const verblib* verb )
{
    return verb->width;
}

void verblib_set_input_width ( verblib* verb, float value )
{
    verb->input_width = value;
}

float verblib_get_input_width ( const verblib* verb )
{
    return verb->input_width;
}

void verblib_set_mode ( verblib* verb, float value )
{
    verb->mode = value;
    verblib_update ( verb );
}

float verblib_get_mode ( const verblib* verb )
{
    if ( verb->mode >= verblib_freezemode )
    {
        return 1.0f;
    }
    return 0.0f;
}

unsigned long verblib_get_decay_time_in_frames ( const verblib* verb )
{
    double decay;
    
    if ( verb->mode >= verblib_freezemode )
    {
        return 0; /* Freeze mode creates an infinite decay. */
    }
    
    decay = verblib_silence_threshold / fabs ( -20.0 * log ( 1.0 / verb->roomsize1 ) );
    decay *= ( double ) ( verb->combR[7].bufsize * 2 );
    return ( unsigned long ) decay;
}
