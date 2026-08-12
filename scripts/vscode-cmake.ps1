# Runs CMake inside the Visual Studio x86 developer environment so the
# Win32 toolchain (cl.exe / lib.exe / link.exe) is configured correctly.
# The L2 client is 32-bit, so l2ui.dll MUST be built with the x86 toolchain.
#
# Usage: vscode-cmake.ps1 [cmake args...]
#   powershell -File scripts\vscode-cmake.ps1 -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]] $CmakeArgs
)

$ErrorActionPreference = 'Stop'

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) {
    Write-Host 'vswhere.exe not found.' -ForegroundColor Red
    Write-Host 'Install Visual Studio 2022/2026 (or Build Tools) with the "Desktop development with C++" workload.' -ForegroundColor Red
    exit 1
}

$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) {
    Write-Host 'No Visual Studio installation with C++ tools found.' -ForegroundColor Red
    Write-Host 'Install the "Desktop development with C++" workload.' -ForegroundColor Red
    exit 1
}
Write-Host "Visual Studio: $vsPath"

# Enter the VS developer shell with the x86 toolchain (needed for a Win32 DLL)
$devShellDll = Join-Path $vsPath 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll'
Import-Module $devShellDll
Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation -DevCmdArguments '-arch=x86 -host_arch=x64' | Out-Null

# Put the VS-bundled Ninja on PATH
$ninjaDir = Join-Path $vsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja'
if (Test-Path $ninjaDir) {
    $env:PATH = "$ninjaDir;$env:PATH"
}

# Prefer cmake from PATH, fall back to the VS-bundled one
$cmake = (Get-Command cmake -ErrorAction SilentlyContinue).Source
if (-not $cmake) {
    $bundled = Join-Path $vsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
    if (Test-Path $bundled) { $cmake = $bundled }
}
if (-not $cmake) {
    Write-Host 'cmake not found.' -ForegroundColor Red
    Write-Host 'Install CMake >= 3.20 from https://cmake.org/download/ (check "Add CMake to the system PATH").' -ForegroundColor Red
    exit 1
}

Write-Host "cmake: $cmake"
& $cmake @CmakeArgs
exit $LASTEXITCODE
