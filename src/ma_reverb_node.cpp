

#include "ma_reverb_node.h"

#ifdef __APPLE__
#include <stdlib.h>  /* OSX gets its malloc stuff through here */
#else
#include <malloc.h>
#endif

ma_reverb_node_config ma_reverb_node_config_init(ma_uint32 channels, ma_uint32 sampleRate)
{
    ma_reverb_node_config config;

    config.nodeConfig = ma_node_config_init();  /* Input and output channels will be set in ma_reverb_node_init(). */
    config.channels   = channels;
    config.sampleRate = sampleRate;
    config.roomSize   = verblib_initialroom;
    config.damping    = verblib_initialdamp;
    config.width      = verblib_initialwidth;
    config.wetVolume  = verblib_initialwet;
    config.dryVolume  = verblib_initialdry;
    config.mode       = verblib_initialmode;

    return config;
}

static void AudioProcessEffectLPF(const float* input_buffer, float* output_buffer, unsigned int frames)
{
    static float low[2] = { 0.0f, 0.0f };
    static const float cutoff = 70.0f / 44100.0f; // 70 Hz lowpass filter
    const float k = cutoff / (cutoff + 0.1591549431f); // RC filter formula
    
    for (unsigned int i = 0; i < frames*2; i += 2)
    {
        const float l = input_buffer[i];
        const float r = input_buffer[i + 1];
        
        low[0] += k * (l - low[0]);
        low[1] += k * (r - low[1]);
        
        output_buffer[i] = low[0];
        output_buffer[i + 1] = low[1];
    }
}

static float *delayBuffer = NULL;
static unsigned int delayBufferSize = 0;
static unsigned int delayReadIndex = 2;
static unsigned int delayWriteIndex = 0;

static void AudioProcessEffectDelay(const float* input_buffer, float* output_buffer, unsigned int frames)
{
    for (unsigned int i = 0; i < frames*2; i += 2)
    {
        float leftDelay = delayBuffer[delayReadIndex++];
        float rightDelay = delayBuffer[delayReadIndex++];
        
        if (delayReadIndex == delayBufferSize) delayReadIndex = 0;
        
        output_buffer[i] = 0.5f * input_buffer[i] + 0.5f * leftDelay;
        output_buffer[i + 1] = 0.5f * input_buffer[i + 1] + 0.5f * rightDelay;
        
        delayBuffer[delayWriteIndex++] = output_buffer[i];
        delayBuffer[delayWriteIndex++] = output_buffer[i + 1];
        
        if (delayWriteIndex == delayBufferSize) delayWriteIndex = 0;
    }
}

static void ma_reverb_node_process_pcm_frames(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut)
{
    ma_reverb_node* pReverbNode = (ma_reverb_node*)pNode;

    (void)pFrameCountIn;
    
    verblib_process(&pReverbNode->reverb, ppFramesIn[0], ppFramesOut[0], *pFrameCountOut);
//    AudioProcessEffectDelay(ppFramesIn[0], ppFramesOut[0], *pFrameCountOut);
}

static ma_node_vtable g_ma_reverb_node_vtable =
{
    ma_reverb_node_process_pcm_frames,
    NULL,
    1,  /* 1 input channel. */
    1,  /* 1 output channel. */
    MA_NODE_FLAG_CONTINUOUS_PROCESSING  /* Reverb requires continuous processing to ensure the tail get's processed. */
};

ma_result ma_reverb_node_init(ma_node_graph* pNodeGraph, const ma_reverb_node_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_reverb_node* pReverbNode)
{
    ma_result result;
    ma_node_config baseConfig;

    if (pReverbNode == NULL) {
        return MA_INVALID_ARGS;
    }

    if (pConfig == NULL) {
        return MA_INVALID_ARGS;
    }

    if (verblib_initialize(&pReverbNode->reverb, (unsigned long)pConfig->sampleRate, (unsigned int)pConfig->channels) == 0) {
        return MA_INVALID_ARGS;
    }

    baseConfig = pConfig->nodeConfig;
    baseConfig.vtable          = &g_ma_reverb_node_vtable;
    baseConfig.pInputChannels  = &pConfig->channels;
    baseConfig.pOutputChannels = &pConfig->channels;

    result = ma_node_init(pNodeGraph, &baseConfig, pAllocationCallbacks, &pReverbNode->baseNode);
    if (result != MA_SUCCESS) {
        return result;
    }
    
    // Allocate buffer for the delay effect
    float delayTime = 0.1f;
    delayBufferSize = delayTime * pConfig->sampleRate * pConfig->channels;
    delayBuffer = (float *)calloc(delayBufferSize, sizeof(float));

    return MA_SUCCESS;
}

void ma_reverb_node_uninit(ma_reverb_node* pReverbNode, const ma_allocation_callbacks* pAllocationCallbacks)
{
    /* The base node is always uninitialized first. */
    ma_node_uninit(pReverbNode, pAllocationCallbacks);
}
