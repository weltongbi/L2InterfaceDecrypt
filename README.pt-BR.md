# L2InterfaceDecrypt

DLL auxiliar somente-dump para clientes Lineage II. `l2ui.dll`
drop-in que side-loada no cliente e decripta uma lista configurável
de arquivos `.xdat`/`.ui` usando a própria API `FFileManager` do
jogo, gravando os resultados numa pasta `decrypted` ao lado da DLL.

**🇺🇸 English version:** [README.md](README.md)

---

## ⚠️ Aviso legal

Este projeto **não é afiliado, endossado nem patrocinado pela
NCSoft**. "Lineage II" é marca registrada da NCSoft Corporation.

Este software se destina **exclusivamente a uso em servidores
privados** que você opere ou esteja autorizado a participar. Usar em
servidores oficiais da NCSoft pode violar os Termos de Serviço
deles. Os autores não se responsabilizam por contas, personagens ou
ações de terceiros usando este software.

O repositório **não contém assets do jogo protegidos por
copyright** — nenhuma textura, áudio ou dado de jogo do L2 é
redistribuído. A DLL chama funções do cliente em runtime; os
arquivos decriptados nunca saem da máquina que rodou o cliente.

Use por sua conta e risco.

---

## Features

- **Dump em lote** — o array `kDumpList` em `src/dllmain.cpp` lista
  os arquivos pelos nomes originais (`Interface.xdat`,
  `Interface.ui`, `core.ui`, `engine.ui`).
- **Decripta pelo próprio jogo** — carrega cada arquivo com
  `appLoadFileToArray` do Core.dll e salva com
  `appSaveArrayToFile` via o `GFileManager` do jogo, então a saída
  já sai decriptada.
- **Saída autocontida** — arquivos dumpados e o log (`overlay.log`)
  vão pra uma pasta `decrypted` criada ao lado da `l2ui.dll`.
- **Themida-safe** — injeta via IAT da Engine.dll, nunca modifica o
  L2.exe (que tem proteção Themida e quebra ao tentar tamper).

## Quick start

```bat
:: Buildar (precisa VS 2022/2026 com workload C++ + CMake 3.20+)
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Release

:: Saída: build/Release/l2ui.dll  (~85 KB)

:: Instalar (uma vez por instalação L2)
copy "L2\System_en\Engine.dll" "L2\System_en\Engine.dll.original"
copy build\Release\l2ui.dll  "L2\System_en\"
python add_import.py "L2\System_en\Engine.dll"

:: Inicie o L2 normalmente — os arquivos aparecem em L2\System_en\decrypted\
```

Guias passo-a-passo completos:

- 🇧🇷 [**docs/BUILDING.pt-BR.md**](docs/BUILDING.pt-BR.md) — compilar do zero (VS ou VS Code)
- 🇧🇷 [**docs/USAGE.pt-BR.md**](docs/USAGE.pt-BR.md) — instalar, configurar, usar
- 🇺🇸 [**docs/BUILDING.md**](docs/BUILDING.md) — build guide (EN)
- 🇺🇸 [**docs/USAGE.md**](docs/USAGE.md) — usage guide (EN)

## Como funciona

```
┌─────────────┐  import IAT    ┌───────────────┐                    ┌──────────┐
│  L2.exe     │───────────────►│  Engine.dll   │───────────────────►│ l2ui.dll │
│ (Themida)   │  (intocado)    │  (intocado)   │   L2UI_Init        │  dump    │
└─────────────┘                └───────────────┘                    └────┬─────┘
                                                                         │
                                                     appLoadFileToArray │ Core.dll
                                                     appSaveArrayToFile ▼
                                                                    ┌──────────┐
                                                                    │ Core.dll │
                                                                    └──────────┘
```

- Um único export `L2UI_Init` é adicionado à tabela de imports da
  Engine.dll via CFF Explorer ou o script Python incluído. O Windows
  carrega a `l2ui.dll` quando resolve os imports da Engine.dll.
- Assim que a `WinDrv.dll` está presente, a DLL chama a API de
  arquivos do Core.dll para cada arquivo da `kDumpList` e salva o
  conteúdo decriptado em `<jogo>\System_en\decrypted\<nome>`.
- O `overlay.log`, na mesma pasta `decrypted`, registra cada par
  `origem -> destino` e uma linha final `concluido (n/m)`.

## Estrutura do projeto

```
.
├── LICENSE                  MIT
├── README.md                este arquivo em inglês
├── README.pt-BR.md          versão em português
├── docs/
│   ├── BUILDING.md          guia de build (EN)
│   ├── BUILDING.pt-BR.md    guia de compilação
│   ├── USAGE.md             guia de uso (EN)
│   └── USAGE.pt-BR.md       guia de uso
├── CMakeLists.txt           build top-level
├── add_import.py            alternativa ao CFF Explorer (baseado em pefile)
├── .vscode/                 tasks/settings do VS Code
├── scripts/                 helpers de build do VS Code (env MSVC x86)
└── src/
    └── dllmain.cpp          DllMain, log, lista de dump, export L2UI_Init
```

## Stack tecnológica

| Componente | Biblioteca | Licença |
|------------|------------|---------|
| Compilador | MSVC (VS 2022/2026, toolchain x86) | — |
| Build | CMake ≥ 3.20 + Ninja | BSD-3-Clause / Apache-2.0 |
| Decriptação | nenhuma — feita pelo jogo (Core.dll) | — |

## Licença

MIT — veja [LICENSE](LICENSE).

