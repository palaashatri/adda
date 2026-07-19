package com.aurora.platform.storage;

import com.aurora.platform.core.AuroraDocumentKind;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.time.Instant;
import java.time.format.DateTimeFormatter;
import java.util.Comparator;
import java.util.List;
import java.util.stream.Stream;

/**
 * Local autosave snapshots stored as Aurora native packages.
 */
public final class AutosaveStore {
    private static final int MAX_SNAPSHOTS_PER_KIND = 5;

    private final Path autosaveDirectory;

    public AutosaveStore(Path autosaveDirectory) {
        this.autosaveDirectory = autosaveDirectory;
    }

    /**
     * Saves a timestamped local autosave snapshot.
     */
    public Path saveSnapshot(AuroraDocumentKind kind, String title, String documentJson) throws IOException {
        Files.createDirectories(autosaveDirectory);
        String timestamp = DateTimeFormatter.ISO_INSTANT.format(Instant.now()).replace(':', '-');
        String safeTitle = sanitize(title == null || title.isBlank() ? "Untitled" : title);
        Path target = autosaveDirectory.resolve(kind.id() + "-" + safeTitle + "-" + timestamp + "." + kind.extension());
        AuroraNativePackage.save(target, kind, title, documentJson);
        prune(kind);
        return target;
    }

    /**
     * Lists autosave snapshots newest-first.
     */
    public List<Path> listSnapshots() throws IOException {
        if (!Files.exists(autosaveDirectory)) {
            return List.of();
        }
        try (Stream<Path> files = Files.list(autosaveDirectory)) {
            return files.filter(Files::isRegularFile)
                    .sorted(Comparator.comparing(this::lastModified).reversed())
                    .toList();
        }
    }

    private void prune(AuroraDocumentKind kind) throws IOException {
        List<Path> snapshots = listSnapshots().stream()
                .filter(path -> path.getFileName().toString().startsWith(kind.id() + "-"))
                .toList();
        for (int index = MAX_SNAPSHOTS_PER_KIND; index < snapshots.size(); index++) {
            Files.deleteIfExists(snapshots.get(index));
        }
    }

    private Instant lastModified(Path path) {
        try {
            return Files.getLastModifiedTime(path).toInstant();
        } catch (IOException exception) {
            return Instant.EPOCH;
        }
    }

    private static String sanitize(String value) {
        String sanitized = value.replaceAll("[^A-Za-z0-9._-]+", "-");
        if (sanitized.length() > 48) {
            return sanitized.substring(0, 48);
        }
        return sanitized;
    }
}
