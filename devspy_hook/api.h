#pragma once

#ifdef DEVSPY_HOOK_DLL
#define DEVSPY_API __declspec(dllexport)
#else
#define DEVSPY_API __declspec(dllimport)
#endif

DEVSPY_API void start_hook(void);
DEVSPY_API void stop_hook(void);

#undef DEVSPY_API
