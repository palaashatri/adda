// MIT License
// Copyright (c) 2026 Palaash

#include "WinPlatform.h"

#include <cstddef>
#include <cstring>

#include <windowsx.h>

#include "core/Event.h"

namespace {

const char* kWindowClassName = "VibedUiWindowClass";

LRESULT CALLBACK VibedUiWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    (void)wParam;
    (void)lParam;

    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcA(hwnd, message, wParam, lParam);
}

} // namespace

namespace platform {

WinPlatform::WinPlatform() {}

WinPlatform::~WinPlatform() {
    if (hdc != nullptr && hwnd != nullptr) {
        ReleaseDC(hwnd, hdc);
        hdc = nullptr;
    }
    if (hwnd != nullptr) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

bool WinPlatform::createWindow(int w, int h, const char* title) {
    const HINSTANCE instance = GetModuleHandleA(nullptr);

    WNDCLASSA windowClass = {};
    windowClass.lpfnWndProc = VibedUiWindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = kWindowClassName;

    if (RegisterClassA(&windowClass) == 0U && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }

    RECT windowRect = {0, 0, w, h};
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    hwnd = CreateWindowExA(
        0,
        kWindowClassName,
        title != nullptr ? title : "vibed-ui",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (hwnd == nullptr) {
        return false;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    hdc = GetDC(hwnd);
    return hdc != nullptr;
}

void WinPlatform::pumpEvents() {
    MSG message = {};
    while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
        core::Event queuedEvent;
        bool shouldQueue = false;

        switch (message.message) {
            case WM_QUIT:
                queuedEvent.type = core::EventType::Quit;
                shouldQueue = true;
                break;
            case WM_LBUTTONDOWN:
                queuedEvent.type = core::EventType::MouseDown;
                queuedEvent.x = GET_X_LPARAM(message.lParam);
                queuedEvent.y = GET_Y_LPARAM(message.lParam);
                shouldQueue = true;
                break;
            case WM_LBUTTONUP:
                queuedEvent.type = core::EventType::MouseUp;
                queuedEvent.x = GET_X_LPARAM(message.lParam);
                queuedEvent.y = GET_Y_LPARAM(message.lParam);
                shouldQueue = true;
                break;
            case WM_MOUSEMOVE:
                queuedEvent.type = core::EventType::MouseMove;
                queuedEvent.x = GET_X_LPARAM(message.lParam);
                queuedEvent.y = GET_Y_LPARAM(message.lParam);
                shouldQueue = true;
                break;
            case WM_KEYDOWN:
                // TODO: Add non-text key handling via WM_KEYDOWN where needed.
                shouldQueue = false;
                break;
            case WM_CHAR:
                queuedEvent.type = core::EventType::KeyDown;
                queuedEvent.keyCode = static_cast<int>(message.wParam);
                queuedEvent.textCode = static_cast<int>(message.wParam);
                shouldQueue = true;
                break;
            case WM_KEYUP:
                queuedEvent.type = core::EventType::KeyUp;
                queuedEvent.keyCode = static_cast<int>(message.wParam);
                queuedEvent.textCode = 0;
                shouldQueue = true;
                break;
            default:
                break;
        }

        if (shouldQueue) {
            core::EventQueue::push(queuedEvent);
        }

        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}

void WinPlatform::blit(const uint8_t* buffer, int w, int h) {
    if (hdc == nullptr || buffer == nullptr || w <= 0 || h <= 0) {
        return;
    }

    BITMAPINFO bitmapInfo = {};
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = w;
    bitmapInfo.bmiHeader.biHeight = -h;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    void* dibPixels = nullptr;
    HBITMAP dib = CreateDIBSection(hdc, &bitmapInfo, DIB_RGB_COLORS, &dibPixels, nullptr, 0);
    if (dib == nullptr || dibPixels == nullptr) {
        return;
    }

    const std::size_t byteCount = static_cast<std::size_t>(w) * static_cast<std::size_t>(h) * 4U;
    std::memcpy(dibPixels, buffer, byteCount);

    HDC memoryDc = CreateCompatibleDC(hdc);
    if (memoryDc != nullptr) {
        HGDIOBJ oldObject = SelectObject(memoryDc, dib);
        BitBlt(hdc, 0, 0, w, h, memoryDc, 0, 0, SRCCOPY);
        SelectObject(memoryDc, oldObject);
        DeleteDC(memoryDc);
    }

    DeleteObject(dib);
}

} // namespace platform
