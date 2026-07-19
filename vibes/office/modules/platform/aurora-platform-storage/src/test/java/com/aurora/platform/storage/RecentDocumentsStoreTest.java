package com.aurora.platform.storage;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.nio.file.Path;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;

class RecentDocumentsStoreTest {
    @TempDir
    Path tempDir;

    @Test
    void recordsRecentDocumentsNewestFirstWithoutDuplicates() throws Exception {
        RecentDocumentsStore store = new RecentDocumentsStore(tempDir.resolve("recent.txt"));
        Path first = tempDir.resolve("one.awrite");
        Path second = tempDir.resolve("two.awrite");

        store.record(first);
        store.record(second);
        store.record(first);

        List<Path> recent = store.load();
        assertEquals(first.toAbsolutePath().normalize(), recent.get(0));
        assertEquals(second.toAbsolutePath().normalize(), recent.get(1));
        assertEquals(2, recent.size());
    }
}
