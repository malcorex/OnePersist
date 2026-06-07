# Execution Methods

## First Execution Method

This method works when OneDrive is **not already running** on the target system, and the victim **manually launches OneDrive** (e.g., from the Start Menu or desktop shortcut).

> **Prerequisite:** OneDrive process is not currently active in the background.

---

## Second Execution Method

This method works when OneDrive is **already running** — either started automatically via **Windows Startup apps**, scheduled tasks, or background services — and the victim does **not** need to manually launch it.

> **Prerequisite:** OneDrive is already active (e.g., auto-starts with Windows or runs as a background service).

---

## Comparison

| Method | Condition | Trigger |
|--------|-----------|---------|
| **First** | OneDrive is **not running** | Victim manually launches OneDrive |
| **Second** | OneDrive is **already running** | Auto-start via Windows Startup / background service |

---