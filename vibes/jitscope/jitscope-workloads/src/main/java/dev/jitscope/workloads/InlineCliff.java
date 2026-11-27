package dev.jitscope.workloads;

/**
 * Deterministic inlining-cliff workload with two comparable modes.
 *
 * <p>{@code --mode=inlineable} (default): a monomorphic call site; the short-lived
 * {@code record Pair(long a, long b)} allocated inside the hot helper can be inlined and
 * scalar-replaced (escape analysis), minimizing allocation.
 *
 * <p>{@code --mode=cliff}: forces a megamorphic call site so inlining fails; the short-lived
 * {@code Pair} escapes and {@code jdk.ObjectAllocationSample} spikes.
 *
 * <p>Target method for findings: {@code dev.jitscope.workloads.InlineCliff.hot}.
 */
public final class InlineCliff {

    /** Public sink so the JIT cannot eliminate computed values. */
    public static volatile long sink;

    /** Short-lived value allocated inside the hot helper. */
    public record Pair(long a, long b) {}

    /** Factory for {@link Pair}; single implementation in inlineable mode, megamorphic in cliff. */
    public interface PairMaker {
        Pair make(long a, long b);
    }

    /** Baseline maker (monomorphic in inlineable mode). */
    public record SimpleMaker() implements PairMaker {
        @Override
        public Pair make(long a, long b) {
            return new Pair(a, b);
        }
    }

    /** Extra implementation #1 (megamorphic in cliff mode). */
    public record OffsetMaker1() implements PairMaker {
        @Override
        public Pair make(long a, long b) {
            return new Pair(a + 1, b);
        }
    }

    /** Extra implementation #2 (megamorphic in cliff mode). */
    public record OffsetMaker2() implements PairMaker {
        @Override
        public Pair make(long a, long b) {
            return new Pair(a, b + 1);
        }
    }

    /** Extra implementation #3 (megamorphic in cliff mode). */
    public record OffsetMaker3() implements PairMaker {
        @Override
        public Pair make(long a, long b) {
            return new Pair(a + 1, b + 1);
        }
    }

    private InlineCliff() {}

    /** Hot helper: allocates a short-lived {@link Pair} and reduces it. */
    public static long helper(PairMaker maker, long a, long b) {
        Pair p = maker.make(a, b);
        return p.a() + p.b();
    }

    /** Hot method calling {@link #helper}. */
    public static long hot(PairMaker maker, long a) {
        return helper(maker, a, a + 1);
    }

    public static void main(String[] args) {
        String mode = "inlineable";
        for (String arg : args) {
            if (arg.startsWith("--mode=")) {
                mode = arg.substring("--mode=".length());
            } else if (arg.equals("inlineable") || arg.equals("cliff")) {
                mode = arg;
            }
        }

        int iterations = 4_000_000;
        if (mode.equals("cliff")) {
            PairMaker[] makers = {
                new SimpleMaker(), new OffsetMaker1(), new OffsetMaker2(), new OffsetMaker3()
            };
            for (int i = 0; i < iterations; i++) {
                PairMaker m = makers[i & 3];
                sink += hot(m, i);
            }
        } else {
            PairMaker maker = new SimpleMaker();
            for (int i = 0; i < iterations; i++) {
                sink += hot(maker, i);
            }
        }

        // Keep sink observable.
        if (sink == 42L) {
            System.err.println("sink=" + sink);
        }
    }
}
