## License

**OnePersist** is licensed under the **GNU General Public License v3.0 (GPL-3.0)**.

# Guide – DLL Hijacking in OneDrive (Persistence)

> **For educational and security research purposes only. Malicious use is illegal.**

---

**Tested versions:**  
On Windows 10/11 with OneDrive versions `26.007.0112.0002`, `25.179.0914.0003` and `26.084.0504.0007`  
- Windows 10 Pro 22H2  
- Windows 11 Pro 24H2  
- Windows 11 Pro 25H2  

---

### Steps to Reproduce the Vulnerability

1. Compile a malicious DLL named `FileCoAuthLib64.dll`  
2. Terminate the following processes (if running):  
   - `UserOOBEBroker.exe`  
   - `OneDrive.exe`  
   - `FileCoAuth.exe`  
3. Replace the malicious DLL in the **OneDrive installation directory**  
4. Wait for `FileCoAuth.exe` to auto-execute or manually launch OneDrive  
5. Observe successful DLL loading (Proof: `MessageBox` appears)  
6. Note that `UserOOBEBroker.exe` correctly **rejects** the unsigned DLL  
7. **Persistence is achieved without privilege escalation**  

---

### Description

A **DLL hijacking vulnerability** exists in the OneDrive component bundled with Windows, where the digital signature of `FileCoAuthLib64.dll` is **not verified** before loading.

While other Windows components like `UserOOBEBroker.exe` properly validate DLL signatures, `OneDrive.exe` and `FileCoAuth.exe` fail to perform this security check.

This allows attackers to replace the legitimate DLL with a malicious version, achieving **persistent code execution at the user's privilege level**.

This vulnerability affects both installation methods:

- `AppData` – user-level, **no admin required**  
- `Program Files` – system-wide, **admin required for DLL replacement**

> **Note 1:** In both cases, the malicious DLL executes with **user privileges**.

> **Note 2 (Important):** The `FileCoAuth.exe` process that loads our malicious DLL is **not visible in Task Manager**! This makes the malware's presence much more stealthy and very difficult for an average user to detect.

> **Note 3:** Depending on the OneDrive version, the target DLL name may be either FileCoAuthLib.dll or FileCoAuthLib64.dll. Be sure to check the actual filename in the OneDrive installation directory before proceeding 

> **Note 4 (Important - Prerequisite):** The `FileCoAuth.exe` process **only executes when Microsoft Office is installed** on the target system. This is because `FileCoAuth.exe` is responsible for real-time co-authoring features in Office files (Word, Excel, PowerPoint).    
 
> **However**, this is **not a significant limitation** because Microsoft Office is installed on the vast majority of Windows systems. It is very rare to find a Windows user without Office installed. Therefore, this vulnerability remains highly effective in real-world scenarios.

---

### Impact of the Vulnerability

- Stealthy persistence  
- No need for privilege escalation if OneDrive is installed by default  
- Bypasses Windows security controls  
- Hidden execution process (not shown in Task Manager)

---

# Execution Methods

### First Execution Method

This method works when OneDrive is **not already running** on the target system, and the victim **manually launches OneDrive** (e.g., from the Start Menu or desktop shortcut).

> **Prerequisite:** OneDrive process is not currently active in the background.

---

### Second Execution Method

This method works when OneDrive is **already running** — either started automatically via **Windows Startup apps**, scheduled tasks, or background services — and the victim does **not** need to manually launch it.

> **Prerequisite:** OneDrive is already active (e.g., auto-starts with Windows or runs as a background service).

---

### Comparison

| Method | Condition | Trigger |
|--------|-----------|---------|
| **First** | OneDrive is **not running** | Victim manually launches OneDrive |
| **Second** | OneDrive is **already running** | Auto-start via Windows Startup / background service |

---

# Quick Start

All the steps described above have been **fully automated** and are available in the **`source`** folder.

### Compilation & Execution

1. Navigate to the **`build`** folder
2. Run the **`build.bat`** script to compile the payload
3. Execute the generated binary to activate the OnePersist technique on the target system

---

# Documentation

For more detailed information about this technique please refer to the **`Docs`** folder.
