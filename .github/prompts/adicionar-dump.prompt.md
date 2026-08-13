---
name: adicionar-dump
description: Adiciona ou remove arquivos da lista de dump (kDumpList) do L2InterfaceDecrypt e valida.
---

Adicione/remova entradas em `kDumpList` (`src/dllmain.cpp`) conforme o pedido.
Siga a skill l2-interface-decrypt:

1. Use apenas o NOME ORIGINAL do arquivo (sem caminho) e a extensão REAL do cliente
   (ex.: `.u`, `.xdat` — NÃO `.ui`).
2. Se possível, confirme que o arquivo existe na pasta do jogo
   (`C:\Users\Welton\Desktop\Games\system_en`).
3. Recompile (Ctrl+Shift+B) e lembre de copiar `build\l2ui.dll` manualmente para a
   pasta do jogo antes de testar.
4. Se a lista mudar de forma relevante, atualize também `docs/USAGE.md` e
   `docs/USAGE.pt-BR.md` (bilíngue).
