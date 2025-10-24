#include "Engine/Core/Log.hxx"
#include "Engine/Misc/CommandLine.hxx"

#include <cpplogger/sinks/FileSink.hpp>
#include <cpplogger/sinks/StdoutSink.hpp>

FLog::FLog()
{
    std::string LogFileLocation;
    if (FCommandLine::Parse("-logfile=", LogFileLocation))
    {
        Sinks.Emplace(std::make_unique<cpplogger::FileSink<FLog::BaseFormatter>>(LogFileLocation, false));
    }
    Sinks.Emplace(std::make_unique<cpplogger::StdoutSink<FLog::ColorFormatter>>(stdout));

    CoreLogger = std::make_unique<cpplogger::Logger>("Core");
    for (std::unique_ptr<cpplogger::ISink>& Sink: Sinks)
    {
        CoreLogger->addSink(Sink.get());
    }
}

FLog::~FLog()
{
}
