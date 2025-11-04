#include "Engine/Threading/Thread.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogThread, Warning)

FThread::FThread(): m_name(), m_internalRuntime(nullptr)
{
}

FThread::~FThread()
{
    End();
}

void FThread::Start()
{
}

void FThread::End(bool bShouldWait)
{
    if (m_internalRuntime)
    {
        m_internalRuntime->Stop();
    }

    m_managedThread.request_stop();

    if (bShouldWait && m_managedThread.joinable())
    {
        m_managedThread.join();
    }

    delete m_internalRuntime;
    m_internalRuntime = nullptr;
}

void FThread::Create(const std::string& name, IThreadRuntime* threadCode)
{
    if (m_managedThread.joinable())
    {
        m_managedThread.request_stop();
        m_managedThread.join();
    }

    m_internalRuntime = threadCode;
    m_name = name;

    m_managedThread = std::jthread(thread_runtime, this);
    FPlatform::setThreadName(m_managedThread, m_name);
}

void FThread::PreRun()
{
}

std::uint32_t FThread::Run(std::stop_token stoken)
{
    std::uint32_t exitCode = 1;
    check(m_internalRuntime);

    if (m_internalRuntime->Init(stoken))
    {
        PreRun();
        exitCode = m_internalRuntime->Run();
    }
    else
    {
        m_internalRuntime->Stop();
    }

    LOG(LogThread, Info, "Thread (\"{}\") exited with code {}", m_name, exitCode);
    return exitCode;
}

void FThread::PostRun()
{
}

void FThread::thread_runtime(std::stop_token stoken, FThread* pThis)
{
    check(pThis);
    pThis->Start();
    pThis->Run(stoken);
    pThis->PostRun();
}
