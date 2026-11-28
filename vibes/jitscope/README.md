# JITScope

**Local-first, read-only HotSpot JVM JIT diagnostics engine, CLI, and MCP server.** Apache-2.0.

Answers investigative compiler-performance questions from JVM recordings and artifacts *without* an
LLM producing the diagnosis:

- Which hot methods failed to reach or retain optimal compilation?
- What is the per-method compilation / inlining / deoptimization lifecycle?
- Is a deopt storm, inlining cliff, or code-cache stall actively degrading execution?
- What JIT behavior regressed between two recordings (code change, JVM flags, JDK 21 vs 25)?

**Engine-first, MCP-last.** All diagnostic logic lives in `jitscope-core`. `jitscope-cli` and
`jitscope-mcp` are thin adapters. **Local-first, zero network egress.** **Read-only, offline-first.**
v1 analyzes offline `.jfr` (+ optional `-XX:+LogCompilation` XML). No live attach. No proprietary
or vendor-specific references; only public OpenJDK APIs.

## Build

Java 21, Maven 3.9+. `--release 21`; runs on JDK 21 LTS and JDK 25 LTS.

```bash
mvn clean verify          # produces jitscope-cli/target/jitscope.jar (single runnable fat JAR)
```

## Capture a recording

```bash
# curated template (jdk.Compilation#threshold=0 ms; inlining/failure/deopt/codecache/execution/allocation; ...)
java -jar jitscope-cli/target/jitscope.jar init-jfc --out jit-diag.jfc            # [--include-cpu-time]

java -XX:StartFlightRecording=settings=jit-diag.jfc,filename=out.jfr -jar your-app.jar

# optional LogCompilation
java -XX:+UnlockDiagnosticVMOptions -XX:+LogCompilation -XX:LogFile=hotspot_pid.log \
     -XX:StartFlightRecording=settings=jit-diag.jfc,filename=out.jfr -jar your-app.jar
```

## CLI

```bash
java -jar jitscope-cli/target/jitscope.jar analyze out.jfr [--log-compilation hotspot_pid.log] [--format md|json] [--top 20]
java -jar jitscope-cli/target/jitscope.jar method out.jfr <pattern> [--log-compilation x] [--format md|json]
java -jar jitscope-cli/target/jitscope.jar diff baseline.jfr candidate.jfr [--format md|json] [--min-delta 0.01]
java -jar jitscope-cli/target/jitscope.jar config-check out.jfr [--format md|json]
java -jar jitscope-cli/target/jitscope.jar init-jfc [--out jit-diag.jfc] [--include-cpu-time]
java -jar jitscope-cli/target/jitscope.jar mcp
```

### Sample real output (bundled `DeoptStorm`, JDK 25, `jit-diag.jfc`; trimmed)

```text
# JITScope analysis
Recording: `/tmp/p5.jfr`
- JVM: `OpenJDK 64-Bit Server VM (25+36-LTS) for bsd-aarch64 JRE (25+36-LTS) (Zulu25.28+85-CA), ...`
- duration: 42 ms
- execution samples (jdk.ExecutionSample): 2
- code-cache: full=0 peakOccupancy=22.19%
## Recording audit
- completeness: **COMPLETE**
- jdk.Compilation#threshold(ms): 0
- disabled/missing required events: none
## Top hot methods
| method | self% | total% | frames | highestLevel | deopts | compilations |
| ... (hot + main; real rows omitted here) |
```

(Real `DeoptStorm` run emits `jdk.Compilation` level-4 + `jdk.Deoptimization` events; `hot` reaches
highestCompileLevel=4; `DEOPT_STORM` can be produced on `dev.jitscope.workloads.DeoptStorm.hot`
(see `mvn verify` Gate 2). The verbatim header lines above are real captured output; hot-method rows
and exact counts vary per run/JDK; nothing is fabricated.)

## Pathologies (evidence-aware)

`HOT_UNDEROPTIMIZED`, `DEOPT_STORM`, `INLINING_ALLOCATION_CLIFF`, `CODE_CACHE_PRESSURE`. Every
finding carries `EvidenceLevel` (`OBSERVED`/`INFERRED`/`UNKNOWN`; correlation noted in evidence as `CORRELATED`, not a separate level; §4 `CORRELATED` maps to `INFERRED`), structured evidence,
and a recommendation. **Statistical honesty:** seeing `Inlined` frames is observed proof of inlining
on that path; absence of `Inlined` is *not* proof of inlining failure; correlation ≠ causation;
`made_not_entrant`/`made_non_compilable` are only `OBSERVED` with `-XX:+LogCompilation`. Sampling:
keep `jdk.ExecutionSample` and `jdk.CPUTimeSample` (JDK 25+ Linux, JEP 509) separate; **never
compare raw counts across JDKs**; normalize to relative sample share; JDK 25 JEP 518 hygiene; flag
cross-JDK-major + sampling-config diffs. `jdk.Compilation#threshold > 0 ms` hides warmup
(`default.jfc`=1000 ms, `profile.jfc`=100 ms) -> `PARTIAL`; disabled/missing signals -> `INSUFFICIENT`
+ exact remediation.

## Bundled workloads (`jitscope-workloads`, zero-dependency)

- `dev.jitscope.workloads.DeoptStorm` — monomorphic warm to Tier 4, then 3 impls + flip cold guard -> `class_check`/`unstable_if` deopts + recompiles. `hot`.
- `dev.jitscope.workloads.InlineCliff` — `--mode=inlineable|cliff`; short-lived `record Pair`; cliff -> megamorphic/oversized -> inlining fails + allocation spike. `hot`/`helper`.
- `dev.jitscope.workloads.CodeCacheCliff` — `-XX:ReservedCodeCacheSize=4m` (tune; spec default 10m); dynamically load/invoke copies via retained custom `ClassLoader`s -> `jdk.CodeCacheFull` / pressure.

## MCP server (STDIO)

`java -jar jitscope-cli/target/jitscope.jar mcp` (or `dev.jitscope.mcp.JitScopeMcp`). **STDIO
discipline:** `System.out` is reserved for JSON-RPC frames; all logging -> `System.err`. **Exactly
five read-only tools** (structured JSON matching `jitscope-core` DTOs + concise Markdown); reject
nonexistent paths (`isError=true`); **no arbitrary shell commands or JVM flags**:

1. `check_recording_config(jfrPath)`
2. `analyze_recording(jfrPath, logCompilationPath?, topN?)`
3. `inspect_method(jfrPath, methodPattern, logCompilationPath?)`
4. `find_pathologies(jfrPath, types?, minSeverity?, logCompilationPath?)`
5. `diff_recordings(baselineJfrPath, candidateJfrPath, minSampleShareDelta?)`

### Client config

Claude Code (`~/.claude.json` or `.mcp.json`):

```json
{ "mcpServers": { "jitscope": { "command": "java", "args": ["-jar", "/abs/path/jitscope-cli/target/jitscope.jar", "mcp"] } } }
```

Codex (`~/.codex/config.toml`):

```toml
[mcp_servers.jitscope]
command = "java"
args = ["-jar", "/abs/path/jitscope-cli/target/jitscope.jar", "mcp"]
```

OpenCode (`opencode.json`):

```json
{ "$schema": "https://opencode.ai/config.json",
  "mcp": { "jitscope": { "type": "local", "command": ["java", "-jar", "/abs/path/jitscope-cli/target/jitscope.jar", "mcp"], "enabled": true } } }
```

## `-XX:+LogCompilation` enricher

Streaming StAX `LogCompilationParser` + `LogCompilationEnricher`. Correlates `compile_id`
(recording-scoped); extracts `task`, `task_done`, `make_not_entrant`, `make_not_compilable`,
`parse`, `inline_fail`, `uncommon_trap`. Upgrades `made_not_entrant`, `made_non_compilable`, and
missing inline reasons `INFERRED`/`UNKNOWN` -> `OBSERVED`. Pass `--log-compilation <xml>` /
`logCompilationPath`.

## Known limitations

- HotSpot-first; no OpenJ9/GraalVM/proprietary JVM modeling.
- Offline `.jfr` (+ optional `-XX:+LogCompilation`); **no live process attach**.
- `made_not_entrant`/`made_non_compilable` only `OBSERVED` with `LogCompilation`.
- Absence of `Inlined` frames != inlining failure; small/sparse recordings reduce confidence.
- Allocation: observed sample weight/frequency, not exact heap totals.
- `jdk.CPUTimeSample` experimental JDK 25+ Linux; separate; `failed`/`biased`/`CPUTimeSamplesLost`
  downgrade confidence; never merged; opt-in.
- **JDK 25 `jdk.CodeCacheStatistics` schema differs** (unified `codeBlobType`, `unallocatedCapacity`,
  `fullCount`, `startAddress`/`reservedTopAddress`) vs JDK 21 per-segment; read defensively; per-
  segment names not fabricated.
- **JDK 25 JEP 518 Cooperative Sampling** changes `jdk.ExecutionSample`; never compare raw counts;
  normalize; flag cross-JDK-major diffs. `jdk.Compilation` success field spelled `succeded` (sic);
  `jdk.CompilerInlining.callee` is a `CalleeMethod` struct; `jdk.CompilerInlining` spells `succeeded`.
- `jdk.Compilation#threshold` default `default.jfc`=1000 ms / `profile.jfc`=100 ms hides warmup.
- JFR parse issues recorded as warnings; never silently swallowed.
