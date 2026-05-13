// MIT License
// Copyright (c) 2026 Palaash

#include "WinPlatform.h"

#include <cstddef>
#include <cstring>

#include <windowsx.h>
#include <dwmapi.h>
#include <dxgi1_6.h>

#include "core/Event.h"

namespace {

const char* kWindowClassName = "VibedUiWindowClass";

LRESULT CALLBACK VibedUiWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    (void)wParam;
    (void)lParam;

    switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
            // Rendering is driven by our own loop; just validate the dirty
            // region so Windows doesn't re-post WM_PAINT every frame.
            ValidateRect(hwnd, nullptr);
            return 0;
        case WM_ERASEBKGND:
            // Suppress GDI background erase to eliminate flicker on resize.
            return 1;
        default:
            break;
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
    if (hdc == nullptr) {
        return false;
    }

    detectDisplayCapabilities();
    return true;
}

void WinPlatform::detectDisplayCapabilities() {
    // Refresh rate from the monitor this window is on
    HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFOEXA monInfo = {};
    monInfo.cbSize = sizeof(monInfo);
    if (GetMonitorInfoA(hMon, &monInfo) != 0) {
        DEVMODEA devMode = {};
        devMode.dmSize = sizeof(devMode);
        if (EnumDisplaySettingsA(monInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode) != 0) {
            if (devMode.dmDisplayFrequency > 1) {
                refreshRateHz = static_cast<int>(devMode.dmDisplayFrequency);
            }
        }
    }

    // HDR capability via DXGI 1.6 (Windows 10 17134+ SDK)
    IDXGIFactory1* factory = nullptr;
    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1),
                                    reinterpret_cast<void**>(&factory));
    if (FAILED(hr) || factory == nullptr) {
        return;
    }

    IDXGIAdapter1* adapter = nullptr;
    for (UINT ai = 0; factory->EnumAdapters1(ai, &adapter) != DXGI_ERROR_NOT_FOUND; ++ai) {
        IDXGIOutput* output = nullptr;
        for (UINT oi = 0; adapter->EnumOutputs(oi, &output) != DXGI_ERROR_NOT_FOUND; ++oi) {
            IDXGIOutput6* output6 = nullptr;
            if (SUCCEEDED(output->QueryInterface(__uuidof(IDXGIOutput6),
                          reinterpret_cast<void**>(&output6)))) {
                DXGI_OUTPUT_DESC1 desc = {};
                if (SUCCEEDED(output6->GetDesc1(&desc))) {
                    // PQ (ST.2084) colour space or peak brightness > 400 nits → HDR capable
                    if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 ||
                        desc.MaxLuminance > 400.0f) {
                        hdrCapable = true;
                    }
                }
                output6->Release();
            }
            output->Release();
        }
        adapter->Release();
        if (hdrCapable) {
            break;
        }
    }
    factory->Release();
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
                {
                    const int vk = static_cast<int>(message.wParam);
                    const bool isNonTextKey =
                        (vk == VK_LEFT || vk == VK_RIGHT || vk == VK_UP || vk == VK_DOWN ||
                         vk == VK_ESCAPE || vk == VK_DELETE || vk == VK_HOME || vk == VK_END ||
                         vk == VK_PRIOR || vk == VK_NEXT || (vk >= VK_F1 && vk <= VK_F24));
                    if (isNonTextKey) {
                        queuedEvent.type = core::EventType::KeyDown;
                        queuedEvent.keyCode = vk;
                        queuedEvent.textCode = 0;
                        shouldQueue = true;
                    }
                }
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

void WinPlatform::waitForVSync() {
    // Synchronise with the DWM compositor tick.
    // On Windows 11 with a VRR-capable display the DWM itself operates at a
    // variable refresh rate, so this call paces presentation without fixing
    // the frame rate to any single static interval.
    DwmFlush();
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
        // Sync to the DWM VRR compositor tick before presenting.
        DwmFlush();
        BitBlt(hdc, 0, 0, w, h, memoryDc, 0, 0, SRCCOPY);
        SelectObject(memoryDc, oldObject);
        DeleteDC(memoryDc);
    }

    DeleteObject(dib);
}

} // namespace platform
