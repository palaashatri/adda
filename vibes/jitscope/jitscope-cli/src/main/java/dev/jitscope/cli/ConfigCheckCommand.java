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

@Command(name = "config-check", description = "Audit JFR settings; report missing events/thresholds.")
public final class ConfigCheckCommand implements Callable<Integer> {

    @Parameters(index = "0", description = "Path to .jfr recording")
    Path jfr;

    @Option(names = "--format", description = "md|json", defaultValue = "md")
    OutputFormat format;

    @Override
    public Integer call() {
        try {
            RecordingAnalysis a = RecordingAnalyzer.analyze(jfr);
            var audit = a.metadata().configAudit();
            String out = format == OutputFormat.json ? JsonRenderer.configCheck(audit) : MarkdownRenderer.configCheck(audit);
            System.out.println(out);
            return 0;
        } catch (Exception e) {
            System.err.println("error: " + e.getMessage());
            return 1;
        }
    }
}
