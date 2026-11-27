package dev.jitscope.workloads;

/**
 * Worker class loaded in many copies by {@link CodeCacheCliff} via custom class loaders. Each copy
 * gets its own {@code Class} and, when invoked enough, its own compiled code, consuming code-cache
 * space.
 */
public final class CodeCacheWorker {

    /** Seed folded into the computation so copies are not trivially identical. */
    public final long seed;

    public CodeCacheWorker() {
        this(0L);
    }

    public CodeCacheWorker(long seed) {
        this.seed = seed;
    }

    /** A non-trivial method body so compilation consumes real code-cache space. */
    public long work(long x) {
        long a = x + seed;
        long b = a * 0x9E3779B97F4A7C15L;
        long c = (b ^ (b >>> 29)) + 0xC2B2AE3D27D4EB4FL;
        long d = (c ^ (c << 17)) * 0x165667B19E3779F9L;
        long e = d + (a ^ seed);
        long f = (e ^ (e >>> 31)) * 0xD6E8FEB86659FD93L;
        long g = f + (c ^ d);
        long h = (g ^ (g >>> 27)) * 0x9E3779B185EBCA87L;
        return h ^ (h >>> 33) ^ a ^ b ^ c ^ d ^ e ^ f ^ g;
    }
}
