# hypr-workspaces-tools

A small C++ utility for **Hyprland** that moves Firefox windows to specific workspaces
based on a workspace number prefix in the window title.

Example window titles:
- `[1] ...`
- `[2] ...`
- `[3] ...`

The tool reads Hyprland’s client list (`hyprctl clients -j`) and, for each Firefox window
whose title begins with `[N]`, it moves that window to workspace `N`.

---

## Why this exists

Hyprland does not automatically restore window → workspace placement across reboots.
Firefox can restore windows/tabs, but not which workspace each window belongs to.

This tool makes placement deterministic by encoding the target workspace into the title.

---

## Requirements

- Hyprland + `hyprctl`
- `jsoncpp` (C++ JSON library)
- Firefox
- A Firefox extension that can modify the real OS window title (e.g. “Window Titler”)

---

## Build

```bash
g++ -O2 -std=c++20 \
  -o organize_workspaces \
  organize_workspaces.cpp \
  -ljsoncpp
