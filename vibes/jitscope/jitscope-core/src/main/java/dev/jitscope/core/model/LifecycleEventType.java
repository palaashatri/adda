package dev.jitscope.core.model;

/** Kinds of lifecycle transitions in a per-method timeline. */
public enum LifecycleEventType {
    COMPILATION_STARTED,
    COMPILATION_FINISHED,
    COMPILATION_FAILED,
    COMPILER_PHASE,
    DEOPTIMIZATION,
    INLINED,
    INLINE_FAILED,
    MADE_NOT_ENTRANT,
    MADE_NON_COMPILABLE,
    CODE_CACHE_FULL
}
