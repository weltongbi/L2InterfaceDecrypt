# Using L2InterfaceDecrypt

Install + operate guide for `l2ui.dll`.

> 🇧🇷 Versão em português: [USAGE.pt-BR.md](USAGE.pt-BR.md)

---

## Concept

`l2ui.dll` is **side-loaded** into the L2 process via an IAT entry
added to **Engine.dll** (NOT L2.exe). When Windows resolves
Engine.dll's imports, our DLL is loaded and, once `WinDrv.dll` is
up, it runs the dump list through Core.dll's file-manager API. No
hooks, no UI.

---

## 1. Install (one-time per L2 install)

### 1.1. Place the DLL

Drop `l2ui.dll` into the L2 client's `System_en` folder (next to
`L2.exe`, `Engine.dll`, `NWindow.dll`, etc.):

```bat
copy build\Release\l2ui.dll  "C:\Games\L2\System_en\"
```

### 1.2. Back up Engine.dll

```bat
copy "C:\Games\L2\System_en\Engine.dll" ^
     "C:\Games\L2\System_en\Engine.dll.original"
```

> ⚠️ Keep this backup. The IAT patch is destructive; if anything
> goes wrong, restore it.

### 1.3. Patch Engine.dll's IAT

Two ways — pick one.

#### Option A: Python script (recommended)

```bat
pip install pefile
python add_import.py "C:\Games\L2\System_en\Engine.dll"
```

The script adds an import for `l2ui.dll!L2UI_Init` to Engine.dll's
import table and saves it in place. Read-only verification:

```bat
python -c "import pefile; p=pefile.PE(r'C:\Games\L2\System_en\Engine.dll'); print([d.dll for d in p.DIRECTORY_ENTRY_IMPORT])"
```

You should see `b'l2ui.dll'` in the list.

#### Option B: CFF Explorer (manual)

1. Download CFF Explorer (free, by NTCore):
   <https://ntcore.com/?page_id=388>
2. Open `Engine.dll` in CFF Explorer.
3. Left tree → **Import Adder**.
4. Right pane → **Add** (bottom-left) → choose `l2ui.dll`.
5. With `l2ui.dll` selected, in the function input box type
   `L2UI_Init` and click **+ Add**.
6. Click **Rebuild Import Table**.
7. **File → Save** (overwrites Engine.dll — that's why we backed it
   up).

### 1.4. Launch

Run `L2.exe` normally. On startup, the decrypted files appear in
`<game>\System_en\decrypted\`.

---

## 2. Why Engine.dll, not L2.exe

L2.exe is **Themida-protected**. Modifying its IAT triggers
Themida's anti-tamper, and on Interlude clients this corrupts the
D3D9 device-recreation path — Alt+Enter (or any in-game resolution
change) fails with `D3DERR_DEVICELOST`, freezing the client.

`Engine.dll` is **not** Themida-protected. L2.exe imports it
naturally, so the load order
L2.exe → Engine.dll → l2ui.dll happens for free.

Don't try to modify L2.exe even if it "seems to work" — the failure
mode only triggers on resolution change.

---

## 3. Output & log

Everything lands in a `decrypted` folder created next to the DLL:

```
C:\Games\L2\System_en\decrypted\
├── overlay.log
├── Interface.xdat
├── Interface.u
├── Core.u
├── Engine.u
└── NWindow.u
```

Expected log (`overlay.log`):

```
[12:34:56.789 pid=12340 tid=5678] dump: l2ui.ini nao encontrado — usando kDumpList interno
[12:34:56.789 pid=12340 tid=5678] dump: ..\system\Interface.xdat -> C:\Games\L2\System_en\decrypted\Interface.xdat
[12:34:56.789 pid=12340 tid=5678] dump: ..\system\Interface.u -> C:\Games\L2\System_en\decrypted\Interface.u
[12:34:56.789 pid=12340 tid=5678] dump: concluido (5/5 arquivos)
```

The `dump:` lines are always written, even in Release builds. The
attach info (`=== l2ui.dll ATTACH ===` etc.) is Debug-only.

---

## 4. Changing the file list

Two ways — the optional INI wins when present.

### 4.1. Optional config: `l2ui.ini` (no rebuild needed)

Create `l2ui.ini` **next to `l2ui.dll`** (same folder). When this
file exists, the DLL reads the list from it and **ignores the
built-in `kDumpList`**. A ready-to-use template is included in the
repo as [`l2ui.ini.example`](../l2ui.ini.example) — copy it next to
the DLL and rename it to `l2ui.ini`:

```bat
copy l2ui.ini.example "C:\Games\L2\System_en\l2ui.ini"
```

Example contents:
```ini
; l2ui.ini — optional dump list (section [files], keys file1..fileN)
[files]
file1=Interface.xdat
file2=Interface.u
file3=Core.u
file4=Engine.u
file5=NWindow.u
```

- Keys are read in order (`file1`, `file2`, …) until the first
  missing key. Max 512 entries.
- Full-line comments start with `;`.
- Only original file names (no paths) — each one is loaded from
  `..\system\<name>` as usual.
- If the INI exists but `[files]` is empty/malformed, **nothing is
  dumped** — `overlay.log` says so.
- Remove `l2ui.ini` to fall back to the built-in list.

### 4.2. Built-in list (fallback)

Edit `kDumpList` in `src/dllmain.cpp`:

```cpp
static const wchar_t *kDumpList[] = {
    L"Interface.xdat",
    L"Interface.u",
    L"Core.u",
    L"Engine.u",
    L"NWindow.u",
};
```

Rebuild after changing it. Each file is loaded from
`..\system\<name>` (the path the game sees) and saved to
`<l2ui.dll folder>\decrypted\<name>`.

---

## 5. Removing it

Restore the Engine.dll backup:

```bat
del "C:\Games\L2\System_en\Engine.dll"
copy "C:\Games\L2\System_en\Engine.dll.original" "C:\Games\L2\System_en\Engine.dll"
```

You can also leave `l2ui.dll` in place — without the IAT entry,
Windows never loads it. Delete the `decrypted` folder if you want
the dumped files gone.

---

## 6. Troubleshooting

### No files and empty/absent log

- The DLL never loaded — your IAT patch didn't take. Re-run
  `add_import.py` and verify with the python one-liner in §1.3.
- Make sure you're not launching with the original (backed-up)
  Engine.dll by mistake.

### Files don't appear in `decrypted`

- The save path is an absolute Windows path. If your client's file
  manager doesn't handle absolute paths, edit `BuildDstPath` in
  `src/dllmain.cpp` to write a relative `decrypted\<name>` instead.

### Game crashes on launch

- The DLL only touches Core.dll after `WinDrv.dll` loads. If it
  still crashes, restore the Engine.dll backup and check the log —
  the crash is almost certainly unrelated to the dump itself.

### Alt+Enter freezes / `D3DERR_DEVICELOST`

This means L2.exe (not Engine.dll) was patched. Restore the L2.exe
backup and patch **Engine.dll** instead. Read §2 again.

---

## 7. Privacy & data

- No accounts, no credentials — nothing is stored or sent anywhere.
- Output = the dumped `.xdat`/`.u` files plus a log with file
  paths, all inside the local `decrypted` folder.

---

## Where to go next

- 📖 [BUILDING.md](BUILDING.md) — build from source
- 📖 [README.md](../README.md) — project overview

