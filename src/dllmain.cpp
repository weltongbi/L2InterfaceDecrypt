// l2ui.dll — dump-only helper for the L2 client.
//
// Loaded into L2.exe via an IAT entry pointing at l2ui.dll!L2UI_Init
// (use add_import.py or CFF Explorer on Engine.dll). Once WinDrv.dll is
// detected, it dumps every file listed in kDumpList below via Core.dll's
// FFileManager API. Dumped files and the log (overlay.log) are saved to a
// "decrypted" folder next to l2ui.dll.
// No hooks, no UI, no ImGui.
// (WIN32_LEAN_AND_MEAN já vem do CMakeLists — não redefinir aqui)

#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cwchar>

extern "C"
{
    // Força o compilador do MSVC a incluir bibliotecas de ponto flutuante na compilação.
    // Isso previne o erro C Runtime Error R6002 por otimização agressiva.
    int _fltused = 1;
}

// Flag de debug global
bool g_isDebugEnabled = false; // Defina como true quando precisar de logs de debug

// -----------------------------------------------------------------------------
// LISTA DE ARQUIVOS PARA DUMP — edite aqui.
// Somente o nome original do arquivo (sem caminho).
// O carregamento usa "..\\system\\<nome>" (como o jogo enxerga) e o
// resultado é salvo em "<pasta da l2ui.dll>\\decrypted\\<nome>".
// -----------------------------------------------------------------------------
static const wchar_t *kDumpList[] = {
    L"Interface.xdat",
    L"Interface.u",
    L"Core.u",
    L"Engine.u",
    L"NWindow.u",
};

// Diretório de saída: <pasta da l2ui.dll>\decrypted (criado se não existir).
// Tanto o log quanto os arquivos dumpados ficam aqui.
static const wchar_t *GetDecryptedDir()
{
    static wchar_t s_dir[MAX_PATH] = {0};
    if (s_dir[0])
        return s_dir;

    wchar_t dllPath[MAX_PATH] = {0};
    GetModuleFileNameW(GetModuleHandleW(L"l2ui.dll"), dllPath, MAX_PATH);
    wchar_t *slash = wcsrchr(dllPath, L'\\');
    if (slash)
        *slash = L'\0'; // fica só o diretório da DLL

    _snwprintf(s_dir, MAX_PATH, L"%s\\decrypted", dllPath);
    CreateDirectoryW(s_dir, nullptr); // ok se já existir
    return s_dir;
}

// Caminho do log: <pasta da l2ui.dll>\decrypted\overlay.log (lazy, 1x por processo).
static const char *GetLogPath()
{
    static char s_path[MAX_PATH] = {0};
    if (s_path[0])
        return s_path;
    _snprintf(s_path, sizeof(s_path), "%ls\\overlay.log", GetDecryptedDir());
    return s_path;
}

// Shared log writer (timestamp + pid/tid).
static void LogToFileV(const char *fmt, va_list ap)
{
    FILE *f = fopen(GetLogPath(), "a");
    if (!f)
        return;
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(f, "[%02d:%02d:%02d.%03d pid=%lu tid=%lu] ",
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
            GetCurrentProcessId(), GetCurrentThreadId());
    vfprintf(f, fmt, ap);
    fputc('\n', f);
    fclose(f);
}

// Logf: só em build Debug.
extern "C" void Logf(const char *fmt, ...)
{
#ifndef NDEBUG
    va_list ap;
    va_start(ap, fmt);
    LogToFileV(fmt, ap);
    va_end(ap);
#endif // NDEBUG
}

// DumpLog: sempre ativo — o progresso do dump precisa aparecer em Release.
static void DumpLog(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    LogToFileV(fmt, ap);
    va_end(ap);
}

namespace
{
    void LogProcessInfo()
    {
        wchar_t exePath[MAX_PATH] = {0};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        wchar_t dllPath[MAX_PATH] = {0};
        GetModuleFileNameW(GetModuleHandleW(L"l2ui.dll"), dllPath, MAX_PATH);
        Logf("=== l2ui.dll ATTACH ===");
        Logf("host exe : %ls", exePath);
        Logf("self dll : %ls", dllPath);
        Logf("cmdline  : %ls", GetCommandLineW());
        Logf("modules visible at attach time:");
        static const wchar_t *const kProbeModules[] = {
            L"kernel32.dll",
            L"user32.dll",
            L"Core.dll",
            L"Engine.dll",
            L"Window.dll",
            L"NWindow.dll",
            L"D3DDrv.dll",
            L"steam_api.dll",
        };
        for (size_t i = 0; i < sizeof(kProbeModules) / sizeof(kProbeModules[0]); ++i)
        {
            HMODULE h = GetModuleHandleW(kProbeModules[i]);
            Logf("  %-14ls -> %p", kProbeModules[i], (void *)h);
        }
    }

} // namespace

// Constrói o caminho de saída: <pasta da l2ui.dll>\decrypted\<nome>.
static void BuildDstPath(wchar_t *out, size_t outChars, const wchar_t *name)
{
    _snwprintf(out, outChars, L"%s\\%s", GetDecryptedDir(), name);
}

static bool DumpOne(const wchar_t *name)
{
    typedef void(__cdecl * f_appLoadFileToArray)(char *, const wchar_t *, int);
    typedef void(__cdecl * f_appSaveArrayToFile)(char *, const wchar_t *, int);

    // Resolve as APIs do Core.dll uma única vez.
    static f_appLoadFileToArray appLoadFileToArray = nullptr;
    static f_appSaveArrayToFile appSaveArrayToFile = nullptr;
    static int *gFileManager = nullptr;
    static bool resolved = false;

    if (!resolved)
    {
        HMODULE hCore = GetModuleHandleA("Core.dll");
        if (!hCore)
        {
            DumpLog("dump: Core.dll ainda nao carregada");
            return false;
        }

        FARPROC pLoad = GetProcAddress(hCore, "?appLoadFileToArray@@YAHAAV?$TArray@E@@PB_WPAVFFileManager@@@Z");
        FARPROC pSave = GetProcAddress(hCore, "?appSaveArrayToFile@@YAHABV?$TArray@E@@PB_WPAVFFileManager@@@Z");
        FARPROC pFM = GetProcAddress(hCore, "?GFileManager@@3PAVFFileManager@@A");

        if (!pLoad || !pSave || !pFM)
        {
            DumpLog("dump: APIs do Core.dll nao encontradas (appLoad/appSave/GFileManager)");
            return false;
        }

        appLoadFileToArray = (f_appLoadFileToArray)pLoad;
        appSaveArrayToFile = (f_appSaveArrayToFile)pSave;
        gFileManager = (int *)pFM;
        resolved = true;
    }

    wchar_t srcPath[MAX_PATH];
    _snwprintf(srcPath, MAX_PATH, L"..\\system\\%s", name);

    wchar_t dstPath[MAX_PATH];
    BuildDstPath(dstPath, MAX_PATH, name);

    char TArray[0x14];
    memset(TArray, 0, 0x14);

    appLoadFileToArray(TArray, srcPath, *gFileManager);
    // TArray<T> em memória: [0]=ponteiro de dados, [4]=ArrayNum, [8]=ArrayMax
    int loadedBytes = *(int *)(TArray + 4);

    appSaveArrayToFile(TArray, dstPath, *gFileManager);

    DWORD savedBytes = 0;
    WIN32_FILE_ATTRIBUTE_DATA attr = {0};
    if (GetFileAttributesExW(dstPath, GetFileExInfoStandard, &attr))
        savedBytes = attr.nFileSizeLow;

    DumpLog("dump: %ls -> %ls (carregado=%d bytes, gravado=%lu bytes)", srcPath, dstPath, loadedBytes, savedBytes);
    return true;
}

static void DumpAllFiles()
{
    int ok = 0;
    int total = (int)(sizeof(kDumpList) / sizeof(kDumpList[0]));
    for (int i = 0; i < total; ++i)
    {
        if (DumpOne(kDumpList[i]))
            ++ok;
    }
    DumpLog("dump: concluido (%d/%d arquivos)", ok, total);
}

bool dumped = false;
static void StartCheck()
{
    // Wait until WinDrv.dll is loaded so we know the engine is fully
    // initialized before touching Core.dll's file manager.
    if (GetModuleHandleA("WinDrv.dll") != NULL)
    {
        if (!dumped)
        {
            DumpAllFiles();
            dumped = true;
        }
    }
}

// Named export so CFF Explorer (or any IAT editor) can wire L2.exe's
// import table to "l2ui.dll!L2UI_Init". Windows loader only needs to be
// able to FIND the export — it doesn't have to call it before our
// DllMain runs (DllMain is called as part of LoadLibrary). The function
// is therefore a permanent no-op.
extern "C" __declspec(dllexport) void __cdecl L2UI_Init()
{
    Logf("L2UI_Init called (someone invoked the named export — usually safe to ignore)");
}

BOOL APIENTRY DllMain(HINSTANCE hDll, DWORD reason, LPVOID)
{
    // DLL_THREAD_ATTACH is the trigger: when a thread attaches and
    // WinDrv.dll is already loaded, run the dump list once.
    if (reason == DLL_THREAD_ATTACH)
    {
        StartCheck();
    }

    if (reason == DLL_PROCESS_ATTACH)
    {
        // NOTE: don't call DisableThreadLibraryCalls() — the dump relies
        // on receiving DLL_THREAD_ATTACH notifications.
#ifndef NDEBUG
        // Clear previous log upon new process attach
        FILE *f = fopen(GetLogPath(), "w");
        if (f)
            fclose(f);
#endif // NDEBUG
        LogProcessInfo();
        Logf("DllMain ATTACH complete (waiting for WinDrv.dll to run the dump list)");
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        Logf("=== l2ui.dll DETACH ===");
    }
    return TRUE;
}
