//
// Perlin Noise Utils
//
#define X_NOISE_GEN (1619)
#define Y_NOISE_GEN (31337)
#define Z_NOISE_GEN (6971)
#define SEED_NOISE_GEN (1013)

cbuffer cbNoise {
    float               g_persistence;
    uint                g_octaves;
    float               g_scale;
    uint                g_noiseSeed;
};

//
// Buffers
//
RWTexture2D<float> g_textureOut;

//
// Functions
//
float nlerp(float a, float b, float t) {
    t = (t * t * (3.0f - 2.0f * t));
    return ((1.0 - t) * a) + (t * b);
}

float baseNoise3(uint x, uint y, uint z, uint seed) {
    uint n = (
        x*X_NOISE_GEN +
        y*Y_NOISE_GEN +
        z*Z_NOISE_GEN +
        seed*SEED_NOISE_GEN
    );
    n = (n>>13)^n;
    return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

float noise2(float px, float py, uint seed) {
    float fx = px-(int)px;
    float fy = py-(int)py;
    uint x0 = (int)px;
    uint y0 = (int)py;
    uint x1 = x0 + 1;
    uint y1 = y0 + 1;
    float n0, n1, ix0, iy0;

    n0 = baseNoise3(x0, y0, 0, seed);
    n1 = baseNoise3(x1, y0, 0, seed);
    ix0 = nlerp(n0, n1, fx);
    n0 = baseNoise3(x0, y1, 0, seed);
    n1 = baseNoise3(x1, y1, 0, seed);
    iy0 = nlerp(n0, n1, fx);
    return nlerp(ix0, iy0, fy);
}

float perlinNoise2(float px, float py, float persistence, int octaves, uint seed) {
    float noise = 0;
    for(int i = 0; i < octaves; ++i) {
        float feq = pow(2.f,i);
        float amp = pow(persistence,i);
        noise += noise2(px*feq,py*feq, seed)*amp;
    }
    return noise;
}

[numthreads(32,32,1)]
void csPerlinNoise(uint3 DTid : SV_DispatchThreadID) {
    float n = perlinNoise2(DTid.x*g_scale, DTid.y*g_scale, g_persistence, g_octaves, g_noiseSeed);
    n += 2; n /= 4;
    g_textureOut[DTid.xy] = n;
}