package dev.jitscope.workloads;

/**
 * Deterministic deoptimization-storm workload.
 *
 * <p>Warms a monomorphic interface call site to Tier 4 (~200k iterations, writing to
 * {@code public static volatile long sink}), then swaps in 3 additional implementations and flips a
 * cold guard flag to trigger repeated {@code class_check} / {@code unstable_if} deopts and
 * recompilations.
 *
 * <p>Target method for findings: {@code dev.jitscope.workloads.DeoptStorm.hot}.
 */
public final class DeoptStorm {

    /** Public sink so the JIT cannot eliminate the computed values. */
    public static volatile long sink;

    /** Monomorphic-then-megamorphic interface used at the hot call site. */
    public interface Work {
        long apply(long x);
    }

    /** Implementation used during warmup (monomorphic). */
    public record ImplA() implements Work {
        @Override
        public long apply(long x) {
            return x + 1;
        }
    }

    /** Additional implementation #1 (forces megamorphic dispatch). */
    public record ImplB() implements Work {
        @Override
        public long apply(long x) {
            return x + 2;
        }
    }

    /** Additional implementation #2 (forces megamorphic dispatch). */
    public record ImplC() implements Work {
        @Override
        public long apply(long x) {
            return x + 3;
        }
    }

    /** Additional implementation #3 (forces megamorphic dispatch). */
    public record ImplD() implements Work {
        @Override
        public long apply(long x) {
            return x + 4;
        }
    }

    /** Cold guard; flipped after warmup to trigger {@code unstable_if} deopts. */
    public static volatile boolean guard;

    private DeoptStorm() {}

    /**
     * Hot method. Single call site that is monomorphic during warmup and megamorphic afterwards;
     * contains a branch that is never taken during warmup.
     */
    public static long hot(Work w, long x) {
        long r = w.apply(x);
        if (guard) {
            r ^= x;
        }
        return r;
    }

    public static void main(String[] args) {
        Work warm = new ImplA();

        // Warm monomorphic call site to Tier 4.
        for (int i = 0; i < 200_000; i++) {
            sink = hot(warm, i);
        }

        // Swap in 3 additional implementations + flip cold guard.
        Work[] impls = {new ImplA(), new ImplB(), new ImplC(), new ImplD()};
        guard = true;

        for (int round = 0; round < 16; round++) {
            for (Work w : impls) {
                for (int i = 0; i < 100_000; i++) {
                    sink = hot(w, i);
                }
            }
        }

        // Final monomorphic flush to provoke recompilation back to a stable form.
        guard = false;
        for (int i = 0; i < 200_000; i++) {
            sink = hot(warm, i);
        }

        // Keep sink observable.
        if (sink == 42L) {
            System.err.println("sink=" + sink);
        }
    }
}
