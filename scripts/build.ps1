# Build wrapper used by the VS Code task "CMake: Compilar".
# On the first run (no CMake cache) it configures with the given config
# (Release by default), then builds.
param(
    [ValidateSet('Release', 'Debug')]
    [string] $Config = 'Release'
)

$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

if (-not (Test-Path 'build\CMakeCache.txt')) {
    Write-Host "No CMake cache found - configuring ($Config) first..." -ForegroundColor Yellow
    & "$PSScriptRoot\vscode-cmake.ps1" -S . -B build -G Ninja `
        "-DCMAKE_BUILD_TYPE=$Config" `
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

& "$PSScriptRoot\vscode-cmake.ps1" --build build
exit $LASTEXITCODE
