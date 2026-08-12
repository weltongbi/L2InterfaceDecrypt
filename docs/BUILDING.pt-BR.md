# Compilando o L2InterfaceDecrypt

Build passo-a-passo do `l2ui.dll`.

> 🇺🇸 English version: [BUILDING.md](BUILDING.md)

---

## Pré-requisitos

| Ferramenta | Necessário | Testado |
|------------|------------|---------|
| Visual Studio 2022 | Workload C++ Desktop | 17.10 |
| CMake | ≥ 3.20 | 3.29 |
| Git | só pra clonar o repositório (o build em si não baixa nada) | 2.40+ |
| Python (opcional) | 3.10+ com `pefile` — pro `add_import.py` | 3.11 |

Onde baixar:

- Visual Studio 2022 Community → <https://visualstudio.microsoft.com/downloads/> (marque **Desenvolvimento para desktop com C++**)
- CMake → <https://cmake.org/download/>
- Python → <https://www.python.org/downloads/>

O build não usa mais FetchContent (ImGui/MinHook foram removidos) —
nada é baixado na configuração.

---

## Build

O cliente L2 é **32 bits**, então o `l2ui.dll` precisa ser buildado
pra **Win32**.

```bat
git clone https://github.com/weltongbi/L2InterfaceDecrypt.git
cd L2InterfaceDecrypt
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Release
```

Saída: `build\Release\l2ui.dll` (~85 KB, só o dump) mais um `l2ui.pdb` pra
debug.

### Opcional: Build Debug

Um build Release ainda ship PDB completo (`/Zi /DEBUG`) e o runtime
CRT é estático (`/MT`), então a DLL é self-contained — sem
dependência de `vcruntime140.dll`.

Se você quiser binário Debug de verdade mesmo assim:

```bat
cmake --build build --config Debug
```

Saída: `build\Debug\l2ui.dll`.

---

## Build no VS Code (recomendado)

O projeto já vem pronto pra VS Code (`.vscode/` + `scripts/`).
Não precisa mais abrir o Visual Studio.

### O que instalar

1. **Toolchain MSVC (obrigatório)** — o VS Code não traz compilador:
   - Se o Visual Studio 2022/2026 já está instalado com o workload
     **Desenvolvimento para desktop com C++**, não precisa fazer nada.
   - Senão: instale o **Visual Studio Build Tools** (gratuito) e marque
     o workload *Desktop development with C++*.
2. **CMake ≥ 3.20** — <https://cmake.org/download/> e marque
   *Add CMake to the system PATH* (ou use o CMake que vem com o VS).
3. **Git** — só é necessário pra clonar o repositório (o build não
   baixa dependências).
4. **Extensões do VS Code** — ao abrir a pasta, aceite as recomendações
   do workspace (ou instale manualmente):
   - `ms-vscode.cpptools` — C/C++ (IntelliSense)
   - `ms-vscode.cmake-tools` — CMake Tools (opcional, veja abaixo)
   - `ms-python.python` — Python (opcional, pro `add_import.py`)

### Compilar

- **Ctrl+Shift+B** → tarefa **CMake: Compilar**. Na 1ª vez ela
  configura sozinha (Release, x86 via developer shell do VS) e depois
  builda. Saída: `build\l2ui.dll` + `l2ui.pdb`.
- Tarefas extras (**Terminal → Run Task**):
  - **CMake: Configurar (Release/Debug)** — força reconfiguração.
  - **CMake: Limpar**
  - **IAT: Adicionar import (add_import.py)** — pede o caminho da
    `Engine.dll` e roda o patch do IAT.

> ℹ️ O toolchain x86 é ativado automaticamente pelo
> `scripts/vscode-cmake.ps1` (developer shell do VS com `-arch=x86`),
> então a DLL nunca sai x64 por engano.

### Instalando na pasta do jogo (manual)

O build não toca mais na pasta do jogo. Depois de compilar, copie a
DLL pro cliente você mesmo:

```bat
copy build\l2ui.dll "LineageII\system\"
```

### CMake Tools (opcional)

Se preferir a extensão **CMake Tools** em vez das tasks: instale
`ms-vscode.cmake-tools`, **selecione um kit x86** na barra de status
(ex.: `Visual Studio Community ... - x86`) e configure/build pela
extensão. Ela usa a pasta separada `build-vscode/` pra nunca conflitar
com as tasks.

---

## Erros comuns

| Sintoma | Solução |
|---------|---------|
| `unresolved external symbol __imp__` | Você configurou pra x64. Re-rode com `-A Win32`. |
| `Cannot find Visual Studio 17 2022` | Instale o workload **Desenvolvimento para desktop com C++**. |
| `vcruntime140.dll missing` em runtime | Você mudou o `CMAKE_MSVC_RUNTIME_LIBRARY` — mantenha o default `MultiThreaded` (CRT estático). |
| `cl.exe não encontrado` / erro no developer shell | Instale o workload **Desenvolvimento para desktop com C++** (VS ou Build Tools). |
| `D3DERR_DEVICELOST` depois do Alt+Enter | Você modificou o L2.exe em vez da Engine.dll. Reverta o L2.exe e injete pelo IAT da Engine.dll. Veja [USAGE.pt-BR.md](USAGE.pt-BR.md). |

---

## Próximos passos

- 📖 [USAGE.pt-BR.md](USAGE.pt-BR.md) — instalar e usar a DLL
- O lado do cliente L2 (patch IAT na Engine.dll) está no
  [USAGE.pt-BR.md](USAGE.pt-BR.md) — leia esse a seguir.
