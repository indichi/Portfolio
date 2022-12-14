#pragma once
#pragma comment(lib, "dbghelp.lib")
#include <Windows.h>
#include <DbgHelp.h>
#include <iostream>
#include <thread>

class CCrashDump
{
public:
    static long _DumpCount;

    CCrashDump()
    {
        _DumpCount = 0;

        _invalid_parameter_handler oldHandler;
        _invalid_parameter_handler newHandler;

        newHandler = myInvalidParameterHandler;

        oldHandler = _set_invalid_parameter_handler(newHandler);    // crt함수에 null포인터 등을 넣었을 때..
        _CrtSetReportMode(_CRT_WARN, 0);                            // CRT 오류 메세지 표시 중단, 바로 덤프 남도록..
        _CrtSetReportMode(_CRT_ASSERT, 0);                          // CRT 오류 메세지 표시 중단, 바로 덤프 남도록..
        _CrtSetReportMode(_CRT_ERROR, 0);                           // CRT 오류 메세지 표시 중단, 바로 덤프 남도록..

        _CrtSetReportHook(_custom_Report_hook);

        // pure virtual function called 에러 핸들러를 사용자 정의 함수로 우회시킨다
        _set_purecall_handler(myPurecallHandler);

        SetHandlerDump();
    }

    
    static void Crash()
    {
        int* p = nullptr;
        *p = 0;
    }

    static void __stdcall WriteDump(PEXCEPTION_POINTERS pExceptionPointer)
    {
        // 현재 날짜와 시간을 알아온다.
        SYSTEMTIME  stNowTime;
        long DumpCount = InterlockedIncrement(&_DumpCount);

        WCHAR filename[MAX_PATH];

        GetLocalTime(&stNowTime);
        wsprintf(filename, L"Dump_%d%02d%02d_%02d.%02d.%02d_%d.dmp", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, DumpCount);

        wprintf(L"\n\n\n!!! Crash Error !!! %d.%d.%d / %d:%d:%d\n", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);
        wprintf(L"Now Save dump file...\n");

        HANDLE hDumpFile = ::CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hDumpFile != INVALID_HANDLE_VALUE)
        {
            _MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation;
            MinidumpExceptionInformation.ThreadId = ::GetCurrentThreadId();
            MinidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
            MinidumpExceptionInformation.ClientPointers = TRUE;

            MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpWithFullMemory, &MinidumpExceptionInformation, NULL, NULL);

            CloseHandle(hDumpFile);

            wprintf(L"Crash Dump Save Finish!\n");
        }
    }

    static LONG WINAPI MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer)
    {
        //int iWorkingMemory = 0;
        
        if (pExceptionPointer->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
        {
            HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)WriteDump, pExceptionPointer, 0, NULL);
            if (hThread == NULL)
                return EXCEPTION_EXECUTE_HANDLER;

            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }
        else
        {
            WriteDump(pExceptionPointer);
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }

    static void SetHandlerDump()
    {
        SetUnhandledExceptionFilter(MyExceptionFilter);
    }

    static void myInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
    {
        Crash();
    }

    static int _custom_Report_hook(int irepossttype, char* message, int* returnvalue)
    {
        Crash();
        return true;
    }

    static void myPurecallHandler(void)
    {
        Crash();
    }

    
};
