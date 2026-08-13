---
name: l2-interface-decrypt
description: >-
  **PROJECT SKILL** — Desenvolvimento da l2ui.dll (projeto L2InterfaceDecrypt), DLL dump-only
  para clientes Lineage II. USE PARA: qualquer mudança em src/dllmain.cpp, CMakeLists.txt,
  scripts/, .vscode/, docs/ ou .github/workflows/; adicionar/remover arquivos da lista de dump
  (kDumpList); ajustar caminhos/APIs do Core.dll (appLoadFileToArray/appSaveArrayToFile);
  compilar via VS Code ou CI; criar releases com tag v*. NÃO USE para: projetos UnrealScript .uc
  (use a skill for-interface-l2-fafurion).
---

# L2InterfaceDecrypt — l2ui.dll (dump-only)

## 🤖 Visão geral

DLL Win32 (x86) que side-loada no cliente L2 via IAT da **Engine.dll** e, quando a
`WinDrv.dll` carrega, **dumpa (decripta) uma lista de arquivos** usando a própria API de
arquivos do jogo (Core.dll). Sem hooks, sem overlay, sem ImGui.

- **Projeto:** `D:\Desenvolvimento\l2fan Classic\Custom_interface\Sources dll\Dll\L2InterfaceDecrypt`
- **Repositório:** `https://github.com/weltongbi/L2InterfaceDecrypt` (branch `main`)
- **Saída do build:** `build\l2ui.dll` + `build\l2ui.pdb`
- **Pasta do jogo (instalação manual):** `C:\Users\Welton\Desktop\Games\system_en\`
- **Saída do dump em runtime:** `<pasta da l2ui.dll>\decrypted\` (arquivos + `overlay.log`)

## ⚙️ Fluxo do dump

```
L2.exe → Engine.dll (IAT: l2ui.dll!L2UI_Init) → l2ui.dll
                                                  │ DLL_THREAD_ATTACH
                                                  ▼
                              StartCheck: WinDrv.dll carregada?
                                                  ▼
                              DumpAllFiles() → l2ui.ini (se existir)
                                              ou kDumpList[]
                                                  │
                                 appLoadFileToArray (Core.dll)
                                 appSaveArrayToFile (Core.dll)
                                                  ▼
                              <pasta da DLL>\decrypted\<nome>
```

- **Gatilho:** `DLL_THREAD_ATTACH` — **nunca** chamar `DisableThreadLibraryCalls()`, senão o dump nunca dispara.
- **APIs do Core.dll** (mangled names exatos — se mudarem no cliente, o dump falha):
  - `?appLoadFileToArray@@YAHAAV?$TArray@E@@PB_WPAVFFileManager@@@Z`
  - `?appSaveArrayToFile@@YAHABV?$TArray@E@@PB_WPAVFFileManager@@@Z`
  - `?GFileManager@@3PAVFFileManager@@A` (global: ponteiro para o FFileManager)
- **TArray** buffer de `0x14` bytes: `[0]` = ponteiro de dados, `[4]` = ArrayNum (tamanho carregado), `[8]` = ArrayMax.
- **Caminho de carga:** `..\system\<nome>` (como o jogo enxerga). **Caminho de gravação:** absoluto, calculado por `GetDecryptedDir()`/`BuildDstPath()`.
- **Config opcional `l2ui.ini`:** se existir ao lado da DLL, **substitui** o `kDumpList`. Seção `[files]`, chaves `file1`, `file2`, … lidas em sequência até a primeira ausente (máx. 512). INI presente com `[files]` vazia = nada é dumpado (log avisa). Funções: `GetIniPath()`/`LoadDumpListFromIni()` em `src/dllmain.cpp`.

## 📁 Estrutura do projeto

```
├── src/dllmain.cpp            # TODO o código: log, kDumpList, dump, DllMain, L2UI_Init
├── CMakeLists.txt             # alvo l2ui.dll, trava x86, CRT estático, /guard:cf-
├── add_import.py              # patch IAT na Engine.dll (precisa: pip install pefile)
├── scripts/
│   ├── vscode-cmake.ps1       # vswhere → VsDevCmd -arch=x86 → cmake (genérico de VS)
│   └── build.ps1              # configura sozinho (Release) na 1ª vez e compila
├── .vscode/
│   ├── tasks.json             # Configurar (Release/Debug), Compilar, Limpar, IAT patch
│   ├── settings.json          # compile_commands + config CMake Tools
│   ├── task-buttons.json      # botões da status bar (condor304.task-buttons)
│   └── extensions.json        # ms-vscode.cpptools, cmake-tools, python, condor304.task-buttons
├── .github/workflows/
│   └── build-release.yml      # tag v* → build x86 (vswhere+Ninja) → Release no GitHub
├── docs/                      # BUILDING.*.md e USAGE.*.md — SEMPRE em EN + pt-BR
└── LICENSE                    # MIT — Welton (weltongbi)
```

## ⚠️ Regras de ouro (inegociáveis)

1. **Sempre Win32 (x86).** O cliente L2 é 32-bit. O `CMakeLists.txt` tem trava
   (`CMAKE_SIZEOF_VOID_P == 4`, senão `FATAL_ERROR`). As tasks do VS Code e o CI já
   forçam x86 via `VsDevCmd -arch=x86` — não remova isso.
2. **Nunca patcheie o L2.exe.** Ele tem Themida (anti-tamper). O IAT vai na **Engine.dll**.
3. **A lista `kDumpList` só tem o NOME ORIGINAL do arquivo** (sem caminho). Carga vira
   `..\system\<nome>` automaticamente. Use a **extensão real do cliente** (`.u`, `.xdat` —
   os `.ui` da listagem antiga eram erro: os pacotes são `.u`).
4. **Não redefina `WIN32_LEAN_AND_MEAN` no código.** Ele já vem do `CMakeLists.txt`
   (duplicar gera warning C4005).
5. **Sem cópia automática para o jogo.** O build só gera `build\l2ui.dll`; instalar é
   manual (`copy build\l2ui.dll "C:\Users\Welton\Desktop\Games\system_en\"`).
6. **Log sempre visível no Release:** `DumpLog()` grava sempre; `Logf()` é só Debug.
7. **Docs são bilíngues.** Toda mudança relevante atualiza `docs/BUILDING.md` **e**
   `docs/BUILDING.pt-BR.md` (idem para USAGE e README).
8. **CI é agnóstico de versão do VS.** No workflow, nunca use o gerador
   `"Visual Studio 17 2022"` hardcoded — use `vswhere` + `VsDevCmd` + `Ninja`.

## 🔨 Como compilar (VS Code)

- **Ctrl+Shift+B** → `CMake: Compilar` (na 1ª vez configura Release sozinho).
- Tarefas extras: `CMake: Configurar (Release/Debug)`, `CMake: Limpar`,
  `IAT: Adicionar import (add_import.py)` (pede o caminho da Engine.dll).
- Linha de comando equivalente:
  `powershell -File scripts\vscode-cmake.ps1 -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
  e depois `cmake --build build`.
- Resultado: `build\l2ui.dll` (~85 KB, CRT estático — sem dependência de vcruntime).

## ✅ Tarefas comuns

### Adicionar/remover arquivo do dump

1. Edite `kDumpList[]` em `src/dllmain.cpp` (só o nome, extensão real).
2. Confira que o arquivo existe no cliente (ex.: `Get-ChildItem system_en -Filter *.u`).
3. Recompile (Ctrl+Shift+B), copie manualmente pra pasta do jogo e teste.
4. O log `decrypted\overlay.log` mostra `carregado=X bytes, gravado=Y bytes` por arquivo.
   **`carregado=0` = arquivo não encontrado** (nome/extensão errada no cliente).

### Configurar a lista via l2ui.ini (sem recompilar)

- Template pronto na raiz do repo: **`l2ui.ini.example`** — copiar para a pasta do jogo e renomear para `l2ui.ini`.
- Crie `l2ui.ini` **ao lado da `l2ui.dll`** (mesma pasta do jogo):
  ```ini
  [files]
  file1=Interface.xdat
  file2=Core.u
  ```
- Se o INI existir, ele **ignora o `kDumpList`** — remover o INI volta à lista interna.
- O log mostra a fonte: `usando l2ui.ini (N arquivos)` ou `usando kDumpList interno`.

### Diagnosticar dump com 0 bytes

- Ver `overlay.log` (na pasta `decrypted`). `carregado=0` → nome errado.
- Se tudo carregou mas nada gravou, o file manager pode não aceitar caminho absoluto:
  troque `BuildDstPath` para gravar relativo (`decrypted\<nome>`).

### Mudar a pasta de saída

- Edite `GetDecryptedDir()` em `src/dllmain.cpp` (log e dumps usam a mesma função).

### Atualizar APIs do Core.dll

- Os nomes mangled estão no `GetProcAddress` dentro de `DumpOne()`. Para encontrar os
  novos, use dumpbin/IDA no `Core.dll` do cliente.

## 🚀 CI/CD e releases

- Workflow: `.github/workflows/build-release.yml`. Gatilhos: tag `v*` e `workflow_dispatch`.
- Tag → build x86 (vswhere + VsDevCmd + Ninja) → empacota **`l2ui.zip`**
  (`l2ui.dll` + `l2ui.ini.example`) → **Release no GitHub** com `l2ui.zip` + `l2ui.pdb`.
- Publicar nova versão:
  ```
  git add -A
  git commit -m "tipo: descrição"   # conventional commits: fix:, chore:, feat:, docs:
  git tag v0.2.0
  git push origin main
  git push origin v0.2.0
  ```
- Convenções: tags `v0.x.y` em minúsculas (gatilho `v*`); commits em inglês no estilo
  conventional commits; nunca commitar `build/` (já no `.gitignore`).

## 🧠 Armadilhas conhecidas

| Sintoma                                              | Causa / solução                                                       |
| ---------------------------------------------------- | --------------------------------------------------------------------- |
| `warning C4005 WIN32_LEAN_AND_MEAN`                  | macro duplicada no código — remover o `#define`                       |
| `unresolved external symbol __imp__`                 | compilou x64 — use toolchain x86                                      |
| `could not find any instance of Visual Studio` no CI | gerador hardcoded — use vswhere+Ninja                                 |
| Dump nunca roda                                      | `DisableThreadLibraryCalls()` chamado, ou IAT patch não pegou         |
| `carregado=0 bytes`                                  | arquivo não existe no caminho `..\system\` (extensão `.u`, não `.ui`) |
| Nada é dumpado com `l2ui.ini` presente               | seção `[files]` vazia/malformada — corrigir o INI ou removê-lo        |
| `D3DERR_DEVICELOST` no Alt+Enter                     | alguém patcheou o L2.exe — restaurar backup e usar Engine.dll         |
