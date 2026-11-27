package dev.jitscope.core;

import java.io.IOException;
import java.net.URI;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

/** Runs a bundled workload in a forked JVM with JFR recording into a temp file. */
public final class WorkloadJfrRecorder {

    private WorkloadJfrRecorder() {}

    public record Recorded(Path jfr, Path workDir, int exitCode, String stderr) {}

    public static Recorded record(Path tempDir, String mainClass, List<String> extraJvmFlags, List<String> appArgs,
            String jfcName) throws IOException, InterruptedException {
        Path out = tempDir.resolve(mainClass.substring(mainClass.lastIndexOf('.') + 1)
                .replace('$', '_') + "-" + System.nanoTime() + ".jfr");
        Path jfc = resolveJfc(jfcName);

        String javaBin = Path.of(System.getProperty("java.home"), "bin", "java").toString();
        String classpath = System.getProperty("java.class.path");

        List<String> cmd = new ArrayList<>();
        cmd.add(javaBin);
        if (extraJvmFlags != null) {
            cmd.addAll(extraJvmFlags);
        }
        cmd.add("-XX:StartFlightRecording=settings=" + jfc + ",filename=" + out);
        cmd.add("-cp");
        cmd.add(classpath);
        cmd.add(mainClass);
        if (appArgs != null) {
            cmd.addAll(appArgs);
        }

        ProcessBuilder pb = new ProcessBuilder(cmd);
        pb.redirectErrorStream(true);
        Process proc = pb.start();
        String output = new String(proc.getInputStream().readAllBytes());
        boolean finished = proc.waitFor(120, TimeUnit.SECONDS);
        if (!finished) {
            proc.destroyForcibly();
            throw new IOException("workload timed out: " + mainClass);
        }
        int exit = proc.exitValue();
        return new Recorded(out, tempDir, exit, output);
    }

    public static Path resolveJfc(String jfcName) throws IOException {
        if (jfcName.equals("default.jfc")) {
            Path javaHome = Path.of(System.getProperty("java.home"));
            Path def = javaHome.resolve("lib").resolve("jfr").resolve("default.jfc");
            if (Files.exists(def)) {
                return def;
            }
            throw new IOException("default.jfc not found at " + def);
        }
        if (jfcName.equals("jit-diag.jfc")) {
            var url = WorkloadJfrRecorder.class.getClassLoader().getResource("jfc/jit-diag.jfc");
            if (url == null) {
                throw new IOException("jit-diag.jfc resource not found");
            }
            try {
                return Path.of(URI.create(url.toString()));
            } catch (RuntimeException ex) {
                // jar URL; extract
                try (var in = url.openStream()) {
                    Path tmp = Files.createTempFile("jit-diag", ".jfc");
                    Files.write(tmp, in.readAllBytes());
                    tmp.toFile().deleteOnExit();
                    return tmp;
                }
            }
        }
        return Path.of(jfcName);
    }
}
