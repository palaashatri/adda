package dev.jitscope.cli;

import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.concurrent.Callable;

import picocli.CommandLine.Command;
import picocli.CommandLine.Option;

@Command(name = "init-jfc", description = "Write curated jit-diag.jfc JFR config.")
public final class InitJfcCommand implements Callable<Integer> {

    @Option(names = "--out", description = "Output path", defaultValue = "jit-diag.jfc")
    Path out;

    @Option(names = "--include-cpu-time", description = "Enable experimental jdk.CPUTimeSample (JDK 25+ Linux, JEP 509)")
    boolean includeCpuTime;

    @Override
    public Integer call() {
        try {
            String template = readTemplate();
            if (includeCpuTime) {
                template = template.replace(
                        "<event name=\"jdk.CPUTimeSample\">\n    <setting name=\"enabled\">false</setting>",
                        "<event name=\"jdk.CPUTimeSample\">\n    <setting name=\"enabled\">true</setting>");
            }
            Path target = out.toAbsolutePath();
            if (target.getParent() != null) {
                Files.createDirectories(target.getParent());
            }
            Files.writeString(target, template);
            System.err.println("wrote " + target);
            return 0;
        } catch (Exception e) {
            System.err.println("error: " + e.getMessage());
            return 1;
        }
    }

    private static String readTemplate() throws IOException {
        var url = InitJfcCommand.class.getClassLoader().getResource("jfc/jit-diag.jfc");
        if (url == null) {
            throw new IOException("bundled jit-diag.jfc resource not found");
        }
        // extract to string (works from jar or classes)
        try (InputStream in = url.openStream()) {
            return new String(in.readAllBytes(), java.nio.charset.StandardCharsets.UTF_8);
        }
    }
}
