#include "Engine/Core/Log.hxx"
#include "Engine/Misc/CommandLine.hxx"

#include <cpplogger/sinks/FileSink.hpp>
#include <cpplogger/sinks/StdoutSink.hpp>

FLog::FLog(): CoreLogger("Core")
{
    std::string LogFileLocation;
    if (FCommandLine::Parse("-logfile=", LogFileLocation))
    {
        Sinks.Emplace(new cpplogger::FileSink<FLog::BaseFormatter>(LogFileLocation, false));
    }
    Sinks.Emplace(new cpplogger::StdoutSink<FLog::ColorFormatter>(stdout));

    for (cpplogger::ISink* Sink: Sinks)
    {
        CoreLogger.addSink(Sink);
    }
}

FLog::~FLog()
{
    for (cpplogger::ISink* Sink: Sinks)
    {
        delete Sink;
    }
}
