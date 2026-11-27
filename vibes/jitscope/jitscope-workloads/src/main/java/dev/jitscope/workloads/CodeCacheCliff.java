package dev.jitscope.workloads;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

/**
 * Deterministic code-cache-pressure workload.
 *
 * <p>Designed to run with {@code -XX:ReservedCodeCacheSize=10m}. Dynamically loads and invokes
 * copies of {@link CodeCacheWorker} via custom {@link ClassLoader}s; loaded class loaders and
 * classes are retained in a {@link List} so nothing is unloaded. Each copy is invoked enough times
 * to be JIT-compiled, filling the code cache until {@code jdk.CodeCacheFull} fires or code-cache
 * pressure (high occupancy) is observable in {@code jdk.CodeCacheStatistics}.
 */
public final class CodeCacheCliff {

    /** Public sink so invocation results cannot be eliminated. */
    public static volatile long sink;

    /** Retained loaders/classes to prevent class unloading. */
    public static final List<Object> RETAINED = new ArrayList<>();

    private static final String WORKER_NAME = "dev.jitscope.workloads.CodeCacheWorker";
    private static final String WORKER_RESOURCE = "/dev/jitscope/workloads/CodeCacheWorker.class";

    private CodeCacheCliff() {}

    /** Custom class loader that defines a private copy of {@link CodeCacheWorker}. */
    public static final class ByteArrayClassLoader extends ClassLoader {
        private final byte[] bytes;

        public ByteArrayClassLoader(byte[] bytes) {
            super(CodeCacheCliff.class.getClassLoader());
            this.bytes = bytes.clone();
        }

        @Override
        protected Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
            synchronized (getClassLoadingLock(name)) {
                Class<?> loaded = findLoadedClass(name);
                if (loaded != null) {
                    return loaded;
                }
                if (name.equals(WORKER_NAME)) {
                    Class<?> defined = defineClass(name, bytes, 0, bytes.length);
                    if (resolve) {
                        resolveClass(defined);
                    }
                    return defined;
                }
                return super.loadClass(name, resolve);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        int maxCopies = args.length > 0 ? Integer.parseInt(args[0]) : 20_000;
        int invocationsPerCopy = args.length > 1 ? Integer.parseInt(args[1]) : 2_000;

        byte[] workerBytes = readWorkerBytes();
        Class<?> workMethodHolder = null;

        for (int copy = 0; copy < maxCopies; copy++) {
            ByteArrayClassLoader loader = new ByteArrayClassLoader(workerBytes);
            Class<?> workerClass;
            try {
                workerClass = Class.forName(WORKER_NAME, true, loader);
            } catch (LinkageError | ClassNotFoundException e) {
                // Stop when the runtime can no longer define more copies (e.g. code cache full).
                break;
            }

            Object instance;
            try {
                instance = workerClass.getDeclaredConstructor(long.class).newInstance((long) copy);
            } catch (ReflectiveOperationException e) {
                instance = workerClass.getDeclaredConstructor().newInstance();
            }

            java.lang.reflect.Method work = workerClass.getMethod("work", long.class);
            try {
                for (int i = 0; i < invocationsPerCopy; i++) {
                    sink += (long) work.invoke(instance, (long) i);
                }
            } catch (ReflectiveOperationException e) {
                // Keep going; pressure is what matters.
            } catch (LinkageError e) {
                break;
            }

            // Retain loader + class so no unloading can reclaim code-cache space.
            RETAINED.add(loader);
            RETAINED.add(workerClass);
            workMethodHolder = workerClass;

        }

        if (workMethodHolder != null) {
            sink ^= workMethodHolder.getName().hashCode();
        }
        // Keep sink observable.
        if (sink == 42L) {
            System.err.println("sink=" + sink + " retained=" + RETAINED.size());
        }
    }

    private static byte[] readWorkerBytes() throws IOException {
        try (InputStream in = CodeCacheCliff.class.getResourceAsStream(WORKER_RESOURCE)) {
            if (in == null) {
                throw new IOException("worker class resource not found: " + WORKER_RESOURCE);
            }
            return in.readAllBytes();
        }
    }
}
