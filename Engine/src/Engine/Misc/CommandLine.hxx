#pragma once

namespace ENGINE_API FCommandLine
{

const char* Get();

void Set(const char* CommandLine);
void Set(const int argc, const char* const* const argv);

void Reset();

bool Param(const char* Key);
bool Parse(const char* Key, int& Value);
bool Parse(const char* Key, std::string& Value);

};    // namespace ENGINE_API FCommandLine
