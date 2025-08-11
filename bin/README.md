
# 🔧 Warhead Binaries Overview

This page provides a description of the key binaries included in the Warhead toolkit. These tools enable different methods of Atom Table-based payload injection, execution, and privilege escalation.

All binaries will be made available once **Warhead** is officially released.

---
# warhead-injection.exe

**Purpose:** Injects an Atom Table–retrieved command into a target process and executes it via a remote thread.  
**Default target:** `notepad.exe` (first running instance found if no PID is provided in future versions).

> ⚠️ **For authorized security testing and research only.** Use on systems you own or have explicit permission to test.

---

## How it works (high-level)

1. **Create Atom:** Writes a command string to the Windows Atom Table with `AddAtomA`  
   - Default command in the sample: `cmd /c start calc.exe`
2. **Retrieve Atom:** Reads the stored command back with `GetAtomNameA`
3. **Resolve API:** Locates `WinExec` from `kernel32.dll`
4. **Select Target:** Finds the first running `notepad.exe` and opens it with `OpenProcess`
5. **Stage Payload:** Allocates memory in the target (`VirtualAllocEx`) and writes the command (`WriteProcessMemory`)
6. **Execute:** Creates a remote thread at `WinExec`, passing the staged command as the parameter

---

## Build

- **Toolchain:** Visual Studio (cl.exe) / C++17 or later  
- **Example (Developer Command Prompt):**
  ```powershell
  cl.exe /nologo /O2 /W3 /D_CRT_SECURE_NO_WARNINGS warhead-injection.cpp /link /OUT:warhead-injection.exe
  ```

> Note: The sample includes `<windows.h>`, `<tlhelp32.h>`, `<iostream>`, `<vector>`, `<string>`.

---

## Usage

```powershell
warhead-injection.exe
```

- Current sample auto-selects **Notepad** as the target.  
- The command executed is the one written to the Atom Table within the binary (e.g., `cmd /c start calc.exe` in the sample).  
- Future variants may support `PID` and custom command/Atom parameters via CLI.

**Sample Output**
```
[+] Atom created with ID: 49367
[+] Retrieved command from Atom: cmd /c start calc.exe
[+] Found Notepad PID: 1234
[+] Atom command executed from Notepad.
```

---

## Expected behavior & caveats

- **Parent/child process tree:** The spawned process (e.g., `calc.exe`) may show **`cmd.exe`** or the **target process** as parent depending on how the command is executed and environment.  
- **Access rights:** `OpenProcess(PROCESS_ALL_ACCESS, …)` requires sufficient privileges. Standard users may fail opening elevated or protected processes.  
- **AV/EDR:** Remote thread creation and cross-process memory writes are commonly monitored. Use only in permitted lab/test conditions.

---

## Blue team notes (defense)

- Monitor for:
  - `AddAtomA` / `GetAtomNameA` use with suspicious strings
  - Cross-process `VirtualAllocEx`/`WriteProcessMemory`
  - `CreateRemoteThread` into non-child processes
  - `WinExec`/`CreateProcess*` from unusual call stacks in GUI processes (e.g., `notepad.exe`)
- Consider policy restrictions and alerting on unexpected process trees (e.g., Notepad launching shells).

---

## Disclaimer

This code is provided for **educational and research purposes** under proper authorization. The authors and contributors are **not responsible** for misuse.

# warhead-wrapper.exe

**Purpose:** Elevates and executes the **WarHead.exe** dropper using a UAC bypass via `fodhelper.exe`.  
**Technique:** Registry hijack of `HKCU\Software\Classes\ms-settings\Shell\Open\command` combined with the `DelegateExecute` trick to trigger elevated execution without prompting the user.

> ⚠️ **For authorized security testing and research only.** Unauthorized use is illegal and unethical.

---

## How it works (high-level)

1. **Resolve Dropper Path:**  
   - The wrapper assumes `WarHead.exe` is in the same directory.
   - Uses `GetFullPathNameA` to resolve the absolute path.

2. **Set Registry Keys for UAC Bypass:**  
   - Adds the full path to `WarHead.exe` as the default value for:
     ```
     HKCU\Software\Classes\ms-settings\Shell\Open\command
     ```
   - Creates an empty `DelegateExecute` value under the same key to suppress UAC prompts.

3. **Trigger Auto-Elevation:**  
   - Disables Wow64 filesystem redirection (for 32-bit process compatibility on 64-bit Windows).
   - Launches `fodhelper.exe` with `ShellExecuteA`, which will read the hijacked registry keys and run `WarHead.exe` elevated.

4. **Cleanup:**  
   - Restores Wow64 redirection after launch.
   - Displays execution success or failure status.

---

## Build

- **Toolchain:** Visual Studio (cl.exe) / C++17 or later  
- **Example (Developer Command Prompt):**
  ```powershell
  cl.exe /nologo /O2 /W3 /D_CRT_SECURE_NO_WARNINGS warhead-wrapper.cpp /link /OUT:warhead-wrapper.exe


