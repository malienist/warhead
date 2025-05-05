
# 🔧 Warhead Binaries Overview

This page provides a description of the key binaries included in the Warhead toolkit. These tools enable different methods of Atom Table-based payload injection, execution, and privilege escalation.

All binaries will be made available once **Warhead** is officially released.

---

## 📦 Included Binaries

### `WarHead-wrapper-elevated.exe`
A wrapper binary designed to run with elevated privileges. It retrieves a payload from the Atom Table (using a passed Atom ID) and executes it. Often used in conjunction with a UAC bypass trigger like `fodhelper.exe`.

---

### `warhead-dropper.exe`
A dropper that writes a command or shellcode to the Atom Table and can optionally trigger execution. This binary handles Atom creation (`GlobalAddAtom`) and can log the generated Atom ID for use by loaders or wrappers.

---

### `WarHead_AtomCommandMenu.exe`
A GUI or console-based helper tool that lets you:
- Write payloads to Atom Table
- View current Atom content
- Launch execution stagers
Useful for testing and prototyping Atom-based payloads interactively.

---

### `warhead-run-elevated.exe`
A companion binary used to launch a specified executable in elevated context, typically using a UAC bypass. It passes an Atom ID to a trusted elevated process to execute the Atom-stored payload.

---

### `warhead-shellcode-no-injection.exe`
A diagnostic binary that simulates shellcode staging and payload extraction from the Atom Table, but does not perform injection. Useful for validating Atom content and shellcode retrieval flow.

---

### `warhead-shellcode-injection.exe`
A full injector binary that pulls shellcode from the Atom Table and injects it into a specified process (either via `WriteProcessMemory` or `QueueUserAPC`). Requires appropriate privileges to target processes.

--- 

### `warhead.exe`
The primary all-in-one binary that performs every stage of the Atom Table attack chain. It supports:
- Atom writing (`GlobalAddAtom`)
- Process discovery
- Shellcode injection (`WriteProcessMemory`, `QueueUserAPC`)
- UAC bypass via `fodhelper.exe`
- Execution triggering

Ideal for end-to-end payload deployment without relying on separate components.

---

## 📌 Notes

- These tools are intended for research and red team simulation in controlled environments.
- Ensure proper permissions and testing scopes before deployment.
- All binaries will be available in the [Releases](https://github.com/malienist/warhead/releases) section when the project is publicly launched.

---

## 📜 License & Usage

> For educational and research purposes only.  
> Do not use these tools on any system without explicit permission.


