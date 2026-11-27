package dev.jitscope.cli;

import java.nio.file.Path;
import java.util.List;
import java.util.concurrent.Callable;

import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;

import dev.jitscope.core.analysis.DiffEngine;
import dev.jitscope.core.analysis.RecordingAnalyzer;
import dev.jitscope.core.model.DiffFinding;
import dev.jitscope.core.model.RecordingAnalysis;
import dev.jitscope.core.report.JsonRenderer;
import dev.jitscope.core.report.MarkdownRenderer;

@Command(name = "diff", description = "Structured regression diff between two recordings.")
public final class DiffCommand implements Callable<Integer> {

    @Parameters(index = "0", description = "Baseline .jfr")
    Path baseline;

    @Parameters(index = "1", description = "Candidate .jfr")
    Path candidate;

    @Option(names = "--format", description = "md|json", defaultValue = "md")
    OutputFormat format;

    @Option(names = "--min-delta", description = "Minimum normalized sample-share delta", defaultValue = "0.01")
    double minDelta;

    @Override
    public Integer call() {
        try {
            RecordingAnalysis base = RecordingAnalyzer.analyze(baseline);
            RecordingAnalysis cand = RecordingAnalyzer.analyze(candidate);
            List<DiffFinding> diffs = DiffEngine.diff(base, cand, minDelta);
            String note = diffNote(base, cand);
            String out = format == OutputFormat.json ? JsonRenderer.diff(diffs, note) : MarkdownRenderer.diff(diffs, note);
            System.out.println(out);
            return 0;
        } catch (Exception e) {
            System.err.println("error: " + e.getMessage());
            return 1;
        }
    }

    private static String diffNote(RecordingAnalysis base, RecordingAnalysis cand) {
        return "baseline=" + base.metadata().sourcePath() + " candidate=" + cand.metadata().sourcePath()
                + "; normalized sample shares; raw cross-recording counts not compared; "
                + "JVM baseline=" + base.metadata().jvmVersion() + " candidate=" + cand.metadata().jvmVersion();
    }
}
