package dev.jitscope.core.model;

/**
 * Identity of a JVM method. {@code className} uses JVM internal name with '/' separators as reported
 * by JFR (e.g. {@code dev/jitscope/workloads/DeoptStorm}); {@code fqn()} renders dotted form.
 */
public record MethodKey(String className, String methodName, String descriptor) {

    public MethodKey {
        if (className == null) {
            className = "";
        }
        if (methodName == null) {
            methodName = "";
        }
        if (descriptor == null) {
            descriptor = "";
        }
    }

    /** Human-readable fully-qualified name: {@code com.example.Foo.bar(I)V}. */
    public String fqn() {
        String dotted = className.replace('/', '.');
        return dotted + "." + methodName + descriptor;
    }

    /** Simple binary class name without package. */
    public String simpleClassName() {
        String dotted = className.replace('/', '.');
        int idx = Math.max(dotted.lastIndexOf('.'), dotted.lastIndexOf('$'));
        return idx >= 0 ? dotted.substring(idx + 1) : dotted;
    }

    @Override
    public String toString() {
        return fqn();
    }
}
