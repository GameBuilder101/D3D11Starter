#ifndef __NORMAL_MAPPING__
#define __NORMAL_MAPPING__

// Converts from 0 to 1 range into -1 to 1
float3 UnpackNormal(float3 sampled)
{
    return normalize(sampled * 2.0f - 1.0f);
}

// Transforms the normal using TBN matrix for use in lighting calculations
float3 TransformNormal(float3 unpacked, float3 surfaceNormal, float3 surfaceTangent)
{
    // Create TBN matrix
    float3 N = normalize(surfaceNormal);
    float3 T = normalize(surfaceTangent - dot(surfaceTangent, N) * N); // Gram–Schmidt orthonormalization
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    
    // Perform transformation and return
    return mul(unpacked, TBN);
}

#endif