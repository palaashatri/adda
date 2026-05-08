# 📘 **MASTER_INSTRUCTIONS.md (STRICT MODE)**  
### **Place this file in the root of your repository. Do not rename it.**

---

# **MASTER INSTRUCTIONS FOR COPILOT**  
### **STRICT MODE — MAXIMUM CONSISTENCY — DO NOT DEVIATE**

Copilot must follow these rules for every file, every class, every method, and every commit.

---

## 1. PROJECT PURPOSE

This repository contains a **cross‑platform C++20 UI toolkit**, **legacy-compatible renderer**, **modern renderer stubs**, **platform abstraction layer**, **mock compositor**, and **Linux desktop shell**.

The goal is to produce a **compilable**, **runnable**, **minimal**, **consistent**, and **extendable** codebase that works on:

- Windows (Win98+)
- Linux (X11)
- macOS (Intel + PPC era)

The Linux version will also include:

- A mock compositor  
- A minimal desktop shell  
- A panel and launcher  

---

## 2. ABSOLUTE RULES FOR COPILOT

Copilot must:

1. **Follow the folder structure exactly.**
2. **Use the class names exactly.**
3. **Use the method signatures exactly.**
4. **Never invent new modules, folders, or abstractions.**
5. **Never rename anything.**
6. **Never introduce external dependencies.**
7. **Never use exceptions.**
8. **Never use RTTI.**
9. **Never use OS-native widgets.**
10. **Never use OpenGL > 1.3, Vulkan, Metal, or DirectX in the legacy renderer.**
11. **Always keep code compilable.**
12. **Always add TODO comments for unimplemented features.**

If unsure, Copilot must choose the **simplest** and **most minimal** implementation.

---

## 3. FOLDER STRUCTURE (MANDATORY)

Copilot must generate files only inside this structure:

```
/src
  /core
    Application.h/.cpp
    Event.h/.cpp
    Timer.h/.cpp
    Logger.h/.cpp

  /platform
    Platform.h/.cpp
    /windows
      WinPlatform.h/.cpp
    /linux
      LinuxPlatform.h/.cpp
    /mac
      MacPlatform.h/.cpp

  /render
    Renderer.h/.cpp
    /legacy
      SoftwareRenderer.h/.cpp
    /modern
      ModernRenderer.h/.cpp

  /ui
    View.h/.cpp
    ViewTree.h/.cpp
    Layout.h/.cpp
    FlexLayout.h/.cpp
    Theme.h/.cpp
    Color.h/.cpp
    Font.h/.cpp
    Button.h/.cpp
    Label.h/.cpp

  /compositor
    Compositor.h/.cpp
    Surface.h/.cpp
    WindowManager.h/.cpp

  /shell
    Panel.h/.cpp
    Launcher.h/.cpp
    ShellApp.h/.cpp

/examples
  demo_basic.cpp
  demo_controls.cpp

CMakeLists.txt
MASTER_INSTRUCTIONS.md
```

Copilot must **never** create files outside this structure.

---

## 4. CODING STYLE (MANDATORY)

- C++20  
- No exceptions  
- No RTTI  
- Use `std::unique_ptr`, `std::shared_ptr`, `std::vector`, `std::string`  
- Use namespaces:
  - `core`
  - `platform`
  - `render`
  - `ui`
  - `compositor`
  - `shell`
- All classes must have:
  - Header file  
  - Implementation file  
  - Constructor  
  - Destructor  
  - Minimal stub methods  

---

## 5. RENDERING RULES

### **Legacy Renderer (SoftwareRenderer)**  
Copilot must implement:

- Pixel buffer  
- drawRect  
- drawRoundedRect  
- drawText (stub)  
- clear()  
- present()  

Platform layer must blit the pixel buffer to screen using:

- **Windows:** GDI BitBlt  
- **Linux:** X11 XPutImage  
- **macOS:** CoreGraphics bitmap draw  

### **Modern Renderer (ModernRenderer)**  
Copilot must:

- Create class + methods  
- Leave all methods unimplemented  
- Add TODO comments  

---

## 6. UI TOOLKIT RULES

### **View**
Must include:

- Position  
- Size  
- Children  
- Background color  
- draw()  
- layout()  
- onEvent()  

### **Layout**
- Flexbox-like  
- Deterministic  
- No constraint solver  

### **Theme**
- Load from JSON  
- Provide:
  - Colors  
  - Radii  
  - Fonts  

### **Widgets**
Copilot must implement:

- Label  
- Button  
- Container views  

---

## 7. PLATFORM LAYER RULES

### Windows
- Win32 API  
- Create window  
- Message loop  
- Blit pixel buffer  

### Linux
- X11  
- Create window  
- Event loop  
- Blit pixel buffer  

### macOS
- Cocoa (Objective‑C++)  
- NSWindow  
- Draw pixel buffer  

---

## 8. COMPOSITOR RULES (LINUX ONLY)

Copilot must implement:

- Surface  
- WindowManager  
- Compositor  

This is a **mock compositor**, not a real Wayland compositor.

---

## 9. DESKTOP SHELL RULES

Copilot must implement:

- Panel (top or bottom bar)  
- Launcher (popup menu)  
- ShellApp (main entrypoint)  

---

## 10. DEMO APPLICATIONS

Copilot must create:

### `/examples/demo_basic.cpp`
- Create window  
- Show label  
- Show button  
- Button updates label  

### `/examples/demo_controls.cpp`
- Multiple buttons  
- Layout demo  
- Theme switching demo  

---

## 11. CMAKE RULES

Copilot must generate:

- Static libraries for each module  
- Executables for demo apps  
- Platform detection  
- Compiler flags for C++20  

---

## 12. WORKFLOW RULES FOR COPILOT

When editing any file, Copilot must:

1. Read this MASTER_INSTRUCTIONS.md  
2. Follow the folder structure  
3. Follow the class names  
4. Follow the method signatures  
5. Keep code minimal  
6. Keep code compilable  
7. Add TODO comments for missing features  
8. Never invent new abstractions  

---

# END OF MASTER INSTRUCTIONS

---

# INITIAL PROJECT SKELETON


This gives Copilot a **perfect starting point**.

---

# ✅ **INITIAL FOLDER STRUCTURE**

```
/src
  /core
    Application.h
    Application.cpp
    Event.h
    Event.cpp
    Timer.h
    Timer.cpp
    Logger.h
    Logger.cpp

  /platform
    Platform.h
    Platform.cpp
    /windows
      WinPlatform.h
      WinPlatform.cpp
    /linux
      LinuxPlatform.h
      LinuxPlatform.cpp
    /mac
      MacPlatform.h
      MacPlatform.cpp

  /render
    Renderer.h
    Renderer.cpp
    /legacy
      SoftwareRenderer.h
      SoftwareRenderer.cpp
    /modern
      ModernRenderer.h
      ModernRenderer.cpp

  /ui
    View.h
    View.cpp
    ViewTree.h
    ViewTree.cpp
    Layout.h
    Layout.cpp
    FlexLayout.h
    FlexLayout.cpp
    Theme.h
    Theme.cpp
    Color.h
    Color.cpp
    Font.h
    Font.cpp
    Button.h
    Button.cpp
    Label.h
    Label.cpp

  /compositor
    Compositor.h
    Compositor.cpp
    Surface.h
    Surface.cpp
    WindowManager.h
    WindowManager.cpp

  /shell
    Panel.h
    Panel.cpp
    Launcher.h
    Launcher.cpp
    ShellApp.h
    ShellApp.cpp

/examples
  demo_basic.cpp
  demo_controls.cpp

CMakeLists.txt
MASTER_INSTRUCTIONS.md
```

---

# ✅ **INITIAL CMakeLists.txt**

Paste this into your root `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(UIToolkit LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
add_subdirectory(examples)
```

Now create `/src/CMakeLists.txt`:

```cmake
add_library(core
    core/Application.cpp
    core/Event.cpp
    core/Timer.cpp
    core/Logger.cpp
)

add_library(platform
    platform/Platform.cpp
    platform/windows/WinPlatform.cpp
    platform/linux/LinuxPlatform.cpp
    platform/mac/MacPlatform.cpp
)

add_library(render
    render/Renderer.cpp
    render/legacy/SoftwareRenderer.cpp
    render/modern/ModernRenderer.cpp
)

add_library(ui
    ui/View.cpp
    ui/ViewTree.cpp
    ui/Layout.cpp
    ui/FlexLayout.cpp
    ui/Theme.cpp
    ui/Color.cpp
    ui/Font.cpp
    ui/Button.cpp
    ui/Label.cpp
)

add_library(compositor
    compositor/Compositor.cpp
    compositor/Surface.cpp
    compositor/WindowManager.cpp
)

add_library(shell
    shell/Panel.cpp
    shell/Launcher.cpp
    shell/ShellApp.cpp
)

target_link_libraries(ui core render)
target_link_libraries(shell ui compositor)
```

And `/examples/CMakeLists.txt`:

```cmake
add_executable(demo_basic demo_basic.cpp)
target_link_libraries(demo_basic core ui platform render)

add_executable(demo_controls demo_controls.cpp)
target_link_libraries(demo_controls core ui platform render)
```

---

# ✅ **INITIAL EMPTY HEADER FILES**

Below is the pattern Copilot must follow for **every header**.

```
Every header file must follow this template:

#pragma once
#include <memory>
#include <vector>
#include <string>

namespace <MODULE> {

class <ClassName> {
public:
    <ClassName>();
    ~<ClassName>();

    // TODO: Add methods
};

} // namespace <MODULE>
```

---

# ✅ **INITIAL Application CLASS**

`src/core/Application.h`

```cpp
#pragma once
#include <memory>
#include <string>

namespace core {

class Application {
public:
    Application();
    ~Application();

    bool initialize();
    void run();
    void shutdown();

private:
    bool running = false;
};

} // namespace core
```

`src/core/Application.cpp`

```cpp
#include "Application.h"
#include "Logger.h"

namespace core {

Application::Application() {}

Application::~Application() {}

bool Application::initialize() {
    Logger::info("Application initialized");
    running = true;
    return true;
}

void Application::run() {
    while (running) {
        // TODO: Event polling
        // TODO: Rendering
    }
}

void Application::shutdown() {
    running = false;
    Logger::info("Application shutdown");
}

} // namespace core
```

---

# ✅ **INITIAL SoftwareRenderer**

`src/render/legacy/SoftwareRenderer.h`

```cpp
#pragma once
#include <vector>
#include <cstdint>

namespace render {

class SoftwareRenderer {
public:
    SoftwareRenderer(int width, int height);
    ~SoftwareRenderer();

    void clear(uint32_t color);
    void drawRect(int x, int y, int w, int h, uint32_t color);
    void present();

    const uint8_t* buffer() const { return pixels.data(); }

private:
    int width;
    int height;
    std::vector<uint8_t> pixels;
};

} // namespace render
```

`src/render/legacy/SoftwareRenderer.cpp`

```cpp
#include "SoftwareRenderer.h"
#include <cstring>

namespace render {

SoftwareRenderer::SoftwareRenderer(int w, int h)
    : width(w), height(h), pixels(w * h * 4) {}

SoftwareRenderer::~SoftwareRenderer() {}

void SoftwareRenderer::clear(uint32_t color) {
    for (int i = 0; i < width * height; ++i) {
        std::memcpy(&pixels[i * 4], &color, 4);
    }
}

void SoftwareRenderer::drawRect(int x, int y, int w, int h, uint32_t color) {
    for (int yy = y; yy < y + h; ++yy) {
        if (yy < 0 || yy >= height) continue;
        for (int xx = x; xx < x + w; ++xx) {
            if (xx < 0 || xx >= width) continue;
            std::memcpy(&pixels[(yy * width + xx) * 4], &color, 4);
        }
    }
}

void SoftwareRenderer::present() {
    // TODO: Platform blit
}

} // namespace render
```

---

# ✅ **INITIAL PLATFORM BACKENDS**

### Windows

`src/platform/windows/WinPlatform.h`

```cpp
#pragma once
#include <windows.h>

namespace platform {

class WinPlatform {
public:
    WinPlatform();
    ~WinPlatform();

    bool createWindow(int w, int h, const char* title);
    void pumpEvents();
    void blit(const uint8_t* buffer, int w, int h);

private:
    HWND hwnd = nullptr;
    HDC hdc = nullptr;
};

} // namespace platform
```

`src/platform/windows/WinPlatform.cpp`

```cpp
#include "WinPlatform.h"

namespace platform {

WinPlatform::WinPlatform() {}

WinPlatform::~WinPlatform() {}

bool WinPlatform::createWindow(int w, int h, const char* title) {
    // TODO: Win32 window creation
    return true;
}

void WinPlatform::pumpEvents() {
    // TODO: Win32 message loop
}

void WinPlatform::blit(const uint8_t* buffer, int w, int h) {
    // TODO: BitBlt
}

} // namespace platform
```

---

### Linux (X11)

`src/platform/linux/LinuxPlatform.h`

```cpp
#pragma once
#include <X11/Xlib.h>

namespace platform {

class LinuxPlatform {
public:
    LinuxPlatform();
    ~LinuxPlatform();

    bool createWindow(int w, int h, const char* title);
    void pumpEvents();
    void blit(const uint8_t* buffer, int w, int h);

private:
    Display* display = nullptr;
    Window window = 0;
    GC gc = 0;
};

} // namespace platform
```

`src/platform/linux/LinuxPlatform.cpp`

```cpp
#include "LinuxPlatform.h"

namespace platform {

LinuxPlatform::LinuxPlatform() {}

LinuxPlatform::~LinuxPlatform() {}

bool LinuxPlatform::createWindow(int w, int h, const char* title) {
    // TODO: X11 window creation
    return true;
}

void LinuxPlatform::pumpEvents() {
    // TODO: X11 event loop
}

void LinuxPlatform::blit(const uint8_t* buffer, int w, int h) {
    // TODO: XPutImage
}

} // namespace platform
```

---

### macOS (Cocoa)

`src/platform/mac/MacPlatform.h`

```cpp
#pragma once

namespace platform {

class MacPlatform {
public:
    MacPlatform();
    ~MacPlatform();

    bool createWindow(int w, int h, const char* title);
    void pumpEvents();
    void blit(const uint8_t* buffer, int w, int h);

private:
    void* window = nullptr; // NSWindow*
};

} // namespace platform
```

`src/platform/mac/MacPlatform.cpp`

```cpp
#include "MacPlatform.h"

namespace platform {

MacPlatform::MacPlatform() {}

MacPlatform::~MacPlatform() {}

bool MacPlatform::createWindow(int w, int h, const char* title) {
    // TODO: Objective-C++ NSWindow creation
    return true;
}

void MacPlatform::pumpEvents() {
    // TODO: Cocoa event loop
}

void MacPlatform::blit(const uint8_t* buffer, int w, int h) {
    // TODO: CoreGraphics bitmap draw
}

} // namespace platform
```

---

# 🎉 **DONE**


---

# ✅ **SECTION: INITIAL UI WIDGETS**

## `src/ui/View.h`

```cpp
#pragma once
#include <vector>
#include <memory>
#include <string>
#include "../render/Renderer.h"

namespace ui {

class View {
public:
    View();
    virtual ~View();

    void setFrame(int x, int y, int w, int h);
    void getFrame(int& x, int& y, int& w, int& h) const;

    void addChild(std::shared_ptr<View> child);
    const std::vector<std::shared_ptr<View>>& children() const;

    virtual void draw(render::Renderer& renderer);
    virtual void layout();
    virtual void onEvent(int eventType, int x, int y);

protected:
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    std::vector<std::shared_ptr<View>> childViews;
};

} // namespace ui
```

## `src/ui/View.cpp`

```cpp
#include "View.h"

namespace ui {

View::View() {}
View::~View() {}

void View::setFrame(int px, int py, int w, int h) {
    x = px; y = py; width = w; height = h;
}

void View::getFrame(int& px, int& py, int& w, int& h) const {
    px = x; py = y; w = width; h = height;
}

void View::addChild(std::shared_ptr<View> child) {
    childViews.push_back(child);
}

const std::vector<std::shared_ptr<View>>& View::children() const {
    return childViews;
}

void View::draw(render::Renderer& renderer) {
    for (auto& c : childViews) c->draw(renderer);
}

void View::layout() {
    for (auto& c : childViews) c->layout();
}

void View::onEvent(int eventType, int ex, int ey) {
    for (auto& c : childViews) c->onEvent(eventType, ex, ey);
}

} // namespace ui
```

---

## `src/ui/Label.h`

```cpp
#pragma once
#include "View.h"

namespace ui {

class Label : public View {
public:
    Label(const std::string& text);
    ~Label();

    void setText(const std::string& t);
    const std::string& text() const;

    void draw(render::Renderer& renderer) override;

private:
    std::string labelText;
};

} // namespace ui
```

## `src/ui/Label.cpp`

```cpp
#include "Label.h"

namespace ui {

Label::Label(const std::string& t) : labelText(t) {}
Label::~Label() {}

void Label::setText(const std::string& t) {
    labelText = t;
}

const std::string& Label::text() const {
    return labelText;
}

void Label::draw(render::Renderer& renderer) {
    renderer.drawText(x, y, labelText);
    View::draw(renderer);
}

} // namespace ui
```

---

## `src/ui/Button.h`

```cpp
#pragma once
#include "View.h"
#include <functional>

namespace ui {

class Button : public View {
public:
    Button(const std::string& text);
    ~Button();

    void setOnClick(std::function<void()> handler);
    void draw(render::Renderer& renderer) override;
    void onEvent(int eventType, int ex, int ey) override;

private:
    std::string label;
    std::function<void()> onClick;
};

} // namespace ui
```

## `src/ui/Button.cpp`

```cpp
#include "Button.h"

namespace ui {

Button::Button(const std::string& t) : label(t) {}
Button::~Button() {}

void Button::setOnClick(std::function<void()> handler) {
    onClick = handler;
}

void Button::draw(render::Renderer& renderer) {
    renderer.drawRect(x, y, width, height, 0xFFCCCCCC);
    renderer.drawText(x + 4, y + 4, label);
    View::draw(renderer);
}

void Button::onEvent(int eventType, int ex, int ey) {
    if (eventType == 1) { // click
        if (ex >= x && ex <= x + width && ey >= y && ey <= y + height) {
            if (onClick) onClick();
        }
    }
    View::onEvent(eventType, ex, ey);
}

} // namespace ui
```

---

# ✅ **SECTION: INITIAL LAYOUT ENGINE**

## `src/ui/Layout.h`

```cpp
#pragma once
#include "View.h"

namespace ui {

class Layout {
public:
    Layout();
    virtual ~Layout();

    virtual void apply(View& parent) = 0;
};

} // namespace ui
```

## `src/ui/Layout.cpp`

```cpp
#include "Layout.h"

namespace ui {

Layout::Layout() {}
Layout::~Layout() {}

} // namespace ui
```

---

## `src/ui/FlexLayout.h`

```cpp
#pragma once
#include "Layout.h"

namespace ui {

class FlexLayout : public Layout {
public:
    FlexLayout();
    ~FlexLayout();

    void apply(View& parent) override;
};

} // namespace ui
```

## `src/ui/FlexLayout.cpp`

```cpp
#include "FlexLayout.h"

namespace ui {

FlexLayout::FlexLayout() {}
FlexLayout::~FlexLayout() {}

void FlexLayout::apply(View& parent) {
    int px, py, pw, ph;
    parent.getFrame(px, py, pw, ph);

    int yOffset = py;
    for (auto& c : parent.children()) {
        c->setFrame(px, yOffset, pw, 30);
        yOffset += 30;
    }
}

} // namespace ui
```

---

# ✅ **SECTION: INITIAL COMPOSITOR SKELETON**

## `src/compositor/Surface.h`

```cpp
#pragma once
#include <memory>

namespace compositor {

class Surface {
public:
    Surface(int w, int h);
    ~Surface();

    int width() const;
    int height() const;

private:
    int w;
    int h;
};

} // namespace compositor
```

## `src/compositor/Surface.cpp`

```cpp
#include "Surface.h"

namespace compositor {

Surface::Surface(int width, int height) : w(width), h(height) {}
Surface::~Surface() {}

int Surface::width() const { return w; }
int Surface::height() const { return h; }

} // namespace compositor
```

---

## `src/compositor/WindowManager.h`

```cpp
#pragma once
#include <vector>
#include <memory>
#include "Surface.h"

namespace compositor {

class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    void addSurface(std::shared_ptr<Surface> s);
    void renderAll();

private:
    std::vector<std::shared_ptr<Surface>> surfaces;
};

} // namespace compositor
```

## `src/compositor/WindowManager.cpp`

```cpp
#include "WindowManager.h"

namespace compositor {

WindowManager::WindowManager() {}
WindowManager::~WindowManager() {}

void WindowManager::addSurface(std::shared_ptr<Surface> s) {
    surfaces.push_back(s);
}

void WindowManager::renderAll() {
    // TODO: Composite surfaces
}

} // namespace compositor
```

---

## `src/compositor/Compositor.h`

```cpp
#pragma once
#include "WindowManager.h"

namespace compositor {

class Compositor {
public:
    Compositor();
    ~Compositor();

    void run();

private:
    WindowManager wm;
};

} // namespace compositor
```

## `src/compositor/Compositor.cpp`

```cpp
#include "Compositor.h"

namespace compositor {

Compositor::Compositor() {}
Compositor::~Compositor() {}

void Compositor::run() {
    while (true) {
        wm.renderAll();
        // TODO: sleep or vsync
    }
}

} // namespace compositor
```

---

# ✅ **SECTION: INITIAL SHELL (PANEL + LAUNCHER)**

## `src/shell/Panel.h`

```cpp
#pragma once
#include "../ui/View.h"

namespace shell {

class Panel : public ui::View {
public:
    Panel();
    ~Panel();

    void draw(render::Renderer& renderer) override;
};

} // namespace shell
```

## `src/shell/Panel.cpp`

```cpp
#include "Panel.h"

namespace shell {

Panel::Panel() {}
Panel::~Panel() {}

void Panel::draw(render::Renderer& renderer) {
    renderer.drawRect(x, y, width, height, 0xFF222222);
    ui::View::draw(renderer);
}

} // namespace shell
```

---

## `src/shell/Launcher.h`

```cpp
#pragma once
#include "../ui/View.h"

namespace shell {

class Launcher : public ui::View {
public:
    Launcher();
    ~Launcher();

    void draw(render::Renderer& renderer) override;
};

} // namespace shell
```

## `src/shell/Launcher.cpp`

```cpp
#include "Launcher.h"

namespace shell {

Launcher::Launcher() {}
Launcher::~Launcher() {}

void Launcher::draw(render::Renderer& renderer) {
    renderer.drawRect(x, y, width, height, 0xFF444444);
    ui::View::draw(renderer);
}

} // namespace shell
```

---

## `src/shell/ShellApp.h`

```cpp
#pragma once
#include "../core/Application.h"
#include "Panel.h"
#include "Launcher.h"

namespace shell {

class ShellApp : public core::Application {
public:
    ShellApp();
    ~ShellApp();

    bool initialize();
    void run();

private:
    Panel panel;
    Launcher launcher;
};

} // namespace shell
```

## `src/shell/ShellApp.cpp`

```cpp
#include "ShellApp.h"

namespace shell {

ShellApp::ShellApp() {}
ShellApp::~ShellApp() {}

bool ShellApp::initialize() {
    core::Application::initialize();
    panel.setFrame(0, 0, 800, 40);
    launcher.setFrame(0, 40, 200, 400);
    return true;
}

void ShellApp::run() {
    core::Application::run();
}

} // namespace shell
```

---

# ✅ **SECTION: DEMO APPS**

## `/examples/demo_basic.cpp`

```cpp
#include "../src/core/Application.h"
#include "../src/ui/View.h"
#include "../src/ui/Button.h"
#include "../src/ui/Label.h"
#include "../src/render/legacy/SoftwareRenderer.h"

int main() {
    core::Application app;
    app.initialize();

    render::SoftwareRenderer renderer(800, 600);

    ui::Label label("Hello World");
    label.setFrame(10, 10, 200, 30);

    ui::Button button("Click Me");
    button.setFrame(10, 50, 200, 30);
    button.setOnClick([&]() {
        label.setText("Button Clicked!");
    });

    while (true) {
        renderer.clear(0xFF000000);
        label.draw(renderer);
        button.draw(renderer);
        renderer.present();
    }

    return 0;
}
```

---

## `/examples/demo_controls.cpp`

```cpp
#include "../src/core/Application.h"
#include "../src/ui/View.h"
#include "../src/ui/Button.h"
#include "../src/ui/Label.h"
#include "../src/ui/FlexLayout.h"
#include "../src/render/legacy/SoftwareRenderer.h"

int main() {
    core::Application app;
    app.initialize();

    render::SoftwareRenderer renderer(800, 600);

    ui::View root;
    root.setFrame(0, 0, 800, 600);

    auto label = std::make_shared<ui::Label>("Controls Demo");
    auto b1 = std::make_shared<ui::Button>("One");
    auto b2 = std::make_shared<ui::Button>("Two");
    auto b3 = std::make_shared<ui::Button>("Three");

    root.addChild(label);
    root.addChild(b1);
    root.addChild(b2);
    root.addChild(b3);

    ui::FlexLayout layout;
    layout.apply(root);

    while (true) {
        renderer.clear(0xFF000000);
        root.draw(renderer);
        renderer.present();
    }

    return 0;
}
```

---

# 🎉 **DONE**

---

## 13. LICENSING RULES

- This project is licensed under the **MIT License**.
- Copilot must:
  - Include the following header at the top of every new `.h` and `.cpp` file:

    ```cpp
    // MIT License
    // Copyright (c) 2026 Palaash
    ```

  - Not introduce any code that requires a non‑MIT‑compatible license.
  - Not add any external dependencies that are not MIT, BSD, Apache‑2.0, or public domain.


