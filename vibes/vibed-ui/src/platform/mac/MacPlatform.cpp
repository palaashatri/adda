// MIT License
// Copyright (c) 2026 Palaash

#include "MacPlatform.h"

#include "core/Event.h"

#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>

@interface VibedContentView : NSView
@end

@implementation VibedContentView

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)isFlipped {
    return YES;
}

@end

@interface VibedWindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation VibedWindowDelegate

- (void)windowWillClose:(NSNotification*)notification {
    (void)notification;
    core::Event quitEvent;
    quitEvent.type = core::EventType::Quit;
    core::EventQueue::push(quitEvent);
}

@end

namespace {

void setupMainMenuIfNeeded() {
    if ([NSApp mainMenu] != nil) {
        return;
    }

    NSMenu* mainMenu = [[NSMenu alloc] initWithTitle:@"MainMenu"];

    NSMenuItem* appItem = [[NSMenuItem alloc] initWithTitle:@"Application" action:nil keyEquivalent:@""];
    [mainMenu addItem:appItem];

    NSMenu* appMenu = [[NSMenu alloc] initWithTitle:@"Application"];
    NSString* appName = [[NSProcessInfo processInfo] processName];
    NSString* quitTitle = [@"Quit " stringByAppendingString:appName];
    NSMenuItem* quitItem = [[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"];
    [appMenu addItem:quitItem];
    [appItem setSubmenu:appMenu];

    NSMenuItem* fileItem = [[NSMenuItem alloc] initWithTitle:@"File" action:nil keyEquivalent:@""];
    [mainMenu addItem:fileItem];

    NSMenu* fileMenu = [[NSMenu alloc] initWithTitle:@"File"];
    NSMenuItem* closeItem = [[NSMenuItem alloc] initWithTitle:@"Close Window" action:@selector(performClose:) keyEquivalent:@"w"];
    [fileMenu addItem:closeItem];
    [fileItem setSubmenu:fileMenu];

    [NSApp setMainMenu:mainMenu];

    [closeItem release];
    [fileMenu release];
    [fileItem release];
    [quitItem release];
    [appMenu release];
    [appItem release];
    [mainMenu release];
}

NSString* keyEventCharacters(NSEvent* event) {
    NSString* chars = [event characters];
    if (chars != nil && [chars length] > 0) {
        return chars;
    }
    return [event charactersIgnoringModifiers];
}

} // namespace
#endif

namespace platform {

MacPlatform::MacPlatform() {}

MacPlatform::~MacPlatform() {
#if defined(__APPLE__)
    if (window != nullptr) {
        NSWindow* nsWindow = (NSWindow*)window;
        [nsWindow setDelegate:nil];
        [nsWindow close];
        [nsWindow release];
        window = nullptr;
    }
    if (windowDelegate != nullptr) {
        [(id)windowDelegate release];
        windowDelegate = nullptr;
    }
#endif
}

bool MacPlatform::createWindow(int w, int h, const char* title) {
#if defined(__APPLE__)
    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        setupMainMenuIfNeeded();
        [NSApp finishLaunching];

        const NSUInteger styleMask = NSWindowStyleMaskTitled
            | NSWindowStyleMaskClosable
            | NSWindowStyleMaskMiniaturizable
            | NSWindowStyleMaskResizable;

        const NSRect frame = NSMakeRect(100.0, 100.0, static_cast<CGFloat>(w), static_cast<CGFloat>(h));
        NSWindow* nsWindow = [[NSWindow alloc] initWithContentRect:frame styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];
        if (nsWindow == nil) {
            return false;
        }

        VibedContentView* contentView = [[VibedContentView alloc] initWithFrame:frame];
        [nsWindow setContentView:contentView];
        [nsWindow makeFirstResponder:contentView];
        [contentView release];

        NSString* windowTitle = [NSString stringWithUTF8String:(title != nullptr ? title : "vibed-ui")];
        [nsWindow setTitle:windowTitle];
        [nsWindow setAcceptsMouseMovedEvents:YES];

        VibedWindowDelegate* delegate = [[VibedWindowDelegate alloc] init];
        [nsWindow setDelegate:delegate];
        windowDelegate = (void*)delegate;

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
                        queuedEvent.textCode = 0;
                        {
                            NSString* chars = keyEventCharacters(event);
                            if (chars != nil && [chars length] > 0) {
                                const unichar ch = [chars characterAtIndex:0];
                                if (ch <= 0x7F) {
                                    queuedEvent.textCode = static_cast<int>(ch);
                                }
                            }
                        }
                        shouldQueue = true;
                        break;
                    case NSEventTypeKeyUp:
                        queuedEvent.type = core::EventType::KeyUp;
                        queuedEvent.keyCode = static_cast<int>([event keyCode]);
                        queuedEvent.textCode = 0;
                        {
                            NSString* chars = keyEventCharacters(event);
                            if (chars != nil && [chars length] > 0) {
                                const unichar ch = [chars characterAtIndex:0];
                                if (ch <= 0x7F) {
                                    queuedEvent.textCode = static_cast<int>(ch);
                                }
                            }
                        }
                        shouldQueue = true;
                        break;
                    default:
                        break;
                }

                if (shouldQueue) {
                    NSWindow* nsWindow = (NSWindow*)window;
                    NSView* contentView = [nsWindow contentView];
                    const NSPoint windowPoint = [event locationInWindow];
                    const NSPoint viewPoint = contentView != nil
                        ? [contentView convertPoint:windowPoint fromView:nil]
                        : windowPoint;
                    queuedEvent.x = static_cast<int>(viewPoint.x);
                    queuedEvent.y = static_cast<int>(viewPoint.y);
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
            static_cast<CGBitmapInfo>(kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host));

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