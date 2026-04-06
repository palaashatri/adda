xp-desktop/
├── CMakeLists.txt
├── build.sh
├── LICENSE
├── xp-desktop.desktop
└── src/
    ├── server.h          # Core compositor state
    ├── main.c            # Compositor entry & setup
    ├── render.c          # XP theme drawing (Cairo)
    ├── shell.c           # Desktop, taskbar, start menu
    ├── input.c           # Mouse/keyboard handling
    └── notepad.c         # Native Wayland XP Notepad