/*
 * SPDX-FileCopyrightText: 2024 Duncan Greatwood
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************
 * pist_check.cc
 *
 * Debugging breakpoints
 *
 */
#include <pistache/winornix.h>

#include <string.h> // memset
#include <map>
#include <mutex>
#include <signal.h>
#include <stdio.h>

#include PST_MISC_IO_HDR // unistd.h e.g. close

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef _IS_WINDOWS
#include <windows.h> // required for dbghelp.h
#include <dbghelp.h>
// See pist_timelog.h for usage
#else
#include <cxxabi.h> // for abi::__cxa_demangle
#include <execinfo.h>
#include <dlfcn.h> // Dl_info
#endif

#include PST_MAXPATH_HDR // for PST_MAXPATHLEN

#include <pistache/ps_basename.h> // for PS_BASENAME_R

#include "pistache/pist_syslog.h"
#include "pistache/pist_check.h"

/*****************************************************************************/

#ifdef _IS_WINDOWS

#include <mutex>

static bool sym_system_inited = false;
static std::mutex sym_system_inited_mutex;

static void logStackTrace(int pri)
{
    PSLogNoLocFn(pri, PS_LOG_AND_STDOUT,
                 "%s", "Stack trace follows...");

    HANDLE process = GetCurrentProcess(); // never fails, apparently

    if (!sym_system_inited)
    {
        std::lock_guard<std::mutex> lock(sym_system_inited_mutex);

        if (!sym_system_inited)
        {
            BOOL syn_init_res = SymInitialize(process,
                                  nullptr, // symbol search path. Use CWD,
                                        // _NT_SYMBOL_PATH environment
                                        // variable, and then
                                        // _NT_ALTERNATE_SYMBOL_PATH
                                  TRUE  // Enumerates the loaded modules for
                                        // the process, calling SymLoadModule64
                                        // on each
                );
            if (!syn_init_res)
            {
                DWORD last_err = GetLastError();

                PS_LOG_INFO_ARGS(
                    "SymInitialize fail, system error code (WinError.h) 0x%x",
                    static_cast<unsigned int>(last_err));
                return;
            }

        sym_system_inited = true;
        }
    }

    SymSetOptions(SYMOPT_UNDNAME); // undecorated names. We call this each
                                   // time, in case some other part of the
                                   // process outside Pistache resets it

    void         * stack[1024+16];
    unsigned short frames = CaptureStackBackTrace(2, // skip first 2 frames
                                                  1024, stack, nullptr);

    if (frames  == 0)
    {
        PS_LOG_DEBUG("Zero frames returned in backtrace");
        return;
    }
    if (frames > 1024)
    {
        PS_LOG_DEBUG("Too many frames?");
        frames = 1024;
    }

    for(unsigned int i = 0; i < frames; i++ )
    {
        const unsigned sym_max_size = 2048;
        unsigned char symbol_and_name[sizeof(SYMBOL_INFO) +
                                      (sym_max_size * sizeof(TCHAR)) + 16];
        memset(&(symbol_and_name[0]), 0, sizeof(symbol_and_name));

        SYMBOL_INFO * symbol =
            reinterpret_cast<SYMBOL_INFO *>(&(symbol_and_name[0]));
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = sym_max_size;

        bool sfa_res = SymFromAddr(process,
              static_cast<DWORD64>(reinterpret_cast<std::uintptr_t>(stack[i])),
              0, symbol);

        const char * name = "{Unknown symbol name}";
        if (!sfa_res)
            name = "{Unknown symbol - SymFromAddr failed}";
        else if (symbol->NameLen)
            name = &(symbol->Name[0]);

        PSLogNoLocFn(pri, PS_LOG_AND_STDOUT, "  ST- %s", name);
    }
}

#else
static void logStackTrace(int pri)
{
    PSLogNoLocFn(pri, PS_LOG_AND_STDOUT,
                 "%s", "Stack trace follows...");

    // write the stack trace to the logging facility

    const int BT_SIZE = 30;
    void *stack[BT_SIZE];
    size_t size = backtrace(stack, BT_SIZE);

    Dl_info info;
    // Start from 1, not 0 since everyone knows we are in PS_Break()
    for (size_t i = 1; i < size; ++i)
    {
        if (!(stack[i]))
        {
            PSLogNoLocFn(pri, PS_LOG_AND_STDOUT,
                         "%s", "  ST- [Null Stack entry] ");
            continue;
        }

        if (dladdr(stack[i], &info) != 0)
        {
            int status = 0;

            // See pist_timelog.h for usage

            char* realname = abi::__cxa_demangle(info.dli_sname, nullptr,
                                                 nullptr, &status);

            if (realname && (realname[0]) && status == 0)
            {
                if (info.dli_saddr)
                {
                    PSLogNoLocFn(pri, PS_LOG_AND_STDOUT,
                                 "  ST- %p:%p %s", info.dli_saddr,
                                 stack[i], realname);
                }
                else
                {
                    PSLogNoLocFn(pri, PS_LOG_AND_STDOUT,
                                 "  ST- %p %s", stack[i], realname);
                }
            }
            else
            {
                if (info.dli_sname)
                    PSLogNoLocFn(pri, PS_LOG_AND_STDOUT,
                                 "  ST- %s", info.dli_sname);
                else if (info.dli_fname)
                    PSLogNoLocFn(pri, PS_LOG_AND_STDOUT,
                                 "  ST- [Unknown addr] %p in %s",
                                 stack[i], info.dli_fname);
                else
                    PSLogNoLocFn(pri, PS_LOG_AND_STDOUT,
                                 "  ST- [Unknown addr] %p in unknown file",
                                 stack[i]);

                // If you're getting a lot of unknown addresses (with
                // info.dli_sname NULL) that you know are in your own code, you
                // probably need to force linker to "add all symbols, not only
                // used ones, to the dynamic symbol table", by using -rdynamic
                // or -export-dynamic for linker flags.
                // https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html
                // http://bit.ly/1KvWpPs (stackoverflow)
            }

            // -1: A memory allocation failure occurred.
            if (realname && status != -1)
                free(realname);
        }
        else
        {
            PSLogNoLocFn(pri, PS_LOG_AND_STDOUT,
                         "  ST- [Unknown addr] %p", stack[i]);
        }
    }
}
#endif // of ifdef _IS_WINDOWS... else...


int PS_LogWoBreak(int pri, const char *p,
                  const char *f, int l, const char *m /* = 0 */)
{
    char buf[1024];
    int ln = 0;
    const char * p_prequote_symbol = (p && (strlen(p))) ? "\"" : "";
    const char * p_postquote_symbol = (p && (strlen(p))) ? "\" @" : "";

    if (m)
        ln = snprintf(buf, sizeof(buf),
                      "PS_LogPt: %s%s%s %s:%d in %s()\n",
                      p_prequote_symbol, p, p_postquote_symbol,
                      f, l, m);
    else
        ln = snprintf(buf, sizeof(buf), "PS_LogPt: %s%s%s %s:%d\n",
                       p_prequote_symbol, p, p_postquote_symbol,
                      f, l);

    // Print it
    if (ln >= (static_cast<int>(sizeof(buf))))
    {
        ln = sizeof(buf);
        buf[sizeof(buf)-2] = '\n';
        buf[sizeof(buf)-1] = 0;
    }

    logStackTrace(pri);

    if ((pri == LOG_EMERG) || (pri == LOG_ALERT) || (pri == LOG_CRIT) ||
        (pri == LOG_ERR))
    {
        fprintf(stderr, "%s", buf); // provide to stderr since "major" issue
    }

    return(1);
}

// ---------------------------------------------------------------------------


#ifdef DEBUG

// class GuardAndDbgLog is used by GUARD_AND_DBG_LOG when DEBUG defined

GuardAndDbgLog::GuardAndDbgLog(const char * mtx_name,
                   unsigned ln, const char * fn,
                   std::mutex * mutex_ptr) :
    mtx_name_(mtx_name), locked_ln_(ln), mutex_ptr_(mutex_ptr)
{
    char buff[PST_MAXPATHLEN+6];
    buff[0] = 0;
    locked_fn_ = std::string(PS_BASENAME_R(fn, &(buff[0])));
}

GuardAndDbgLog::~GuardAndDbgLog()
{
    PS_LOG_DEBUG_ARGS("%s (at %p) unlocked, was locked %s:%u",
                      mtx_name_.c_str(), mutex_ptr_,
                      locked_fn_.c_str(),
                      locked_ln_);
}

#endif
