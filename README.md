# L2InterfaceDecrypt

Dump-only helper DLL for Lineage II clients. Drop-in `l2ui.dll`
that side-loads into the client and decrypts a configurable list of
`.xdat`/`.u` files through the game's own `FFileManager` API,
writing the results to a `decrypted` folder next to the DLL.

**🇧🇷 Versão em português:** [README.pt-BR.md](README.pt-BR.md)

---

## ⚠️ Disclaimer

This project is **not affiliated with, endorsed by, or sponsored by
NCSoft**. "Lineage II" is a trademark of NCSoft Corporation.

This software is intended **exclusively for use on private servers**
that you operate or are authorized to participate in. Using it on
official NCSoft servers may violate their Terms of Service. The
authors take no responsibility for accounts, characters, or actions
taken by third parties using this software.

The repository contains **no copyrighted game assets** — no L2
textures, audio, or game data is redistributed. The DLL calls client
functions at runtime; decrypted files never leave the machine that
ran the client.

Use at your own risk.

---

## Features

- **Bulk file dump** — the `kDumpList` array in `src/dllmain.cpp`
  lists files by their original names (`Interface.xdat`,
  `Interface.u`, `Core.u`, `Engine.u`, `NWindow.u`).
- **Optional `l2ui.ini`** — create `l2ui.ini` next to the DLL with a
  `[files]` section (`file1=`, `file2=`, …) to override the built-in
  list without recompiling.
- **Decrypts via the game itself** — loads each file with Core.dll's
  `appLoadFileToArray` and saves it with `appSaveArrayToFile`
  through the game's `GFileManager`, so the output is already
  decrypted.
- **Self-contained output** — dumped files and the log
  (`overlay.log`) land in a `decrypted` folder created next to
  `l2ui.dll`.
- **Themida-safe** — injects via Engine.dll's IAT, never modifies
  L2.exe (which is Themida-protected and breaks on tamper).

## Quick start

```bat
:: Build (needs VS 2022/2026 with C++ workload + CMake 3.20+)
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Release

:: Output: build/Release/l2ui.dll  (~85 KB)

:: Install (one-time per L2 install)
copy "L2\System_en\Engine.dll" "L2\System_en\Engine.dll.original"
copy build\Release\l2ui.dll  "L2\System_en\"
python add_import.py "L2\System_en\Engine.dll"

:: Launch L2 normally — files appear in L2\System_en\decrypted\
```

Full step-by-step guides:

- 🇺🇸 [**docs/BUILDING.md**](docs/BUILDING.md) — compile from source (VS or VS Code)
- 🇺🇸 [**docs/USAGE.md**](docs/USAGE.md) — install, configure, use
- 🇧🇷 [**docs/BUILDING.pt-BR.md**](docs/BUILDING.pt-BR.md) — guia de compilação
- 🇧🇷 [**docs/USAGE.pt-BR.md**](docs/USAGE.pt-BR.md) — guia de uso

## How it works

```
┌─────────────┐  IAT import    ┌───────────────┐                    ┌──────────┐
│  L2.exe     │───────────────►│  Engine.dll   │───────────────────►│ l2ui.dll │
│ (Themida)   │  (untouched)   │  (untouched)  │   L2UI_Init        │  dump    │
└─────────────┘                └───────────────┘                    └────┬─────┘
                                                                         │
                                                     appLoadFileToArray │ Core.dll
                                                     appSaveArrayToFile ▼
                                                                    ┌──────────┐
                                                                    │ Core.dll │
                                                                    └──────────┘
```

- A single `L2UI_Init` export is added to Engine.dll's import table
  via CFF Explorer or the bundled Python script. Windows loads
  `l2ui.dll` when Engine.dll's imports are resolved.
- Once `WinDrv.dll` is present, the DLL calls Core.dll's
  file-manager API for every file in `kDumpList` — or in
  `l2ui.ini`, when that file exists next to the DLL — and saves the
  decrypted content to `<game>\System_en\decrypted\<name>`.
- `overlay.log`, in the same `decrypted` folder, records each
  `source -> destination` pair and a final `concluido (n/m)` line.

## Project layout

```
.
├── LICENSE                  MIT
├── README.md                this file (English)
├── README.pt-BR.md          Portuguese version
├── docs/
│   ├── BUILDING.md          build guide (EN)
│   ├── BUILDING.pt-BR.md    guia de compilação
│   ├── USAGE.md             usage guide (EN)
│   └── USAGE.pt-BR.md       guia de uso
├── CMakeLists.txt           top-level build
├── add_import.py            CFF Explorer alternative (pefile-based)
├── .vscode/                 VS Code tasks/settings
├── scripts/                 VS Code build helpers (MSVC x86 env)
└── src/
    └── dllmain.cpp          DllMain, log, dump list, L2UI_Init export
```

## Tech stack

| Component | Library | License |
|-----------|---------|---------|
| Compiler | MSVC (VS 2022/2026, x86 toolchain) | — |
| Build | CMake ≥ 3.20 + Ninja | BSD-3-Clause / Apache-2.0 |
| Decryption | none — done by the game (Core.dll) | — |

## License

MIT — see [LICENSE](LICENSE).

