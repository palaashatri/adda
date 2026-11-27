package dev.jitscope.cli;

import java.nio.file.Path;
import java.util.concurrent.Callable;

import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;

import dev.jitscope.core.analysis.RecordingAnalyzer;
import dev.jitscope.core.model.RecordingAnalysis;
import dev.jitscope.core.report.JsonRenderer;
import dev.jitscope.core.report.MarkdownRenderer;

@Command(name = "analyze", description = "Analyze a .jfr recording: audit + top hot methods + prioritized findings.")
public final class AnalyzeCommand implements Callable<Integer> {

    @Parameters(index = "0", description = "Path to .jfr recording")
    Path jfr;

    @Option(names = "--log-compilation", description = "Optional -XX:+LogCompilation XML log")
    Path logCompilation;

    @Option(names = "--format", description = "md|json", defaultValue = "md")
    OutputFormat format;

    @Option(names = "--top", description = "Top N hot methods", defaultValue = "20")
    int top;

    @Override
    public Integer call() {
        try {
            RecordingAnalysis a = RecordingAnalyzer.analyze(jfr, logCompilation);
            String out = format == OutputFormat.json ? JsonRenderer.analysis(a) : MarkdownRenderer.analysis(a, top);
            System.out.println(out);
            return 0;
        } catch (Exception e) {
            System.err.println("error: " + e.getMessage());
            return 1;
        }
    }
}
