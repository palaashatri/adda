package dev.jitscope.core.model;

/**
 * An inlining decision. {@code caller} is a RecordedMethod; {@code callee} is a CalleeMethod struct
 * (type/name/descriptor), NOT a RecordedMethod. Joins keyed on {@code (recordingId, compileId)}.
 */
public record InliningRecord(
        int compileId,
        MethodKey caller,
        MethodKey callee,
        int bci,
        boolean succeeded,
        String message,
        int bytes) {}
