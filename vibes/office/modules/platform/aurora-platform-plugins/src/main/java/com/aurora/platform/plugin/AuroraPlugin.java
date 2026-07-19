package com.aurora.platform.plugin;

/**
 * ServiceLoader-discoverable Aurora plugin contract.
 */
public interface AuroraPlugin {
    /**
     * Stable plugin identifier.
     */
    String id();

    /**
     * User-facing plugin name.
     */
    String name();

    /**
     * Registers plugin contributions.
     */
    void activate(AuroraPluginContext context);

    /**
     * Releases plugin resources.
     */
    void deactivate();
}
