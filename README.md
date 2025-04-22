# Warhead

**Warhead** is a Windows post-exploitation payload launcher that leverages the Windows Atom Table for shellcode storage and reconstruction, enabling stealthy execution without writing payloads to disk.
![Warhead Banner](/img/warhead.webp)
## 🚀 Features

- 🔬 Atom Table-based shellcode injection
- 🧬 Fully dynamic reconstruction and decoding
- 🧱 Minimal memory footprint
- 🕵️ Designed for evasion and stealth
- ⚙️ Written in C for low-level control

## ⚠️ Disclaimer

This project is intended for **educational and research purposes** only. Use responsibly and only on systems you own or have explicit permission to test. The authors assume no liability for misuse or damage caused by this tool.

## 🛠️ Build Instructions

### Prerequisites

- Windows (any supported development environment)
- Visual Studio or `cl.exe` from Visual Studio Build Tools
- CMake (optional)

### Compilation (Example using cl.exe)

```sh
cl.exe /nologo /O2 /W3 /D_CRT_SECURE_NO_WARNINGS warhead.cpppp /link /OUT:WarHead.exe
```

Or use the included `build.bat` if provided.

## 🧪 Usage

### Basic Usage

```sh
WarHead.exe <hex_shellcode>
```

The shellcode must be a hex-encoded string (e.g., `fc4883e4f0e8c000000041514150...`).

### Example

```sh
WarHead.exe fc4883e4f0e8c000000041514150...
```

Warhead will:

1. Split the shellcode into chunks.
2. Store each chunk in the Windows Atom Table.
3. Retrieve and reconstruct the full shellcode.
4. Allocate memory and execute the payload.

## 🧬 Atom Table Technique

Warhead utilizes the Global Atom Table to:
- Store shellcode in chunks as strings.
- Reconstruct shellcode at runtime via `GlobalGetAtomName`.
- Avoid using traditional storage (e.g., disk, registry, or environment variables).

## 🔍 Analysis

For a deep-dive analysis of the Atom Table technique and payload evasion, see the [blog post](https://example.com) or [BlackHat Arsenal 2025 presentation](https://example.com/warhead-arsenal).

## 📁 Project Structure

```
.
├── warhead.cpp           # Main launcher source code
├── README.md           # This file
├── LICENSE             # Licensing information
└── build.bat           # Optional build script for Windows
```

## 🧠 Credits

Created by Malienist (https://github.com/malienist)  
Inspired by Atom Bombing and novel in-memory execution techniques.

## 📜 License

MIT License. See [LICENSE](./LICENSE) for details.
