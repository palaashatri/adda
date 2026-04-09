// MIT License
// Copyright (c) 2026 Palaash

#include "LinuxPlatform.h"

#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "core/Event.h"

namespace platform {

LinuxPlatform::LinuxPlatform() {}

LinuxPlatform::~LinuxPlatform() {
    if (display != nullptr) {
        if (gc != 0) {
            XFreeGC(display, gc);
            gc = 0;
        }
        if (window != 0) {
            XDestroyWindow(display, window);
            window = 0;
        }
        XCloseDisplay(display);
        display = nullptr;
    }
}

bool LinuxPlatform::createWindow(int w, int h, const char* title) {
    display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        return false;
    }

    screen = DefaultScreen(display);
    window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        100,
        100,
        static_cast<unsigned int>(w),
        static_cast<unsigned int>(h),
        1,
        BlackPixel(display, screen),
        WhitePixel(display, screen));

    if (window == 0) {
        XCloseDisplay(display);
        display = nullptr;
        return false;
    }

    XStoreName(display, window, title != nullptr ? title : "vibed-ui");
    XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);

    wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);

    XMapWindow(display, window);
    gc = XCreateGC(display, window, 0, nullptr);
    XFlush(display);

    return gc != 0;
}

void LinuxPlatform::pumpEvents() {
    if (display == nullptr) {
        return;
    }

    while (XPending(display) > 0) {
        XEvent event = {};
        XNextEvent(display, &event);

        core::Event queuedEvent;
        bool shouldQueue = false;

        switch (event.type) {
            case ButtonPress:
                queuedEvent.type = core::EventType::MouseDown;
                queuedEvent.x = event.xbutton.x;
                queuedEvent.y = event.xbutton.y;
                shouldQueue = true;
                break;
            case ButtonRelease:
                queuedEvent.type = core::EventType::MouseUp;
                queuedEvent.x = event.xbutton.x;
                queuedEvent.y = event.xbutton.y;
                shouldQueue = true;
                break;
            case MotionNotify:
                queuedEvent.type = core::EventType::MouseMove;
                queuedEvent.x = event.xmotion.x;
                queuedEvent.y = event.xmotion.y;
                shouldQueue = true;
                break;
            case KeyPress:
                queuedEvent.type = core::EventType::KeyDown;
                queuedEvent.keyCode = event.xkey.keycode;
                shouldQueue = true;
                break;
            case KeyRelease:
                queuedEvent.type = core::EventType::KeyUp;
                queuedEvent.keyCode = event.xkey.keycode;
                shouldQueue = true;
                break;
            case ClientMessage:
                if (static_cast<Atom>(event.xclient.data.l[0]) == wmDeleteMessage) {
                    queuedEvent.type = core::EventType::Quit;
                    shouldQueue = true;
                }
                break;
            default:
                break;
        }

        if (shouldQueue) {
            core::EventQueue::push(queuedEvent);
        }
    }
}

void LinuxPlatform::blit(const uint8_t* buffer, int w, int h) {
    if (display == nullptr || window == 0 || gc == 0 || buffer == nullptr || w <= 0 || h <= 0) {
        return;
    }

    XImage* image = XCreateImage(
        display,
        DefaultVisual(display, screen),
        static_cast<unsigned int>(DefaultDepth(display, screen)),
        ZPixmap,
        0,
        reinterpret_cast<char*>(const_cast<uint8_t*>(buffer)),
        static_cast<unsigned int>(w),
        static_cast<unsigned int>(h),
        32,
        0);

    if (image == nullptr) {
        return;
    }

    XPutImage(display, window, gc, image, 0, 0, 0, 0, static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    XFlush(display);

    image->data = nullptr;
    XDestroyImage(image);
}

} // namespace platform
