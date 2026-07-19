package com.aurora.platform.storage;

import java.nio.file.Path;

/**
 * Resolves Aurora user-data paths.
 */
public final class AuroraUserPaths {
    private final Path baseDirectory;

    public AuroraUserPaths(Path baseDirectory) {
        this.baseDirectory = baseDirectory;
    }

    /**
     * Default user-data location.
     */
    public static AuroraUserPaths defaults() {
        return new AuroraUserPaths(Path.of(System.getProperty("user.home"), ".aurora"));
    }

    /**
     * Root Aurora user-data directory.
     */
    public Path baseDirectory() {
        return baseDirectory;
    }

    /**
     * AI and UI settings file.
     */
    public Path settingsFile() {
        return baseDirectory.resolve("settings.properties");
    }

    /**
     * Recent documents file.
     */
    public Path recentDocumentsFile() {
        return baseDirectory.resolve("recent-documents.txt");
    }

    /**
     * Directory for local autosave snapshots.
     */
    public Path autosaveDirectory() {
        return baseDirectory.resolve("autosave");
    }
}
