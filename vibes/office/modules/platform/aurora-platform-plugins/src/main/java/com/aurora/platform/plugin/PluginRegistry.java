package com.aurora.platform.plugin;

import java.util.ArrayList;
import java.util.List;
import java.util.ServiceLoader;

/**
 * In-memory plugin registry and ServiceLoader bootstrapper.
 */
public final class PluginRegistry implements AuroraPluginContext {
    private final List<AuroraPlugin> plugins = new ArrayList<>();
    private final List<PluginAction> menuActions = new ArrayList<>();
    private final List<PluginAction> toolbarActions = new ArrayList<>();
    private final List<PluginAction> aiCommands = new ArrayList<>();

    /**
     * Loads and activates plugins from the thread context class loader.
     */
    public static PluginRegistry loadInstalledPlugins() {
        PluginRegistry registry = new PluginRegistry();
        ServiceLoader.load(AuroraPlugin.class).forEach(registry::activate);
        return registry;
    }

    /**
     * Activates a plugin and records it for later deactivation.
     */
    public void activate(AuroraPlugin plugin) {
        plugin.activate(this);
        plugins.add(plugin);
    }

    /**
     * Deactivates all active plugins.
     */
    public void deactivateAll() {
        for (AuroraPlugin plugin : List.copyOf(plugins)) {
            plugin.deactivate();
        }
        plugins.clear();
        menuActions.clear();
        toolbarActions.clear();
        aiCommands.clear();
    }

    @Override
    public void addMenuAction(PluginAction action) {
        menuActions.add(action);
    }

    @Override
    public void addToolbarAction(PluginAction action) {
        toolbarActions.add(action);
    }

    @Override
    public void addAiCommand(PluginAction action) {
        aiCommands.add(action);
    }

    /**
     * Menu actions currently contributed by active plugins.
     */
    public List<PluginAction> menuActions() {
        return List.copyOf(menuActions);
    }

    /**
     * Toolbar actions currently contributed by active plugins.
     */
    public List<PluginAction> toolbarActions() {
        return List.copyOf(toolbarActions);
    }

    /**
     * AI commands currently contributed by active plugins.
     */
    public List<PluginAction> aiCommands() {
        return List.copyOf(aiCommands);
    }
}
