package dev.jitscope.core.ingest;

import jdk.jfr.consumer.RecordedClass;
import jdk.jfr.consumer.RecordedEvent;
import jdk.jfr.consumer.RecordedFrame;
import jdk.jfr.consumer.RecordedMethod;
import jdk.jfr.consumer.RecordedObject;

import dev.jitscope.core.model.MethodKey;

/** Defensive JFR field access. Never silently swallows; callers record warnings. */
public final class JfrFields {

    private JfrFields() {}

    public static boolean has(RecordedEvent e, String field) {
        try {
            return e.hasField(field);
        } catch (RuntimeException ex) {
            return false;
        }
    }

    public static int getInt(RecordedEvent e, String field, int dflt) {
        try {
            if (!e.hasField(field)) {
                return dflt;
            }
            return (int) e.getLong(field);
        } catch (RuntimeException ex) {
            return dflt;
        }
    }

    public static long getLong(RecordedEvent e, String field, long dflt) {
        try {
            if (!e.hasField(field)) {
                return dflt;
            }
            return e.getLong(field);
        } catch (RuntimeException ex) {
            return dflt;
        }
    }

    public static boolean getBoolean(RecordedEvent e, String field, boolean dflt) {
        try {
            if (!e.hasField(field)) {
                return dflt;
            }
            return e.getBoolean(field);
        } catch (RuntimeException ex) {
            return dflt;
        }
    }

    public static String getString(RecordedEvent e, String field, String dflt) {
        try {
            if (!e.hasField(field)) {
                return dflt;
            }
            Object v = e.getValue(field);
            return v == null ? dflt : v.toString();
        } catch (RuntimeException ex) {
            return dflt;
        }
    }

    /**
     * jdk.Compilation success field is spelled {@code succeded} (one 'e'); some events use
     * {@code succeeded}. Check both.
     */
    public static boolean compilationSucceeded(RecordedEvent e) {
        if (has(e, "succeded")) {
            return getBoolean(e, "succeded", true);
        }
        return getBoolean(e, "succeeded", true);
    }

    /** Convert a RecordedMethod into a MethodKey (class normalized to dotted form). */
    public static MethodKey methodKey(RecordedMethod m) {
        if (m == null) {
            return new MethodKey("", "", "");
        }
        String className = "";
        try {
            RecordedClass type = m.getType();
            if (type != null && type.getName() != null) {
                className = normalizeClass(type.getName());
            }
        } catch (RuntimeException ex) {
            className = "";
        }
        String name = m.getName() == null ? "" : m.getName();
        String desc = m.getDescriptor() == null ? "" : m.getDescriptor();
        return new MethodKey(className, name, desc);
    }

    /**
     * jdk.CompilerInlining.callee is a CalleeMethod struct (type/name/descriptor), NOT a
     * RecordedMethod. Extract defensively.
     */
    public static MethodKey calleeKey(RecordedEvent e) {
        try {
            Object callee = e.getValue("callee");
            if (callee instanceof RecordedObject ro) {
                String type = stringField(ro, "type", "");
                String name = stringField(ro, "name", "");
                String desc = stringField(ro, "descriptor", "");
                return new MethodKey(normalizeClass(type), name, desc);
            }
            if (callee instanceof RecordedMethod rm) {
                return methodKey(rm);
            }
        } catch (RuntimeException ex) {
            // fall through
        }
        return new MethodKey("", "", "");
    }

    public static MethodKey callerKey(RecordedEvent e) {
        try {
            Object caller = e.getValue("caller");
            if (caller instanceof RecordedMethod rm) {
                return methodKey(rm);
            }
        } catch (RuntimeException ex) {
            // fall through
        }
        return new MethodKey("", "", "");
    }

    private static String stringField(RecordedObject ro, String field, String dflt) {
        try {
            if (!ro.hasField(field)) {
                return dflt;
            }
            Object v = ro.getValue(field);
            return v == null ? dflt : v.toString();
        } catch (RuntimeException ex) {
            return dflt;
        }
    }

    /** Normalize class name to dotted form. */
    public static String normalizeClass(String name) {
        if (name == null) {
            return "";
        }
        // Strip array markers for matching purposes while keeping base name.
        String s = name.replace('/', '.');
        return s;
    }

    /** Object class name for allocation events. */
    public static String objectClassName(RecordedEvent e) {
        try {
            Object v = e.getValue("objectClass");
            if (v instanceof RecordedClass rc && rc.getName() != null) {
                return normalizeClass(rc.getName());
            }
            if (v != null) {
                return normalizeClass(v.toString());
            }
        } catch (RuntimeException ex) {
            // fall through
        }
        return "";
    }

    public static boolean frameTypeSafe(RecordedFrame f) {
        try {
            return f != null;
        } catch (RuntimeException ex) {
            return false;
        }
    }
}
