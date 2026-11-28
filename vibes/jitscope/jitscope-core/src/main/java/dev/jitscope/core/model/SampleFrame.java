package dev.jitscope.core.model;

/** A single frame; {@code type} is the RecordedFrame.getType() value. */
public record SampleFrame(MethodKey method, String type, int lineNumber, int bytecodeIndex) {}
