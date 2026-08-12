cmake_minimum_required(VERSION 3.20)
project(L2InterfaceDecrypt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# -----------------------------------------------------------------------------
# O cliente L2 é 32-bit: a l2ui.dll TEM que ser Win32 (x86).
# Falha a configuração imediatamente se o toolchain for x64.
# (As tasks do VS Code já forçam x86 via scripts/vscode-cmake.ps1;
#  esta checagem é a rede de segurança pra qualquer outra forma de config.)
# -----------------------------------------------------------------------------
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR
        "l2ui.dll deve ser compilada como Win32 (x86), mas o toolchain detectado e x64. "
        "Configure com o toolchain x86: use as tasks do VS Code "
        "(CMake: Configurar (Release)) ou 'cmake -A Win32' no Visual Studio.")
endif()

# -----------------------------------------------------------------------------
# l2ui.dll — loaded into L2.exe via an IAT entry pointing at
# l2ui.dll!L2UI_Init (use CFF Explorer or `add_import.py` to wire it up).
# Dump-only build: dumps Interface.xdat via Core.dll, no hooks/overlay.
# -----------------------------------------------------------------------------
add_library(l2ui SHARED
    src/dllmain.cpp
)
set_target_properties(l2ui PROPERTIES
    PREFIX ""
    OUTPUT_NAME "l2ui"
    SUFFIX ".dll"
)
target_compile_definitions(l2ui PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
    _CRT_SECURE_NO_WARNINGS
)
if(MSVC)
    target_compile_options(l2ui PRIVATE /guard:cf- /Zi /fp:precise)
    set_target_properties(l2ui PROPERTIES
        LINK_FLAGS "/GUARD:NO /MAP /DEBUG /OPT:REF /OPT:ICF"
    )
endif()
