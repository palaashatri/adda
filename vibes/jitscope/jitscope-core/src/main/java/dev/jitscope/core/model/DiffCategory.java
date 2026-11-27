package dev.jitscope.core.model;

/** Regression diff categories. */
public enum DiffCategory {
    TIER_REGRESSION,
    INLINING_REGRESSION,
    DEOPT_REGRESSION,
    HOTNESS_SHIFT,
    COMPILATION_REGRESSION,
    CODE_CACHE_REGRESSION,
    SAMPLING_CONFIG_DIFFERENCE,
    JVM_VERSION_DIFFERENCE
}
