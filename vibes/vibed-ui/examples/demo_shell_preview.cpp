// MIT License
// Copyright (c) 2026 Palaash

#include "shell/ShellApp.h"

int main() {
    shell::ShellApp app;
    if (!app.initialize()) {
        return 1;
    }

    app.run();
    app.shutdown();
    return 0;
}