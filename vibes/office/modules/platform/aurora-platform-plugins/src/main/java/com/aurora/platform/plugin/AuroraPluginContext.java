package com.aurora.platform.plugin;

/**
 * Contribution surface exposed to plugins.
 */
public interface AuroraPluginContext {
    /**
     * Adds a menu action contributed by a plugin.
     */
    void addMenuAction(PluginAction action);

    /**
     * Adds a toolbar action contributed by a plugin.
     */
    void addToolbarAction(PluginAction action);

    /**
     * Adds an AI command contributed by a plugin.
     */
    void addAiCommand(PluginAction action);
}
