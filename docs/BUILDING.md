# Building L2InterfaceDecrypt

Step-by-step build for `l2ui.dll`.

> 🇧🇷 Versão em português: [BUILDING.pt-BR.md](BUILDING.pt-BR.md)

---

## Prerequisites

| Tool | Required | Tested |
|------|----------|--------|
| Visual Studio 2022 | C++ Desktop workload | 17.10 |
| CMake | ≥ 3.20 | 3.29 |
| Git | only for cloning the repo (the build itself downloads nothing) | 2.40+ |
| Python (optional) | 3.10+ with `pefile` — for `add_import.py` | 3.11 |

Where to get them:

- Visual Studio 2022 Community → <https://visualstudio.microsoft.com/downloads/> (select **Desktop development with C++**)
- CMake → <https://cmake.org/download/>
- Python → <https://www.python.org/downloads/>

The build no longer uses FetchContent (ImGui/MinHook were removed) —
nothing is downloaded at configure time.

---

## Build

The L2 client is **32-bit**, so `l2ui.dll` must be built for
**Win32**.

```bat
git clone https://github.com/weltongbi/L2InterfaceDecrypt.git
cd L2InterfaceDecrypt
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Release
```

Output: `build\Release\l2ui.dll` (~85 KB, dump only) plus a matching
`l2ui.pdb` for debugging.

### Optional: Debug build

A Release build still ships a full PDB (`/Zi /DEBUG`) and CRT
runtime is static (`/MT`), so the DLL is self-contained — no
`vcruntime140.dll` dependency.

If you want a true Debug binary anyway:

```bat
cmake --build build --config Debug
```

Output: `build\Debug\l2ui.dll`.

---

## Building in VS Code (recommended)

The project ships pre-configured for VS Code (`.vscode/` + `scripts/`).
No need to open Visual Studio anymore.

### Install

1. **MSVC toolchain (required)** — VS Code does not ship a compiler:
   - If Visual Studio 2022/2026 is already installed with the
     **Desktop development with C++** workload, nothing to do.
   - Otherwise install the free **Visual Studio Build Tools** and pick
     the *Desktop development with C++* workload.
2. **CMake ≥ 3.20** — <https://cmake.org/download/>, tick
   *Add CMake to the system PATH* (or use the CMake bundled with VS).
3. **Git** — only needed to clone the repo (the build itself downloads
   nothing).
4. **VS Code extensions** — accept the workspace recommendations:
   - `ms-vscode.cpptools` — C/C++ (IntelliSense)
   - `ms-vscode.cmake-tools` — CMake Tools (optional, see below)
   - `ms-python.python` — Python (optional, for `add_import.py`)

### Build

- **Ctrl+Shift+B** → task **CMake: Compilar** (Build). On the first run
  it configures automatically (Release, x86 via the VS developer
  shell) and then builds. Output: `build\l2ui.dll` + `l2ui.pdb`.
- Extra tasks (**Terminal → Run Task**):
  - **CMake: Configurar (Release/Debug)** — force a reconfigure.
  - **CMake: Limpar** — clean.
  - **IAT: Adicionar import (add_import.py)** — asks for the
    `Engine.dll` path and runs the IAT patch.

> ℹ️ The x86 toolchain is activated automatically by
> `scripts/vscode-cmake.ps1` (VS developer shell with `-arch=x86`),
> so the DLL can never come out x64 by accident.

### Installing into the game folder (manual)

The build no longer touches the game folder. After a successful
build, copy the DLL into the client yourself:

```bat
copy build\l2ui.dll "LineageII\system_en\"
```

### CMake Tools (optional)

If you prefer the **CMake Tools** extension over the tasks: install
`ms-vscode.cmake-tools`, **pick an x86 kit** in the status bar (e.g.
`Visual Studio Community ... - x86`) and configure/build from the
extension. It uses a separate `build-vscode/` folder so it never
clashes with the tasks above.

---

## Common build errors

| Symptom | Fix |
|---------|-----|
| `unresolved external symbol __imp__` | You configured for x64. Re-run with `-A Win32`. |
| `Cannot find Visual Studio 17 2022` | Install the **Desktop development with C++** workload. |
| `vcruntime140.dll missing` at runtime | You changed `CMAKE_MSVC_RUNTIME_LIBRARY` — keep the default `MultiThreaded` (static CRT). |
| `cl.exe not found` / developer shell error | Install the **Desktop development with C++** workload (VS or Build Tools). |
| `D3DERR_DEVICELOST` after Alt+Enter | You modified L2.exe instead of Engine.dll. Revert L2.exe and inject via Engine.dll's IAT. See [USAGE.md](USAGE.md). |

---

## Where to go next

- 📖 [USAGE.md](USAGE.md) — install and use the DLL
- The L2 client side of the install (IAT patch on Engine.dll) is in
  [USAGE.md](USAGE.md) — read that next.
