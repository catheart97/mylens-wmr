/**
 * This file is used to define global constants which are used inside HLSL and C++.
 * All code must be parsable in HLSL and C++
 */

/** 
 * The maximum number of light sources supported (Due to shader instruction limit at 512) for the PBR Shader.
 */
#define MAX_NUMBER_LIGHTS 2