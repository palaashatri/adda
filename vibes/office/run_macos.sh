#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

if [[ -z "${JAVA_HOME:-}" ]]; then
    if /usr/libexec/java_home -v 21 >/dev/null 2>&1; then
        export JAVA_HOME="$(/usr/libexec/java_home -v 21)"
    elif /usr/libexec/java_home -v 17 >/dev/null 2>&1; then
        export JAVA_HOME="$(/usr/libexec/java_home -v 17)"
    else
        echo "Aurora Office requires Java 17 or newer. Install a JDK, then rerun this script." >&2
        exit 1
    fi
fi

JAVA_MAJOR="$("$JAVA_HOME/bin/java" -version 2>&1 | awk -F[\".] '/version/ {print $2}')"
if [[ "$JAVA_MAJOR" -lt 17 ]]; then
    echo "Aurora Office requires Java 17 or newer. Current JAVA_HOME is $JAVA_HOME." >&2
    exit 1
fi

export GRADLE_USER_HOME="${GRADLE_USER_HOME:-$ROOT_DIR/.gradle-home}"

if [[ "$#" -gt 0 ]]; then
    exec "$ROOT_DIR/gradlew" --no-daemon "$@"
else
    exec "$ROOT_DIR/gradlew" --no-daemon :modules:apps:aurora-launcher:run
fi
