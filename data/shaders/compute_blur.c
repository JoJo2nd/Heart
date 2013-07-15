
#if 0
#   ifndef SCAN_WARP_SIZE
#   	define SCAN_WARP_SIZE (32)
#   endif
#   ifndef SCAN_LOG2_WARP_SIZE
#   	define SCAN_LOG2_WARP_SIZE (5)
#   endif
#   ifndef THREADS_PER_GROUP
#   	define THREADS_PER_GROUP (128)
#   endif
#   ifndef SCAN_SMEM_SIZE
#   	define SCAN_SMEM_SIZE (1200)
#   endif
#   ifndef TEXELS_PER_THREAD
#   	define TEXELS_PER_THREAD (5)
#   endif
#   ifndef NUM_IMAGE_ROWS
#   	define NUM_IMAGE_ROWS (1200)
#   endif
#   ifndef NUM_IMAGE_COLS
#   	define NUM_IMAGE_COLS (1600)
#   endif
#endif
//-----------------------------------------------------------------------------
// Shared memory variables
//-----------------------------------------------------------------------------
#define CUSTOM_METHOD
#ifdef CUSTOM_METHOD

// The compute shaders for column and row filtering are created from the same
// function, distinguished by macro "SCAN_COL_PASS" parsed from compiler.
#ifdef CM_SCAN_COL_PASS
	#define input_color(a, b)	input_col_color(a, b)
	#define output_color(a, b)	output_col_color(a, b)
#pragma message "Compute Blur Col Pass"
#else
	#define input_color(a, b)	input_row_color(a, b)
	#define output_color(a, b)	output_row_color(a, b)
#pragma message "Compute Blur Row Pass"
#endif
#define CM_SCAN_SMEM_SIZE (1920)
// A Thread group size of 64 results in a GPU time of 2ms on Radeon HD 7800
// A Thread group size of 265 results in a GPU time of 0.5ms on Radeon HD 7800
// A Thread group size of 512 results in a GPU time of 0.398ms on Radeon HD 7800
#define CM_THREAD_GROUP_X (512) 
#define CM_THREADS_PER_GROUP (CM_THREAD_GROUP_X)
#define CM_GROUP_TOP_VALUE_COUNT (CM_SCAN_SMEM_SIZE/CM_THREAD_GROUP_X)
// Shared memory for prefix sum operation (scan).
groupshared float3 gs_scanData[CM_SCAN_SMEM_SIZE];

//-----------------------------------------------------------------------------
// Buffers, textures & samplers
//-----------------------------------------------------------------------------

// FP16x4 texture of rendered scene
Texture2D				g_texInput;
// R11G11B10 or XRGB8 texture for output
RWTexture2D<uint>		g_rwtOutput;

//-----------------------------------------------------------------------------
// Shader constant buffers
//-----------------------------------------------------------------------------

cbuffer cbParams
{
    uint  g_inputDataSize;              // 
    uint  g_inputDataSizeLog2;          //
	uint  g_numApproxPasses;
	float g_halfBoxFilterWidth;			// w/2
	float g_fracHalfBoxFilterWidth;		// frac(w/2+0.5)
	float g_invFracHalfBoxFilterWidth;	// 1-frac(w/2+0.5)
	float g_rcpBoxFilterWidth;			// 1/w
};

///
///
///
void all_partial_sums_global(uint k_threadid, uint size, uint j_log2) {
    uint read1=k_threadid;
    uint read2offset=1 << (j_log2-1);//pow(2,(j_log2-1));
    if (k_threadid >= read2offset) {//pow(2, j_log2)
        while (read1 < size) {
            gs_scanData[read1] = gs_scanData[read1-read2offset] + gs_scanData[read1];
            read1+=CM_THREADS_PER_GROUP;
        }
    }
}
///
///
///
void all_partial_sums(uint threadid, uint size, uint sizelog2, const bool do_filtering) {
    //First do all partial sum at the thread group level and store in scanData
    uint i;
    uint j;
    uint TGsize=CM_THREADS_PER_GROUP;
    uint TGlog2=log2(TGsize);
    float3 top_results[CM_GROUP_TOP_VALUE_COUNT];
    for (j=1; j <= TGlog2; ++j) { 
        all_partial_sums_global(threadid, g_inputDataSize, j);
    }
    GroupMemoryBarrierWithGroupSync();
    
    //Grab the top/end/final result from each thread group. We accumulate this locally.
    uint row = CM_THREADS_PER_GROUP-1;
    uint top_result_count=((g_inputDataSize+(CM_THREADS_PER_GROUP-1))/CM_THREADS_PER_GROUP);

    for (i=0; i<top_result_count; ++i) {
        top_results[i]=gs_scanData[row];
        row += CM_THREADS_PER_GROUP;
    }
    for (i=1; i<top_result_count; ++i) {
        top_results[i]+=top_results[i-1];
    }
    
    //Add the accumulated results to gs_scanData, start at
    //threadid + CM_THREADS_PER_GROUP to skip the 1st thread group and 
    //leave unchanged.
    row = threadid + CM_THREADS_PER_GROUP;
    for (i=0; i<(top_result_count-1); ++i) {
        gs_scanData[row] += top_results[i];
        row += CM_THREADS_PER_GROUP;
    }
    // Must wait for results to be complete before moving on
    GroupMemoryBarrierWithGroupSync();

}
///
///
///
uint color3_to_uintR11G11B10(float3 color) {
    // Convert to R11G11B10
    color = clamp(color, 0, 1);
    uint int_r = (uint)(color.r * 2047.0f + 0.5f);
    uint int_g = (uint)(color.g * 2047.0f + 0.5f);
    uint int_b = (uint)(color.b * 1023.0f + 0.5f);
    
    // Pack into UINT32
    return (int_r << 21) | (int_g << 10) | int_b;
}
///
///
///
uint color3_to_uintX8R8G8B8(float3 color) {
    // Convert to R8G8B8
    color = clamp(color, 0, 1);
    uint int_r = (uint)(color.r * 255.0f + 0.5f);
    uint int_g = (uint)(color.g * 255.0f + 0.5f);
    uint int_b = (uint)(color.b * 255.0f + 0.5f);
    
    // Pack into UINT32
    return (255 << 24) | (int_r << 16) | (int_g << 8) | int_b;
}
///
///
///
void input_row_colour(uint row, uint col) { // --X++
    //uint col = group_id;
    //uint row = thread_id;
    // Fetch in color
    while (row < g_inputDataSize) {
        gs_scanData[row].rgb = g_texInput[uint2(row, col)].rgb;
        row += CM_THREADS_PER_GROUP;
    }
}
///
///
///
void output_row_colour(uint row, uint col) { // --X++
    while (row < g_inputDataSize) {
        g_rwtOutput[uint2(row, col)]=color3_to_uintX8R8G8B8(gs_scanData[row].rgb);
        row += CM_THREADS_PER_GROUP;
    }
}
///
///
///
[numthreads(CM_THREADS_PER_GROUP, 1, 1)]
void GaussianColor_CS(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID) {
    input_row_colour(GTid.x, Gid.x);
    GroupMemoryBarrierWithGroupSync();
    all_partial_sums(GTid.x, g_inputDataSize, g_inputDataSizeLog2, false);
    //for (uint i=0; i<g_numApproxPasses; ++i) {
    //    all_partial_sums(GTid.x, g_inputDataSize, g_inputDataSizeLog2, true);
    //}
    output_row_colour(GTid.x, Gid.x);
}

#else

#endif