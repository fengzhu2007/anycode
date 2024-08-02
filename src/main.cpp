#include <QApplication>
#include <QTranslator>
#include <QDateTime>
#include "mainwindow.h"
#include "dockwindow.h"
#include "idewindow.h"
#include <QDebug>
#ifdef Q_OS_WIN
#include <tchar.h>
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "user32.lib")
#endif




#ifdef Q_OS_WIN
int GenerateMiniDump(PEXCEPTION_POINTERS pExceptionPointers)
{
    typedef BOOL(WINAPI * MiniDumpWriteDumpT)(
        HANDLE,
        DWORD,
        HANDLE,
        MINIDUMP_TYPE,
        PMINIDUMP_EXCEPTION_INFORMATION,
        PMINIDUMP_USER_STREAM_INFORMATION,
        PMINIDUMP_CALLBACK_INFORMATION
        );
    MiniDumpWriteDumpT pfnMiniDumpWriteDump = NULL;
    HMODULE hDbgHelp = LoadLibraryW(L"DbgHelp.dll");
    if (NULL == hDbgHelp)
    {
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    pfnMiniDumpWriteDump = (MiniDumpWriteDumpT)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

    if (NULL == pfnMiniDumpWriteDump)
    {
        FreeLibrary(hDbgHelp);
        return EXCEPTION_CONTINUE_EXECUTION;


    }
    wchar_t szFileName[MAX_PATH] = { 0 };
    wchar_t szVersion[] = L"DumpFile";
    SYSTEMTIME stLocalTime;
    GetLocalTime(&stLocalTime);
    wsprintfW(szFileName, L"%s-%04d%02d%02d-%02d%02d%02d.dmp",
        szVersion, stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
        stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond);
    HANDLE hDumpFile = CreateFileW(szFileName, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
    if (INVALID_HANDLE_VALUE == hDumpFile)
    {
        FreeLibrary(hDbgHelp);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    MINIDUMP_EXCEPTION_INFORMATION expParam;
    expParam.ThreadId = GetCurrentThreadId();
    expParam.ExceptionPointers = pExceptionPointers;
    expParam.ClientPointers = FALSE;
    pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
        hDumpFile, MiniDumpWithDataSegs, (pExceptionPointers ? &expParam : NULL), NULL, NULL);
    CloseHandle(hDumpFile);
    FreeLibrary(hDbgHelp);
    return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo)
{
    if (IsDebuggerPresent()) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    return GenerateMiniDump(lpExceptionInfo);
}
#endif


int main(int argc, char *argv[])
{

    #ifdef Q_OS_WIN
    #ifndef Q_DEBUG
        SetUnhandledExceptionFilter(ExceptionFilter);
    #endif
    #endif

    QApplication a(argc, argv);
    QString dir = QCoreApplication::applicationDirPath();
    {
        QTranslator* translator = new QTranslator(&a);
        if(translator->load(dir + "/langs/App.zh_CN.qm")){
            a.installTranslator(translator);
        }
    }
    {
        QTranslator* translator = new QTranslator(&a);
        if(translator->load(dir + "/langs/qt_zh_CN.qm")){
            a.installTranslator(translator);
        }
    }

    {
        QTranslator* translator = new QTranslator(&a);
        if(translator->load(dir + "/langs/qt_help_zh_CN.qm")){
            a.installTranslator(translator);
        }
    }
    //QFont globalFont("Microsoft YaHei", 10);
    //a.setFont(globalFont);
    /*{
        QTranslator* translator = new QTranslator(&a);
        if(translator->load(dir + "/langs/qtbase_zh_TW.qm")){
            a.installTranslator(translator);
        }
    }*/

    //ady::MainWindow w;
    ady::IDEWindow w;
    //w.setLocale(QLocale::Chinese);
    //w.showMaximized();
    w.show();
    return a.exec();
}





