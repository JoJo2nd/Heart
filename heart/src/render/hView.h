/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hStringID.h"
#include "render/hRenderPrim.h"
#include <memory>

namespace Heart {
class hRenderingPipeline;

namespace hRenderer {

struct hPipelineState;
struct hInputState;
struct hView;
struct hCmdList;

struct hDrawCall {
    static const hUint64 KeyMask = 0x7FFFFFFFFFFFFFFF;

    hUint64 sortKey = 0;
    hRenderer::hCmdList* customCall = nullptr;
    hPipelineState* pipelineState = nullptr;
    hInputState* inputState = nullptr;
    Primative primType = Primative::Triangles;
    hUint primCount = 0;
    hUint firstVertex = 0;
};

class hDrawCallSet {
    hUint reserve = 0;
    hUint count = 0;
    hView* owning_view = 0;
    hDrawCall* begin;
public:

    hDrawCallSet(hUint in_reserve, hDrawCall* begin_ptr, hView* owning_view) {
        count = 0;
        reserve = in_reserve;
        begin = begin_ptr;
    }
    hBool addDrawCall(hUint64 key, hPipelineState* pipeline_state, hInputState* input_state, Primative in_prim_type, hUint in_prim_count, hUint in_first_vertex) {
        hcAssertMsg((key & hDrawCall::KeyMask) == 0, "The MSB in a draw call key is reserved.");
        if (count >= reserve) return hFalse;
        hDrawCall* dc = begin+count;
        dc->sortKey = key&hDrawCall::KeyMask;
        dc->customCall = nullptr;
        dc->pipelineState = pipeline_state;
        dc->inputState = input_state;
        dc->primType = in_prim_type;
        dc->primCount = in_prim_count;
        dc->firstVertex = in_first_vertex;
        ++count;
        return hTrue;
    }
    hBool addCustomCall(hUint64 key, hRenderer::hCmdList* cmd_list) {
        hcAssertMsg((key & hDrawCall::KeyMask) == 0, "The MSB in a draw call key is reserved.");
        hcAssertMsg(cmd_list, "cmd_list cannot be null!");
        if (count >= reserve) return hFalse;
        hDrawCall* dc = begin + count;
        dc->sortKey = key&hDrawCall::KeyMask;
        dc->customCall = cmd_list;
        ++count;
        return hTrue;
    }
    void finish() {
        for (hUint i=count, n=reserve; i<n; ++i) {
            begin[i].sortKey=(~hDrawCall::KeyMask);
        }
    }
};

void registerViewTasks();
void reinitialiseViews(const hRenderingPipeline& pipeline);
hView* getView(hStringID view_name);
hCmdList* getViewPreDrawCmdList(hView* view);
hUint32 getViewTechniqueFlags(hView* view);
void setupViewForFrame(hView* view /*viewport? camera? projection?*/);
hDrawCallSet allocateDrawCallsForView(hView* view, hUint count);

extern const hStringID initialise_views_for_frame_task;
extern const hStringID sort_views_for_frame_task;
extern const hStringID submit_views_task;
extern const hStringID submit_frame_task;

}
}