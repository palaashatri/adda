// MIT License
// Copyright (c) 2026 Palaash

#include "MacPlatform.h"

#include "core/Event.h"

#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#endif

namespace platform {

MacPlatform::MacPlatform() {}

MacPlatform::~MacPlatform() {
#if defined(__APPLE__)
    if (window != nullptr) {
        NSWindow* nsWindow = (NSWindow*)window;
        [nsWindow close];
        [nsWindow release];
        window = nullptr;
    }
#endif
}

bool MacPlatform::createWindow(int w, int h, const char* title) {
#if defined(__APPLE__)
    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        const NSUInteger styleMask = NSWindowStyleMaskTitled
            | NSWindowStyleMaskClosable
            | NSWindowStyleMaskMiniaturizable
            | NSWindowStyleMaskResizable;

        const NSRect frame = NSMakeRect(100.0, 100.0, static_cast<CGFloat>(w), static_cast<CGFloat>(h));
        NSWindow* nsWindow = [[NSWindow alloc] initWithContentRect:frame styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];
        if (nsWindow == nil) {
            return false;
        }

        NSString* windowTitle = [NSString stringWithUTF8String:(title != nullptr ? title : "vibed-ui")];
        [nsWindow setTitle:windowTitle];
        [nsWindow makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];

        window = (void*)nsWindow;
        return true;
    }
#else
    (void)w;
    (void)h;
    (void)title;
    return false;
#endif
}

void MacPlatform::pumpEvents() {
#if defined(__APPLE__)
    @autoreleasepool {
        NSEvent* event = nil;
        do {
            event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:[NSDate distantPast]
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES];
            if (event != nil) {
                core::Event queuedEvent;
                bool shouldQueue = false;
                const NSEventType eventType = [event type];

                switch (eventType) {
                    case NSEventTypeLeftMouseDown:
                    case NSEventTypeRightMouseDown:
                    case NSEventTypeOtherMouseDown:
                        queuedEvent.type = core::EventType::MouseDown;
                        shouldQueue = true;
                        break;
                    case NSEventTypeLeftMouseUp:
                    case NSEventTypeRightMouseUp:
                    case NSEventTypeOtherMouseUp:
                        queuedEvent.type = core::EventType::MouseUp;
                        shouldQueue = true;
                        break;
                    case NSEventTypeMouseMoved:
                    case NSEventTypeLeftMouseDragged:
                    case NSEventTypeRightMouseDragged:
                    case NSEventTypeOtherMouseDragged:
                        queuedEvent.type = core::EventType::MouseMove;
                        shouldQueue = true;
                        break;
                    case NSEventTypeKeyDown:
                        queuedEvent.type = core::EventType::KeyDown;
                        queuedEvent.keyCode = static_cast<int>([event keyCode]);
                        shouldQueue = true;
                        break;
                    case NSEventTypeKeyUp:
                        queuedEvent.type = core::EventType::KeyUp;
                        queuedEvent.keyCode = static_cast<int>([event keyCode]);
                        shouldQueue = true;
                        break;
                    default:
                        break;
                }

                if (shouldQueue) {
                    const NSPoint point = [event locationInWindow];
                    queuedEvent.x = static_cast<int>(point.x);
                    queuedEvent.y = static_cast<int>(point.y);
                    core::EventQueue::push(queuedEvent);
                }

                [NSApp sendEvent:event];
            }
        } while (event != nil);

        [NSApp updateWindows];
    }
#endif
}

void MacPlatform::blit(const uint8_t* buffer, int w, int h) {
#if defined(__APPLE__)
    if (window == nullptr || buffer == nullptr || w <= 0 || h <= 0) {
        return;
    }

    @autoreleasepool {
        NSWindow* nsWindow = (NSWindow*)window;
        NSView* contentView = [nsWindow contentView];
        if (contentView == nil) {
            return;
        }

        [contentView lockFocus];
        NSGraphicsContext* nsContext = [NSGraphicsContext currentContext];
        if (nsContext == nil) {
            [contentView unlockFocus];
            return;
        }

        CGContextRef context = [nsContext CGContext];
        if (context == nullptr) {
            [contentView unlockFocus];
            return;
        }

        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        if (colorSpace == nullptr) {
            [contentView unlockFocus];
            return;
        }

        CGContextRef bitmapContext = CGBitmapContextCreate(
            const_cast<uint8_t*>(buffer),
            static_cast<size_t>(w),
            static_cast<size_t>(h),
            8U,
            static_cast<size_t>(w) * 4U,
            colorSpace,
            static_cast<CGBitmapInfo>(kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big));

        if (bitmapContext != nullptr) {
            CGImageRef image = CGBitmapContextCreateImage(bitmapContext);
            if (image != nullptr) {
                const NSRect bounds = [contentView bounds];
                CGContextDrawImage(context, CGRectMake(0.0, 0.0, bounds.size.width, bounds.size.height), image);
                CGImageRelease(image);
            }
            CGContextRelease(bitmapContext);
        }

        CGColorSpaceRelease(colorSpace);
        [contentView unlockFocus];
        [nsWindow displayIfNeeded];
    }
#else
    (void)buffer;
    (void)w;
    (void)h;
#endif
}

} // namespace platform