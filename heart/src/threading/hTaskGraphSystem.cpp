/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "threading/hTaskGraphSystem.h"
#include "base/hStringUtil.h"
#include "base/hMutexAutoScope.h"
#include "lfds/lfds.h"
extern "C" {
#   include "lua.h"
#   include "lualib.h"
#   include "lauxlib.h"
}
#include <algorithm>
#include <unordered_map>

namespace Heart {
namespace hTaskScheduler {

    std::vector<lfds_queue_state*> workerInputQueues;
    std::vector<std::unique_ptr<hThread>> workerThreads;
    lfds_queue_state* graphInputQueue;
    hSemaphore schedulerSemphore;
    hSemaphore schedulerKillSemphore;
    hSemaphore workerSemphore;
    hThread schedulerThread;
    hAtomicInt taskToComplete;
}
    hUint32 hTaskHandle::postWaitingCompleted() {
        return owner->internalNotifyWaitingJob(*this);
    }

    Heart::hTaskHandle hTaskGraph::addTask(std::function<void(hTaskInfo*)> proc) {
        hcAssert(!isRunning());
        hTask new_task;
        new_task.work = proc;
        tasks.emplace_back(std::move(new_task));
        hTaskHandle handle;
        handle.owner = this;
        handle.firstTaskIndex = (hUint32)tasks.size()-1;
        handle.lastTaskIndex = (hUint32)tasks.size() - 1;
        return handle;
    }

    void hTaskGraph::addTaskInput(hTaskHandle handle, void* in_taskinput) {
        hcAssert(!isRunning());
        hcAssertMsg(handle.owner == this && handle.firstTaskIndex < tasks.size(), "Task does not belong to this task graph");
        tasks[handle.firstTaskIndex].taskInputs.push_back(in_taskinput);
        hAtomic::LWMemoryBarrier();
    }

    void hTaskGraph::clearTaskInputs(hTaskHandle handle) {
        tasks[handle.firstTaskIndex].taskInputs.clear();
    }

    void hTaskGraph::createTaskDependency(hTaskHandle first, hTaskHandle second) {
        hcAssertMsg(first.owner == this && first.firstTaskIndex < tasks.size(), "First task does not belong to this task graph");
        hcAssertMsg(second.owner == this && second.firstTaskIndex < tasks.size(), "Second task does not belong to this task graph");
        if (std::find(tasks[first.firstTaskIndex].dependentTasks.begin(), tasks[first.firstTaskIndex].dependentTasks.end(), second) == tasks[first.firstTaskIndex].dependentTasks.end()) {
            tasks[first.firstTaskIndex].dependentTasks.push_back(second);
            ++tasks[second.firstTaskIndex].initialWaitingTaskCount;
        }
    }

    void hTaskGraph::clear() {
        tasks.clear();
    }

    void hTaskGraph::kick() {
        for (auto& i : tasks) {
            i.completed = &jobsWaiting;
            hAtomic::AtomicSet(i.currentWaitingTaskCount, i.initialWaitingTaskCount);
            hAtomic::AtomicSet(i.started, 0);
            if (i.initialWaitingTaskCount == 0)
                hAtomic::AtomicSet(i.toSend, hMax((hInt32)i.taskInputs.size(), 1));
            else// As this task waits on others, set this value to something that'll ensure it's never sent to a worker. Allows for
                hAtomic::AtomicSet(i.toSend, -1);// an earlier task to set up tasks for a later task.
            i.inFlight = 0;
        }
        hAtomic::AtomicSet(running, 1);
        hAtomic::AtomicSet(jobsWaiting, (hUint32)tasks.size());
        lfds_queue_enqueue(hTaskScheduler::graphInputQueue, this);
        hTaskScheduler::schedulerSemphore.Post();
    }

    hTask* hTaskGraph::getNextAvaibleJob() {
        for (hSize_t i=0, n=tasks.size(); i<n; ++i) {
            if (hAtomic::AtomicGet(tasks[i].currentWaitingTaskCount) == 0) {
                hInt32 toSend = hAtomic::AtomicGet(tasks[i].toSend);
                hcAssert((hInt32)tasks[i].inFlight <= toSend || toSend < 0);
                if ((hInt32)tasks[i].inFlight < toSend) {
                    ++tasks[i].inFlight;
                    return &tasks[i];
                }
            }
        }
        return nullptr;
    }

namespace hTaskScheduler {

    // There are probably better ways to handle this but it'll do for now
    hUint32 schedulerProcess(void* null) {
        std::vector<hTaskGraph*> graphsToProcess;
        std::vector<hTask*> tasksToQueue;
        hAtomic::AtomicSet(taskToComplete, 0);
        lfds_queue_use(graphInputQueue);
        lfds_queue_use(workerInputQueues[0]);
        while(!schedulerKillSemphore.poll()) {
            schedulerSemphore.Wait();
            // Check the input queue for new graphs to process. (i.e. hTaskGraph.kick() called)
            hTaskGraph* graph_ptr;
            while (lfds_queue_dequeue(graphInputQueue, (void**)&graph_ptr)) {
                for (auto& i : graphsToProcess) {
                    if (!i) {
                        i = graph_ptr;
                        graph_ptr = nullptr;
                        break;
                    }
                }
                if (graph_ptr) graphsToProcess.push_back(graph_ptr);
            }

            //if (!graphsToProcess.empty() && !hAtomic::AtomicGet(taskToComplete)) {
            //    hAtomic::AtomicSet(taskToComplete, 1);
            //}

            for (auto& i : graphsToProcess) {
                if (i && i->internalAllJobsDone()) {
                    i->internalPostComplete();
                    i = nullptr;
                }
            }

            //if (graphsToProcess.empty() && hAtomic::AtomicGet(taskToComplete)) {
            //    hAtomic::AtomicSet(taskToComplete, 0);
            //}

            for (auto& i : graphsToProcess) {
                if (!i) continue;
                while(auto* t = i->getNextAvaibleJob()) {
                    lfds_queue_enqueue(workerInputQueues[0], (void*)t);//tasksToQueue.push_back(t);
                    workerSemphore.Post();
                }
            }

            /*
            for (auto& i : tasksToQueue) {
                for (auto& q : workerInputQueues) {
                    //
                    lfds_queue_enqueue(q, (void*)i);
                }
            }
            tasksToQueue.clear();
            */
        }
        return 0;
    }

    hUint32 workerProcess(void* workerQueuePtr) {
        lfds_queue_state* workerQueue = (lfds_queue_state*)workerQueuePtr;
        lfds_queue_use(workerQueue);
        while(1) {
            workerSemphore.Wait();
            //Grab an task and run it
            hTask* task = nullptr;
            if (lfds_queue_dequeue(workerQueue, (void**)&task)) {
                auto task_index = hAtomic::Increment(task->started);
                hcAssert(task_index <= hAtomic::AtomicGet(task->toSend));
                hTaskInfo info;
                info.taskInput = task->taskInputs.empty() ? nullptr : task->taskInputs[task_index-1];
                task->work(&info);

                if (task_index == hAtomic::AtomicGet(task->toSend)) { // Was this the last task of this type that needed to be run.
                    for (auto& i : task->dependentTasks) {
                        if (i.postWaitingCompleted() == 0) {
                            schedulerSemphore.Post();
                        }
                    }
                    if (hAtomic::Decrement(*task->completed) == 0)
                        schedulerSemphore.Post();
                }
            }
        }
        return 0;
    }

    hBool initialise(hUint32 worker_count, hUint32 job_queue_size) {
        hUint32 processor_count = worker_count;
        
        workerInputQueues.resize(1);
        workerThreads.resize(processor_count);
        workerSemphore.Create(0, processor_count);
        schedulerSemphore.Create(0, 1);
        schedulerKillSemphore.Create(0, 1);
        hAtomic::AtomicSet(taskToComplete, 0);
        lfds_queue_new(&graphInputQueue, job_queue_size);
        lfds_queue_new(&workerInputQueues[0], job_queue_size);
        for (hUint32 i=0; i<processor_count; ++i) {
            hChar name[128];
            hStrPrintf(name, sizeof(name), "Worker Thread %d", i+1);
            workerThreads[i].reset(new hThread());
            workerThreads[i]->create(name, 0, workerProcess, workerInputQueues[0]);
        }

        schedulerThread.create("Task Graph Scheduler", 0, schedulerProcess, nullptr);

        return hTrue;
    }

    void destroy() {
        schedulerKillSemphore.Post();
        schedulerThread.join();
    }

}

namespace hTaskFactory {

    std::unordered_map<hStringID, hTaskProc> taskRegistry;
    std::unordered_map<hStringID, std::unique_ptr<hTaskGraph>> taskGraphRegistry;

    void registerTask(hStringID task_name, const hTaskProc& task) {
        auto& e = taskRegistry[task_name];
        e = task;
    }

    Heart::hTaskProc getTask(hStringID task_name) {
        return taskRegistry[task_name];
    }

    void registerNamedTaskGraph(hStringID task_graph_name) {
        taskGraphRegistry[task_graph_name].reset(new hTaskGraph());
    }

    hTaskGraph* getNamedTaskGraph(hStringID task_graph_name) {
        const auto& r = taskGraphRegistry[task_graph_name];
        return r.get();
    }

}

int heart_luaB_register_task_graph(lua_State* L);
int heart_luaB_create_task_graph(lua_State* L);
int heart_luaB_add_task_graph_task(lua_State* L);
int heart_luaB_add_task_graph_dependency(lua_State* L);
int heart_luaB_add_task_graph_order(lua_State* L);

static const luaL_Reg heartTaskGraphMethods[] = {
    { "add_task_to_graph", heart_luaB_add_task_graph_task },
    { "add_task_order_to_graph", heart_luaB_add_task_graph_order },
    { "add_dependency_to_graph", heart_luaB_add_task_graph_dependency },
    { nullptr, nullptr }
};

static const luaL_Reg heartTaskGraphLib[] = {
    { "create_task_graph", heart_luaB_create_task_graph },
    { nullptr, nullptr }
};

int heart_luaB_register_task_graph(lua_State* L) {
    luaL_newmetatable(L, "Heart.TaskGraphRef");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, heartTaskGraphMethods, 0);
    lua_pop(L, 1); // remove the metatable

    luaL_newmetatable(L, "Heart.TaskGraphHandle");
    lua_pop(L, 1); // remove the metatable

    // set create in the global table
    lua_pushglobaltable(L);
    luaL_setfuncs(L, heartTaskGraphLib, 0);

    return 0;
}

struct hTaskGraphRef {
    hTaskGraph* graph;
    char* name;
};

static int heart_luaB_create_task_graph(lua_State* L) {
    size_t l = 0;
    const char* n = luaL_checklstring(L, 1, &l);
    auto* ud = (hTaskGraphRef*)lua_newuserdata(L, sizeof(hTaskGraphRef)+l+1);
    auto nid = hStringID(n);
    hTaskFactory::registerNamedTaskGraph(nid);
    ud->graph = hTaskFactory::getNamedTaskGraph(nid);
    ud->name = (char*)(ud+1);
    strcpy(ud->name, n);
    luaL_getmetatable(L, "Heart.TaskGraphRef");
    lua_setmetatable(L, -2);
    return 1;
}

static int heart_luaB_add_task_graph_task(lua_State* L) {
    hTaskGraphRef* tg = (hTaskGraphRef*)luaL_checkudata(L, 1, "Heart.TaskGraphRef");
    const char* tn = luaL_checkstring(L, 2);
    auto* ud = (hTaskHandle*)lua_newuserdata(L, sizeof(hTaskHandle));
    *ud = tg->graph->addTask(hStringID(tn));
    luaL_getmetatable(L, "Heart.TaskGraphHandle");
    lua_setmetatable(L, -2);
    return 1;
}

static int heart_luaB_add_task_graph_order(lua_State* L) {
    hTaskGraphRef* tg = (hTaskGraphRef*)luaL_checkudata(L, 1, "Heart.TaskGraphRef");
    hTaskHandle* p = nullptr;
    for (int i=2, n=lua_gettop(L); i<=n; ++i) {
        auto* h = (hTaskHandle*)luaL_checkudata(L, i, "Heart.TaskGraphHandle");
        if (p) {
            tg->graph->createTaskDependency(*p, *h);
        }
        p = h;
    }
    return 0;
}

static int heart_luaB_add_task_graph_dependency(lua_State* L) {
    hTaskGraphRef* tg = (hTaskGraphRef*)luaL_checkudata(L, 1, "Heart.TaskGraphRef");
    hTaskHandle* p = (hTaskHandle*)luaL_checkudata(L, 2, "Heart.TaskGraphHandle");;
    for (int i = 3, n = lua_gettop(L); i <= n; ++i) {
        auto* h = (hTaskHandle*)luaL_checkudata(L, i, "Heart.TaskGraphHandle");
        tg->graph->createTaskDependency(*p, *h);
    }
    return 0;
}

}