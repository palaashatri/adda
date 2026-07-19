package com.aurora.platform.plugin;

/**
 * Executable plugin contribution.
 */
public record PluginAction(String id, String label, Runnable handler) {
    public PluginAction {
        if (id == null || id.isBlank()) {
            throw new IllegalArgumentException("Plugin action id is required");
        }
        if (label == null || label.isBlank()) {
            throw new IllegalArgumentException("Plugin action label is required");
        }
        handler = handler == null ? () -> {
        } : handler;
    }

    /**
     * Executes this contribution.
     */
    public void execute() {
        handler.run();
    }
}
