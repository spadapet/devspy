#include "pch.h"
#include "api.h"

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, void* reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

void start_hook(void)
{
}

void stop_hook(void)
{
}
