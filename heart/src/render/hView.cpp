/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "hView.h"
#include "base/hAtomic.h"
#include "render/hRenderPipeline.h"
#include "render/hRendererCamera.h"
#include "render/hTechniques.h"
#include "threading/hTaskGraphSystem.h"
#include <vector>
#include "base/hMutexAutoScope.h"

namespace Heart {
namespace hRenderer {
const hStringID initialise_views_for_frame_task("heart::views::begin");
const hStringID sort_views_for_frame_task("heart::views::sort");
const hStringID submit_views_task("heart::views::submit");
const hStringID submit_frame_task("heart::views::submit_frame");

struct hView {
    hView() 
        : nTechniques(0) {
        hMemSet(targets, 0, sizeof(targets));
    }
    hView(const hView&& rhs) {
        viewName = rhs.viewName;
        nTechniques = rhs.nTechniques;
        for (hUint i=0; i<8; ++i) techniques[i] = rhs.techniques[i];
        hAtomic::AtomicSet(allocatedDrawCalls,hAtomic::AtomicGet(rhs.allocatedDrawCalls));
        drawcalls = std::move(drawcalls);
    }
    hStringID viewName;
    hBool clearColour : 1;
    hBool clearDepth : 1;
    hBool clearStencil : 1;
    hUint8 stencilValue;
    hFloat depthValue;
    hColour colourValue;
    hUint maxDrawCalls;
    hUint32 techniqueFlags;
    hUint nTechniques;
    hStringID techniques[8];
    hRendererCamera camera;
    hRenderer::hCmdList* preDrawCmdList;
    hRenderer::hCmdList* drawCmdList;
    hMutex allocMtx;
    hAtomicInt allocatedDrawCalls;
    hUint nTargets;
    hRenderTarget* targets[8];
    std::vector<hDrawCall> drawcalls;

    hDrawCall* reserveDrawCalls(hInt count) {
        hMutexAutoScope mas(&allocMtx);
        auto end_index = hAtomic::AtomicAdd(allocatedDrawCalls, count);
        // Out of space? we could set a flag here and allocate more draw calls in the next frame?
        if (end_index >= maxDrawCalls) {
            hcAssert("Ran out of draw calls!");
            return nullptr;
        }
        return ((drawcalls.data()+end_index)-count);
    }
};

namespace HEART_ANONYMOUS_NAMESPACE {
    struct hViewTaskInput {
        hView* view;
        hRenderer::hCmdList* renderCmdList;
    };

    std::vector<hView> activeViews;
    std::vector<hViewTaskInput> viewTaskInputs;

    void initialiseViewsForFrame(hTaskInfo* info) {
        auto* graph = info->owningGraph;
        hTaskHandle sort_task = graph->findTaskByName(sort_views_for_frame_task);
        hTaskHandle submit_task = graph->findTaskByName(submit_views_task);
        graph->clearTaskInputs(sort_task);
        graph->clearTaskInputs(submit_task);
        for (hSize_t i=0, n=activeViews.size(); i<n; ++i) {
            setupViewForFrame(&activeViews[i]);
            viewTaskInputs[i].view = &activeViews[i];
            viewTaskInputs[i].renderCmdList = activeViews[i].drawCmdList;
            graph->addTaskInput(sort_task, &viewTaskInputs[i]);
            graph->addTaskInput(submit_task, &viewTaskInputs[i]);
        }
    }

    void sortViews(hTaskInfo* info) {
        auto* task_info = (hViewTaskInput*)info->taskInput;
        if (!task_info) return; // No task input, bail.
        auto draw_count = hAtomic::AtomicGet(task_info->view->allocatedDrawCalls);
        if (!draw_count) return;
        std::sort(task_info->view->drawcalls.begin(), task_info->view->drawcalls.begin()+draw_count, [](const hDrawCall& lhs, const hDrawCall& rhs) {
            return lhs.sortKey < rhs.sortKey;
        });
    }

    void submitViews(hTaskInfo* info) {
        auto* task_info = (hViewTaskInput*)info->taskInput;
        if (!task_info) return; // No task input, bail.
        auto* view = task_info->view;
        hMutexAutoScope mas(&view->allocMtx);
        auto* targets = view->targets;
        auto n_targets = view->nTargets;
        auto* cl = task_info->renderCmdList;
        auto draw_count = hAtomic::AtomicGet(task_info->view->allocatedDrawCalls);
        auto* dc_ptr = task_info->view->drawcalls.data();
        auto* dc_end = dc_ptr+draw_count;
        
        hRenderer::setRenderTargets(cl, targets, n_targets);
        if (view->clearColour)
            hRenderer::clear(cl, view->colourValue, view->depthValue);
        for (hUint dc_i=0; dc_i < draw_count; ++dc_i) {
            if (dc_ptr[dc_i].sortKey == ~hDrawCall::KeyMask) 
                break;
            if (dc_ptr[dc_i].customCall)
                hRenderer::call(cl, dc_ptr[dc_i].customCall);
            else
                hRenderer::draw(cl, dc_ptr[dc_i].pipelineState, dc_ptr[dc_i].inputState, dc_ptr[dc_i].primType, dc_ptr[dc_i].primCount, dc_ptr[dc_i].firstVertex);
        }
    }

    void submitFrameTask(hTaskInfo* info) {
        auto* cl = (hRenderer::hCmdList*)info->taskInput;
        for (hSize_t i = 0, n = activeViews.size(); i<n; ++i) {
            if (activeViews[i].preDrawCmdList) {
                hRenderer::call(cl, activeViews[i].preDrawCmdList);
                activeViews[i].preDrawCmdList = nullptr;
            }
            if (activeViews[i].drawCmdList){
                hRenderer::call(cl, activeViews[i].drawCmdList);
                activeViews[i].drawCmdList = nullptr;
            }
        }
        
        hRenderer::swapBuffers(cl);
    }
} 
HEART_USE_ANONYMOUS_NAMESPACE;


void registerViewTasks() {
    hTaskFactory::registerTask(initialise_views_for_frame_task, initialiseViewsForFrame);
    hTaskFactory::registerTask(sort_views_for_frame_task, sortViews);
    hTaskFactory::registerTask(submit_views_task, submitViews);
    hTaskFactory::registerTask(submit_frame_task, submitFrameTask);
}

void reinitialiseViews(const hRenderingPipeline& pipeline) {
    activeViews.clear();
    activeViews.resize(pipeline.getStageCount());
    viewTaskInputs.clear();
    viewTaskInputs.resize(pipeline.getStageCount());
    for (hUint i=0, n=pipeline.getStageCount(); i<n; ++i) {
        const auto& stage = pipeline.getStage(i);
        activeViews[i].viewName = stage.viewName;
        activeViews[i].clearColour = stage.clearColour;
        activeViews[i].clearDepth = stage.clearDepth;
        activeViews[i].clearStencil = stage.clearStencil;
        activeViews[i].colourValue = stage.colorValue;
        activeViews[i].depthValue = stage.depthValue;
        activeViews[i].stencilValue = stage.stencilValue;
        activeViews[i].maxDrawCalls = stage.maxDrawCalls;
        activeViews[i].nTechniques = stage.nTechniques;
        for (hUint tech_i=0, tech_n=activeViews[i].nTechniques; tech_i<tech_n; ++tech_i) {
            hTechniques::registerTechnique(stage.techniques[tech_i]);
            activeViews[i].techniques[tech_i] = stage.techniques[tech_i];
        }
        activeViews[i].techniqueFlags = hTechniques::getTechniqueFlags(activeViews[i].techniques, activeViews[i].nTechniques);
        activeViews[i].targets[0] = nullptr;
        activeViews[i].nTargets = hMax(stage.nOutputs, 1);
        for (hUint output_i=0, output_n=stage.nOutputs; output_i<output_n; ++output_i) {
            activeViews[i].targets[output_i] = stage.outputs[output_i].target;
        }
    }
}

hView* getView(hStringID view_name) {
    for (auto& i : activeViews) {
        if (i.viewName == view_name) return &i;
    }
    return nullptr;
}
 
hCmdList* getViewPreDrawCmdList(hView* view) {
    return view->preDrawCmdList;
}

hUint32 getViewTechniqueFlags(hView* view) {
    return view->techniqueFlags;
}

void setupViewForFrame(hView* view /*viewport? camera? projection?*/) {
    hMutexAutoScope mas(&view->allocMtx);
    hcAssert(view >= activeViews.data() && view < activeViews.data()+activeViews.size());
    view->drawcalls.resize(view->maxDrawCalls);
    hAtomic::AtomicSet(view->allocatedDrawCalls, 0);
    view->preDrawCmdList = hRenderer::createCmdList();
    view->drawCmdList = hRenderer::createCmdList();
}

hDrawCallSet allocateDrawCallsForView(hView* view, hUint count) {    
    hcAssert(view >= activeViews.data() && view < activeViews.data() + activeViews.size());
    hDrawCall* dc_ptr = view->reserveDrawCalls(count);
    return hDrawCallSet(dc_ptr ? count : 0, dc_ptr, view);
}

}
}