package dev.jitscope.cli;

import picocli.CommandLine;
import picocli.CommandLine.Command;

/** JITScope CLI. Thin adapter; all diagnostic logic in jitscope-core. */
@Command(
        name = "jitscope",
        mixinStandardHelpOptions = true,
        version = "jitscope 0.1.0-SNAPSHOT",
        description = "Local-first, read-only HotSpot JVM JIT diagnostics.",
        subcommands = {
                AnalyzeCommand.class,
                MethodCommand.class,
                DiffCommand.class,
                ConfigCheckCommand.class,
                InitJfcCommand.class,
                McpCommand.class
        })
public final class JitScopeMain implements Runnable {

    @Override
    public void run() {
        CommandLine.usage(this, System.out);
    }

    public static void main(String[] args) {
        int exit = new CommandLine(new JitScopeMain()).execute(args);
        System.exit(exit);
    }
}
