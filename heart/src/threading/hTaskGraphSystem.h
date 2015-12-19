/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef JOBMANAGER_H__
#define JOBMANAGER_H__

#include "base/hTypes.h"
#include "base/hStringID.h"
#include "base/hArray.h"
#include "base/hMutex.h"
#include "base/hThread.h"
#include "base/hConditionVariable.h"
#include "base/hSemaphore.h"
#include <functional>
#include <vector>

struct lua_State;

namespace Heart {
    struct hTaskInfo {
        // todo... some info
        void* taskInput = nullptr;
    };

    typedef std::function<void(hTaskInfo*)> hTaskProc;
    class hTaskGraph;

namespace hTaskScheduler {
    hBool initialise(hUint32 worker_count, hUint32 job_queue_size);
    void destroy();
}

namespace hTaskFactory {
    void registerTask(hStringID task_name, const hTaskProc& task);
    hTaskProc getTask(hStringID task_name);
    void registerNamedTaskGraph(hStringID task_graph_name);
    hTaskGraph* getNamedTaskGraph(hStringID task_graph_name);
}

    class hTaskHandle {
        hTaskGraph* owner = nullptr;
        hUint firstTaskIndex = -1;
        hUint lastTaskIndex = -1; // If 
        friend class hTaskGraph;
    public:
        hUint32 postWaitingCompleted();

        hBool operator == (const hTaskHandle& rhs) const {
            return owner == rhs.owner && firstTaskIndex == rhs.firstTaskIndex;
        }
    };

    struct hTask {
        hTaskProc work;
        hUint inFlight = 0; // How many of this task have we queued?
        hUint initialWaitingTaskCount = 0; // the number of tasks that need to complete before this can be entered into the task queue.
        hAtomicInt currentWaitingTaskCount; //
        hAtomicInt started; //
        hAtomicInt toSend; // How many of this task should we queue? normally max(taskInputs.size(), 1), atomic to allow to be update mid graph
        hAtomicInt* completed; // set when a worker finishes the job and has notified the dependent tasks.
        std::vector<hTaskHandle> dependentTasks; // list of tasks waiting on our completion
        std::vector<void*> taskInputs; //if not empty task will be run once, otherwise run [taskInputs.size()] times
    };

    class hTaskGraph {
        std::vector<hTask> tasks;
        hSemaphore  graphComplete;
        hAtomicInt  running;
        hAtomicInt  jobsWaiting;
    public:
        hTaskGraph() {
            graphComplete.Create(0, 1);
            hAtomic::AtomicSet(running, 0);
            hAtomic::AtomicSet(jobsWaiting, 0);
        }
        hTaskHandle addTask(std::function<void(hTaskInfo*)> proc);
        hTaskHandle addTask(hStringID task_name) {
            return addTask(hTaskFactory::getTask(task_name));
        }
        hTaskHandle addSubGraph(const hTaskGraph& graph); // TODO: This is quite complex I think but worth doing...I think
        void addTaskInput(hTaskHandle handle, void* in_taskinput);
        void clearTaskInputs(hTaskHandle handle);
        void createTaskDependency(hTaskHandle first, hTaskHandle second);
        void clear();
        void kick();
        void wait() {
            graphComplete.Wait();
            hAtomic::AtomicSet(running, 0);
        }
        bool isRunning() const { return !!hAtomic::AtomicGet(running); }
        bool isComplete() const { return !isRunning(); }
        hTask* getNextAvaibleJob();
        bool internalAllJobsDone() const { return hAtomic::AtomicGet(jobsWaiting) == 0; }
        void internalPostComplete() { graphComplete.Post(); hAtomic::AtomicSet(running, 0); }
        hUint32 internalNotifyWaitingJob(const hTaskHandle& handle) {
            auto ret = hAtomic::Decrement(tasks[handle.firstTaskIndex].currentWaitingTaskCount);
            if (ret == 0) {
                hAtomic::AtomicSet(tasks[handle.firstTaskIndex].toSend, hMax((hInt32)tasks[handle.firstTaskIndex].taskInputs.size(), 1));
            }
            return ret;
        }
    };

    int heart_luaB_register_task_graph(lua_State* L);
}

#endif // JOBMANAGER_H__