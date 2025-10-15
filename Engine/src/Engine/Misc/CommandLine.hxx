#pragma once

namespace FCommandLine
{

ENGINE_API const char* Get();

ENGINE_API void Set(const char* CommandLine);
ENGINE_API void Set(const int argc, const char* const* const argv);

ENGINE_API void Reset();

ENGINE_API bool Param(const char* Key);
ENGINE_API bool Parse(const char* Key, int& Value);
ENGINE_API bool Parse(const char* Key, std::string& Value);

};    // namespace FCommandLine
