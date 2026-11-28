package dev.jitscope.core.model;

import java.time.Instant;
import java.util.List;

/** A sampled stack (timestamp + ordered frames). Used for time-windowed correlation. */
public record Sample(Instant timestamp, String thread, List<SampleFrame> frames) {
    public Sample {
        frames = frames == null ? List.of() : List.copyOf(frames);
    }
}
