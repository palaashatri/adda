package dev.jitscope.core.model;

import java.nio.file.Path;
import java.time.Duration;
import java.time.Instant;

/** Recording-level metadata. {@code recordingId} scopes all compileIds. */
public record RecordingMetadata(
        String recordingId,
        Path sourcePath,
        String jvmVersion,
        String jvmArguments,
        Instant startTime,
        Instant endTime,
        Duration duration,
        ConfigAudit configAudit) {

    public RecordingMetadata {
        if (duration == null) {
            duration = Duration.ZERO;
        }
    }
}
