#include "Engine/Threading/ThreadPool.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogWorkerThreadRuntime, Warning);
DECLARE_LOGGER_CATEGORY(Core, LogThreadPool, Info);

FThreadPool::FThreadPool()
{
}

void FThreadPool::Start(unsigned i)
{
    LOG(LogThreadPool, Info, "Starting ThreadPool with {} threads", i);
    Resize(i);
}

void FThreadPool::Stop()
{
    LOG(LogThreadPool, Info, "Stopping ThreadPool");
    for (auto& thread: thread_p)
    {
        thread.End();
    }

    state.q_var.notify_all();
    thread_p.Clear();

    // Make sure all work is cleared
    {
        std::unique_lock lock(state.q_mutex);
        while (!state.qWork.empty())
        {
            LOG(LogThreadPool, Warning, "Stopping the ThreadPool while work is still pending !");
            state.qWork.pop();
        }
    }
}

void FThreadPool::Resize(unsigned size)
{
    unsigned old_size = thread_p.Size();
    thread_p.Resize(size);
    for (; old_size < thread_p.Size(); old_size++)
    {
        FWorkerPoolRuntime* Runtime = new FWorkerPoolRuntime(&state);
        thread_p[old_size].Create(std::format("Worker Thread nb {}", old_size), Runtime);
    }
}

std::atomic_int FThreadPool::FWorkerPoolRuntime::s_threadIDCounter = 0;

FThreadPool::FWorkerPoolRuntime::FWorkerPoolRuntime(FThreadPool::FState* context)
    : i_threadID(s_threadIDCounter++)
    , p_state(context)
{
}

bool FThreadPool::FWorkerPoolRuntime::Init(std::stop_token InStoken)
{
    stoken = std::move(InStoken);
    return true;
}

std::uint32_t FThreadPool::FWorkerPoolRuntime::Run()
{
    using namespace std::chrono_literals;
    FThreadPool::WorkUnits work;

    while (!stoken.stop_requested())
    {
        {
            std::unique_lock lock(p_state->q_mutex);
            if (p_state->qWork.empty())
                p_state->q_var.wait_for(lock, 10ms);
            /// lock is owned by this thread when .wait return

            if (p_state->qWork.empty())
                continue;

            work = std::move(p_state->qWork.front());
            p_state->qWork.pop();
        }
        if (work)
        {
            work(i_threadID);
        }
    }
    return 0;
}

void FThreadPool::FWorkerPoolRuntime::Stop()
{
    LOG(LogWorkerThreadRuntime, Info, "Thread {}: exit requested", i_threadID);
}

void FThreadPool::FWorkerPoolRuntime::Exit()
{
    LOG(LogWorkerThreadRuntime, Info, "Thread {}: exit requested", i_threadID);
}
