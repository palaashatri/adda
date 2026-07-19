package com.aurora.format.nativepkg;

import com.aurora.platform.core.AuroraDocumentKind;
import com.aurora.platform.storage.AuroraNativePackage;
import com.aurora.platform.storage.LoadedAuroraPackage;

import java.io.IOException;
import java.nio.file.Path;
import java.util.Objects;

/**
 * Native Aurora format facade for `.awrite`, `.asheet`, and `.adeck` packages.
 */
public final class NativeFormatService {
    /**
     * Saves a native Aurora document package.
     */
    public void save(Path target, AuroraDocumentKind kind, String title, String documentJson) throws IOException {
        AuroraNativePackage.save(
                Objects.requireNonNull(target, "target"),
                Objects.requireNonNull(kind, "kind"),
                title,
                documentJson
        );
    }

    /**
     * Loads a native Aurora document package.
     */
    public LoadedAuroraPackage load(Path source) throws IOException {
        return AuroraNativePackage.load(Objects.requireNonNull(source, "source"));
    }

    /**
     * Whether this service supports the supplied path by extension.
     */
    public boolean supports(Path path) {
        return AuroraDocumentKind.fromExtension(path == null ? "" : path.toString()).isPresent();
    }
}
