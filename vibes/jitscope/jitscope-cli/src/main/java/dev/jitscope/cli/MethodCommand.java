package dev.jitscope.cli;

import java.nio.file.Path;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.Callable;

import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;

import dev.jitscope.core.analysis.RecordingAnalyzer;
import dev.jitscope.core.model.MethodLifecycle;
import dev.jitscope.core.model.RecordingAnalysis;
import dev.jitscope.core.report.JsonRenderer;
import dev.jitscope.core.report.MarkdownRenderer;

@Command(name = "method", description = "Full per-method lifecycle for matching methods.")
public final class MethodCommand implements Callable<Integer> {

    @Parameters(index = "0", description = "Path to .jfr recording")
    Path jfr;

    @Parameters(index = "1", description = "Method pattern (substring, case-insensitive, on fqn/class/name)")
    String pattern;

    @Option(names = "--log-compilation", description = "Optional -XX:+LogCompilation XML log")
    Path logCompilation;

    @Option(names = "--format", description = "md|json", defaultValue = "md")
    OutputFormat format;

    @Override
    public Integer call() {
        try {
            RecordingAnalysis a = RecordingAnalyzer.analyze(jfr, logCompilation);
            String needle = pattern.toLowerCase(Locale.ROOT);
            List<MethodLifecycle> matches = a.lifecycles().stream()
                    .filter(l -> l.method().fqn().toLowerCase(Locale.ROOT).contains(needle)
                            || l.method().className().toLowerCase(Locale.ROOT).contains(needle)
                            || l.method().methodName().toLowerCase(Locale.ROOT).contains(needle))
                    .toList();
            String out = format == OutputFormat.json ? JsonRenderer.methodList(matches) : MarkdownRenderer.method(matches, pattern);
            System.out.println(out);
            return 0;
        } catch (Exception e) {
            System.err.println("error: " + e.getMessage());
            return 1;
        }
    }
}
