package com.aurora.platform.storage;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

/**
 * Persists recently opened or saved Aurora document paths.
 */
public final class RecentDocumentsStore {
    private static final int MAX_RECENT_DOCUMENTS = 12;

    private final Path recentFile;

    public RecentDocumentsStore(Path recentFile) {
        this.recentFile = recentFile;
    }

    /**
     * Loads recent documents newest-first.
     */
    public List<Path> load() throws IOException {
        if (!Files.exists(recentFile)) {
            return List.of();
        }
        List<Path> result = new ArrayList<>();
        for (String line : Files.readAllLines(recentFile, StandardCharsets.UTF_8)) {
            if (!line.isBlank()) {
                result.add(Path.of(line));
            }
        }
        return List.copyOf(result);
    }

    /**
     * Adds a document path as the most recent entry.
     */
    public void record(Path path) throws IOException {
        if (path == null) {
            return;
        }
        Set<Path> unique = new LinkedHashSet<>();
        unique.add(path.toAbsolutePath().normalize());
        unique.addAll(load());
        List<String> lines = unique.stream()
                .limit(MAX_RECENT_DOCUMENTS)
                .map(Path::toString)
                .toList();
        Files.createDirectories(recentFile.toAbsolutePath().getParent());
        Files.write(recentFile, lines, StandardCharsets.UTF_8);
    }
}
