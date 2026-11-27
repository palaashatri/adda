package dev.jitscope.cli;

import java.util.concurrent.Callable;

import picocli.CommandLine.Command;

/**
 * Launch the local STDIO MCP server. Delegates to jitscope-mcp. STDIO discipline: System.out reserved
 * for JSON-RPC frames; logging to System.err. (Server implemented in jitscope-mcp / Phase 4.)
 */
@Command(name = "mcp", description = "Launch local STDIO MCP server (delegates to jitscope-mcp).")
public final class McpCommand implements Callable<Integer> {

    @Override
    public Integer call() {
        try {
            Class<?> server = Class.forName("dev.jitscope.mcp.JitScopeMcp");
            var main = server.getMethod("main", String[].class);
            main.invoke(null, (Object) new String[0]);
            return 0;
        } catch (ClassNotFoundException e) {
            System.err.println("MCP server not available: dev.jitscope.mcp.JitScopeMcp not on classpath "
                    + "(jitscope-mcp / Phase 4).");
            return 2;
        } catch (Exception e) {
            System.err.println("error launching MCP server: " + e.getMessage());
            return 1;
        }
    }
}
