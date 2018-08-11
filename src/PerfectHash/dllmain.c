/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    dllmain.c

Abstract:

    This is the DLL main entry point for the PerfectHash component.  It hooks
    into process and thread attach and detach messages in order to provide TLS
    glue (see PerfectHashTls.c for more information).

--*/

#include "stdafx.h"

HMODULE PerfectHashModule;

BOOL
APIENTRY
_DllMainCRTStartup(
    _In_    HMODULE     Module,
    _In_    DWORD       Reason,
    _In_    LPVOID      Reserved
    )
{
    switch (Reason) {
        case DLL_PROCESS_ATTACH:
            __security_init_cookie();
            PerfectHashModule = Module;
            if (!PerfectHashTlsProcessAttach(Module, Reason, Reserved)) {
                return FALSE;
            }
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            if (!PerfectHashTlsProcessDetach(Module, Reason, Reserved)) {
                NOTHING;
            }
            break;
    }

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :