#include "pch.h"

int APIENTRY wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previous_instance, _In_ wchar_t* command_line, _In_ int command_show)
{
    start_hook();
    stop_hook();
    return 0;
}
