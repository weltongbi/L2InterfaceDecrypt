# Usando o L2InterfaceDecrypt

Guia de instalação + operação do `l2ui.dll`.

> 🇺🇸 English version: [USAGE.md](USAGE.md)

---

## Conceito

`l2ui.dll` é **side-loaded** no processo L2 via uma entrada de IAT
adicionada na **Engine.dll** (NÃO no L2.exe). Quando o Windows
resolve os imports da Engine.dll, nossa DLL é carregada e, assim
que a `WinDrv.dll` sobe, roda a lista de dump pela API de arquivos
do Core.dll. Sem hooks, sem UI.

---

## 1. Instalar (uma vez por instalação L2)

### 1.1. Colocar a DLL

Solte o `l2ui.dll` na pasta `System_en` do cliente L2 (ao lado de
`L2.exe`, `Engine.dll`, `NWindow.dll`, etc.):

```bat
copy build\Release\l2ui.dll  "C:\Games\L2\System_en\"
```

### 1.2. Backup da Engine.dll

```bat
copy "C:\Games\L2\System_en\Engine.dll" ^
     "C:\Games\L2\System_en\Engine.dll.original"
```

> ⚠️ Mantenha esse backup. O patch IAT é destrutivo; se algo der
> errado, restaure.

### 1.3. Patchear o IAT da Engine.dll

Duas formas — escolha uma.

#### Opção A: Script Python (recomendado)

```bat
pip install pefile
python add_import.py "C:\Games\L2\System_en\Engine.dll"
```

O script adiciona um import pra `l2ui.dll!L2UI_Init` na tabela de
imports da Engine.dll e salva no lugar. Verificação read-only:

```bat
python -c "import pefile; p=pefile.PE(r'C:\Games\L2\System_en\Engine.dll'); print([d.dll for d in p.DIRECTORY_ENTRY_IMPORT])"
```

Você deve ver `b'l2ui.dll'` na lista.

#### Opção B: CFF Explorer (manual)

1. Baixe o CFF Explorer (grátis, by NTCore):
   <https://ntcore.com/?page_id=388>
2. Abra `Engine.dll` no CFF Explorer.
3. Árvore esquerda → **Import Adder**.
4. Painel direito → **Add** (canto inferior esquerdo) → escolha `l2ui.dll`.
5. Com `l2ui.dll` selecionado, na caixa de função à direita digite
   `L2UI_Init` e clique **+ Add**.
6. Clique **Rebuild Import Table**.
7. **File → Save** (sobrescreve a Engine.dll — por isso o backup).

### 1.4. Lançar

Rode o `L2.exe` normalmente. Na inicialização, os arquivos
decriptados aparecem em `<jogo>\System_en\decrypted\`.

---

## 2. Por que Engine.dll e não L2.exe

O L2.exe tem proteção **Themida**. Modificar o IAT dele dispara o
anti-tamper do Themida e, em clientes Interlude, isso corrompe o
caminho de device-recreation do D3D9 — Alt+Enter (ou qualquer
mudança de resolução in-game) falha com `D3DERR_DEVICELOST` e
trava o cliente.

A `Engine.dll` **não** tem Themida. O L2.exe importa ela
naturalmente, então a ordem de load
L2.exe → Engine.dll → l2ui.dll acontece de graça.

Não tente modificar o L2.exe mesmo que "pareça funcionar" — o modo
de falha só dispara ao mudar resolução.

---

## 3. Saída & log

Tudo vai pra uma pasta `decrypted` criada ao lado da DLL:

```
C:\Games\L2\System_en\decrypted\
├── overlay.log
├── Interface.xdat
├── Interface.ui
├── core.ui
└── engine.ui
```

Log esperado (`overlay.log`):

```
[12:34:56.789 pid=12340 tid=5678] dump: ..\system\Interface.xdat -> C:\Games\L2\System_en\decrypted\Interface.xdat
[12:34:56.789 pid=12340 tid=5678] dump: ..\system\Interface.ui -> C:\Games\L2\System_en\decrypted\Interface.ui
[12:34:56.789 pid=12340 tid=5678] dump: concluido (4/4 arquivos)
```

As linhas `dump:` são sempre gravadas, mesmo em build Release. As
informações de attach (`=== l2ui.dll ATTACH ===` etc.) são só de
Debug.

---

## 4. Mudando a lista de arquivos

Edite o `kDumpList` em `src/dllmain.cpp`:

```cpp
static const wchar_t *kDumpList[] = {
    L"Interface.xdat",
    L"Interface.ui",
    L"core.ui",
    L"engine.ui",
};
```

Recompile depois de mudar. Cada arquivo é carregado de
`..\system\<nome>` (o caminho que o jogo enxerga) e salvo em
`<pasta da l2ui.dll>\decrypted\<nome>`.

---

## 5. Removendo

Restaure o backup da Engine.dll:

```bat
del "C:\Games\L2\System_en\Engine.dll"
copy "C:\Games\L2\System_en\Engine.dll.original" "C:\Games\L2\System_en\Engine.dll"
```

Você pode também deixar o `l2ui.dll` no lugar — sem a entrada IAT,
o Windows nunca carrega ele. Apague a pasta `decrypted` se quiser
os arquivos dumpados fora.

---

## 6. Troubleshooting

### Nenhum arquivo e log vazio/ausente

- A DLL nunca carregou — seu patch IAT não pegou. Re-rode o
  `add_import.py` e verifique com o one-liner Python em §1.3.
- Garanta que não está iniciando com a Engine.dll original (do
  backup) por engano.

### Arquivos não aparecem em `decrypted`

- O caminho de saída é um caminho absoluto do Windows. Se o file
  manager do seu cliente não lidar com caminhos absolutos, edite o
  `BuildDstPath` em `src/dllmain.cpp` pra gravar relativo
  `decrypted\<nome>`.

### Jogo crasha ao iniciar

- A DLL só toca no Core.dll depois que a `WinDrv.dll` carrega. Se
  ainda assim crashar, restaure o backup da Engine.dll e verifique
  o log — o crash quase certamente não é do dump em si.

### Alt+Enter trava / `D3DERR_DEVICELOST`

Isso quer dizer que o L2.exe (não a Engine.dll) foi patcheado.
Restaure o backup do L2.exe e patcheie a **Engine.dll**. Releia §2.

---

## 7. Privacidade & dados

- Sem contas, sem credenciais — nada é armazenado nem enviado pra
  lugar nenhum.
- Saída = os arquivos `.xdat`/`.ui` dumpados + um log com caminhos
  de arquivo, tudo dentro da pasta local `decrypted`.

---

## Próximos passos

- 📖 [BUILDING.pt-BR.md](BUILDING.pt-BR.md) — compilar do zero
- 📖 [README.pt-BR.md](../README.pt-BR.md) — visão geral do projeto

