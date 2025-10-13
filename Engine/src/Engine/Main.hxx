#pragma once

class IApplication;

ENGINE_API int EngineMain(const int ac, const char* const* av, IApplication* (*ApplicationEntryPoint)());
