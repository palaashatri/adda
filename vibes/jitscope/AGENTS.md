


# AGENTS.md — JITScope

## 1. Mission & Product Identity
Build **JITScope**, an open-source (Apache-2.0), local-first, read-only HotSpot JVM JIT diagnostics engine, CLI, and MCP server.

JITScope answers investigative compiler performance questions from JVM recordings and artifacts—**without requiring an LLM to produce the diagnosis**:
- Which hot methods failed to reach or retain optimal compilation?
- What is the per-method compilation, inlining, and deoptimization lifecycle?
- Is a deoptimization storm, inlining cliff, or code-cache stall actively degrading execution?
- What JIT behavior regressed between two recordings (e.g., code change, JVM flags, or JDK 21 vs. JDK 25)?

### Hard Non-Goals & Boundaries
- **Engine-first, MCP-last:** All diagnostic logic lives in `jitscope-core`. The CLI (`jitscope-cli`) and MCP server (`jitscope-mcp`) must be thin adapters with zero diagnostic business logic.
- **HotSpot-first (no premature abstraction):** Model OpenJDK HotSpot JFR and compiler semantics accurately in v1. Do NOT invent a speculative cross-JVM abstraction layer for OpenJ9, GraalVM, or proprietary JVMs.
- **Local-first & zero network egress:** Never send recording data, class names, or method names over the network.
- **Read-only & offline-first:** v1 analyzes offline `.jfr` files (and optional `-XX:+LogCompilation` XML logs). Do NOT implement live process attach (`jcmd`/Attach API) in v1.
- **Clean-room open source:** Use only public OpenJDK builds, public JFR APIs (`jdk.jfr.consumer`), and synthetic/public workloads. Include no proprietary or vendor-specific references.

---

## 2. Technology Stack & Repository Layout

### Stack
- **Language:** Java 21 (`--release 21` bytecode target; must run cleanly on both JDK 21 LTS and JDK 25 LTS).
- **Build:** Multi-module Apache Maven (`3.9+`).
- **JFR Parsing:** Built-in `jdk.jfr.consumer` (zero external JFR parsing libraries).
- **CLI:** `info.picocli:picocli:4.7.7`.
- **MCP Server:** Official Java MCP SDK via BOM `io.modelcontextprotocol.sdk:mcp-bom:2.2.0` (`io.modelcontextprotocol.sdk:mcp` using STDIO transport; note `mcp` bundles Jackson 3.x via `mcp-json-jackson3`—align JSON serialization across modules or use `mcp-core` + `mcp-json-jackson2` consistently so there are no classpath conflicts).
- **Testing:** JUnit 5 (or JUnit Jupiter 5.11+/6.x) + AssertJ.
- **License:** Apache-2.0.

### Multi-Module Maven Layout
```text
jitscope/
├── AGENTS.md
├── LICENSE
├── README.md
├── pom.xml                          # Parent POM (dependencyManagement, Java 21, surefire/failsafe)
├── jitscope-core/                   # Pure diagnostic engine (ZERO CLI or MCP dependencies)
│   └── src/main/java/dev/jitscope/core/
│       ├── model/                   # Immutable Java records (MethodKey, Compilation, Lifecycle, etc.)
│       ├── ingest/                  # JfrLoader, ConfigAuditor, LogCompilationParser
│       ├── analysis/                # LifecycleBuilder, PathologyDetectors, DiffEngine
│       ├── report/                  # MarkdownRenderer, JsonRenderer
│       └── profile/                 # Bundled jit-diag.jfc template resource
├── jitscope-cli/                    # Picocli CLI + shaded runnable JAR
│   └── src/main/java/dev/jitscope/cli/
├── jitscope-mcp/                    # Thin STDIO MCP server delegating to jitscope-core
│   └── src/main/java/dev/jitscope/mcp/
└── jitscope-workloads/              # Deterministic pathological workloads for tests & demos
    └── src/main/java/dev/jitscope/workloads/
        ├── DeoptStorm.java
        ├── InlineCliff.java
        └── CodeCacheCliff.java

```

---

## 3. Mandatory HotSpot JFR & Diagnostic Invariants

Read and obey every item below—violating any of these will produce broken parsing or misleading diagnostics:

1. **Field-Name Spelling Quirk (`jdk.Compilation`):**
* On `jdk.Compilation`, the boolean success field is spelled **`succeded`** (one `e`), whereas `jdk.CompilerInlining` spells it **`succeeded`**. Always check `event.hasField("succeded") ? event.getBoolean("succeded") : event.getBoolean("succeeded")`.


2. **`jdk.CompilerInlining.callee` Type:**
* `caller` is a `RecordedMethod`, while `callee` is a `RecordedObject` (`CalleeMethod` struct with `type`, `name`, `descriptor`), NOT a `RecordedMethod`. Extract `callee` fields defensively via `RecordedObject`.


3. **Preserve Relational Keys on `jdk.Deoptimization`:**
* Capture `compileId`, `compiler`, `bci`, `lineNumber`, `reason`, `action`, and `method` (when present via `hasField`).
* Scope `compileId` to a `recordingId` (never treat `compileId` as globally unique across files).
* Join `jdk.Deoptimization`, `jdk.CompilerInlining`, `jdk.CompilerPhase`, and `jdk.CompilationFailure` to `jdk.Compilation` via `(recordingId, compileId)`.


4. **Use `RecordedFrame.getType()` Directly:**
* Every `RecordedFrame` in a sampled stack trace reports its execution type (`"Interpreted"`, `"JIT compiled"`, `"Inlined"`, `"Native"`).
* **Statistical honesty rule:** Seeing `"Inlined"` frames is *observed* proof a method was inlined on that call path. Absence of `"Inlined"` frames is NOT proof of inlining failure—corroborate with `jdk.CompilerInlining` or `LogCompilation` before claiming an inlining failure.
* Distinguish **self-frame** (top frame, index 0) from **stack-frame** (depth 1..N) counts so callers and inlined callees are attributed accurately.


5. **Sampling Semantics & JDK 21 vs. JDK 25 Hygiene:**
* Keep `jdk.ExecutionSample` and `jdk.CPUTimeSample` (JDK 25+ Linux experimental, JEP 509) in **strictly separate datasets**. Never merge their counts into one metric.
* For `jdk.CPUTimeSample`, inspect `failed` (`null` stack trace), `biased`, and `jdk.CPUTimeSamplesLost` (`lostSamples`). Downgrade confidence when sample loss or failures are non-zero.
* Because JDK 25 changed `jdk.ExecutionSample` via JEP 518 (Cooperative Sampling), **never compare raw sample counts across recordings**. Always normalize to relative sample share (`methodSamples / totalValidSamples`) and flag cross-JDK major-version diffs in the report metadata.
* For allocations, support `jdk.ObjectAllocationSample` as primary (fall back to `jdk.ObjectAllocationInNewTLAB` / `jdk.ObjectAllocationOutsideTLAB` if present) and report observed sample weight/frequency rather than claiming exact heap byte totals.


6. **Recording Configuration Audit:**
* Inspect `jdk.ActiveSetting` events on load. Flag when:
* `jdk.Compilation#threshold` is > `0 ms` (warn explicitly about `default.jfc` = 1000 ms and `profile.jfc` = 100 ms hiding warmup).
* `jdk.CompilerInlining`, `jdk.CompilationFailure`, `jdk.Deoptimization`, `jdk.CodeCacheFull`, or `jdk.ExecutionSample` are disabled or missing.


* Always emit the exact `-XX:StartFlightRecording=...` or `jit-diag.jfc` remediation command when signals are missing.


7. **Confidence-Aware Provenance (No Fabricated Certainty):**
* Every lifecycle transition and every `PathologyFinding` must carry:
* `EvidenceLevel`: `OBSERVED` | `INFERRED` | `UNKNOWN`
* Structured `Evidence` items (event types, `compileId`s, timestamps, sample shares, threshold values).


* Never claim a method was `made_not_entrant` or `made_non_compilable` as `OBSERVED` from JFR alone—mark as `INFERRED` unless backed by a `LogCompilation` XML event.


8. **MCP STDIO Discipline:**
* In `jitscope-mcp` STDIO mode, `System.out` is strictly reserved for JSON-RPC protocol frames. Route all logging and diagnostics to `System.err`.



---

## 4. Core Domain Model & Diagnostic Engine (`jitscope-core`)

### Key Immutable Records (`dev.jitscope.core.model`)

* `MethodKey(String className, String methodName, String descriptor)` (with human-readable formatter `fqn()`).
* `RecordingMetadata(String recordingId, Path sourcePath, String jvmVersion, String jvmArguments, Instant startTime, Instant endTime, Duration duration, ConfigAudit configAudit)`.
* `CompilationRecord(int compileId, MethodKey method, int compileLevel, boolean osr, boolean succeeded, Instant startTime, Duration duration, int codeSizeBytes, int inlinedBytes, String failureMessage)`.
* `DeoptimizationRecord(int compileId, MethodKey method, int bci, int lineNumber, String reason, String action, String compiler, Instant timestamp)`.
* `InliningRecord(int compileId, MethodKey caller, MethodKey callee, int bci, boolean succeeded, String message, int bytes)`.
* `FrameStats(long totalSamples, long selfSamples, Map<FrameType, Long> byFrameType, double selfShare, double totalShare)`.
* `MethodLifecycle(MethodKey method, List<LifecycleEvent> timeline, int highestCompileLevel, int compilationCount, int deoptimizationCount, FrameStats executionSamples, Optional<FrameStats> cpuTimeSamples, AllocationStats allocationStats, EvidenceLevel lifecycleConfidence)`.
* `PathologyFinding(String id, PathologyType type, Severity severity, EvidenceLevel evidenceLevel, MethodKey primaryMethod, String title, String explanation, List<String> evidence, String recommendation)`.
* `DiffFinding(MethodKey method, DiffCategory category, Severity severity, EvidenceLevel evidenceLevel, String baselineSummary, String candidateSummary, double sampleShareDelta, String explanation)`.

### Required Diagnostic Analyzers (`dev.jitscope.core.analysis`)

1. **ConfigAuditor:** Parses `jdk.ActiveSetting` and event counts; scores recording completeness (`COMPLETE`, `PARTIAL`, `INSUFFICIENT`) and outputs actionable capture warnings.
2. **LifecycleBuilder:** Joins `Compilation`, `CompilationFailure`, `CompilerInlining`, and `Deoptimization` by `compileId`; overlays `ExecutionSample`, `CPUTimeSample` (separate), and `ObjectAllocationSample` per `MethodKey`.
3. **HotUnderoptimizedDetector:** Flags methods with high execution sample share (configurable threshold, default >= 2% self or >= 5% total) that remain predominantly `"Interpreted"` or fail to reach Tier 4 (`compileLevel < 4`) after warmup.
4. **DeoptStormDetector:** Groups deoptimizations by `(method, reason, action)`, flags repeated deopt-recompile cycles (>= 3 deopts on the same method or deopt storms across the JVM), joins each deopt to its `compileId` tier, and checks whether `"Interpreted"` frame samples increased in the window immediately following the deopt (`CORRELATED` / `INFERRED` runtime impact).
5. **InliningAllocationDetector:** Identifies hot methods with failed `jdk.CompilerInlining` decisions (e.g., `callee is too large`, `hot method too big`, `already compiled into a big method`, megamorphic/virtual dispatch) and correlates them with elevated `jdk.ObjectAllocationSample` frequency (indicating potential missed Escape Analysis / scalar replacement).
6. **CodeCachePressureDetector:** Tracks `jdk.CodeCacheConfiguration`, `jdk.CodeCacheStatistics` (per segment: `non-profiled nmethods`, `profiled nmethods`, `non-nmethods`), and `jdk.CodeCacheFull`; correlates high occupancy (> 85%) or `CodeCacheFull` events with subsequent compilation failures or stalled compilations.
7. **DiffEngine:** Compares `baseline` vs. `candidate` `RecordingAnalysis`. Detects:
* Tier regression (e.g., Tier 4 -> Tier 3/1/0).
* Inlining regression (callee frame type shifted from `"Inlined"` -> `"JIT compiled"` or new `CompilerInlining` failures).
* Deoptimization regression (significant increase in deopt count/new reasons).
* Hotness shift (normalized relative sample-share delta).
* Explicitly annotates if baseline and candidate differ in JVM major version or sampling configuration.


8. **LogCompilationEnricher (Phase 5):** Optional streaming StAX parser for `-XX:+LogCompilation` XML (`<task>`, `<task_done>`, `<make_not_entrant>`, `<parse>`, `<inline_fail>`, `<uncommon_trap>`). Upgrades lifecycle transitions (`made_not_entrant`) and missing inline reasons from `INFERRED`/`UNKNOWN` to `OBSERVED`.

---

## 5. CLI & MCP Contracts

### CLI (`jitscope-cli`, runnable via `java -jar jitscope-cli.jar`)

* `jitscope analyze <file.jfr> [--log-compilation <xml>] [--format md|json] [--top 20]`
* Prints recording audit, top hot methods with frame-type breakdown, and prioritized pathology findings with evidence.


* `jitscope method <file.jfr> <pattern> [--log-compilation <xml>] [--format md|json]`
* Full per-method compilation lifecycle, compile/deopt/inlining tables, and frame-type distribution for matching methods.


* `jitscope diff <baseline.jfr> <candidate.jfr> [--format md|json] [--min-delta 0.01]`
* Structured regression report highlighting tier drops, inlining regressions, deopt spikes, and normalized sample-share shifts.


* `jitscope config-check <file.jfr> [--format md|json]`
* Audits JFR settings and reports missing events/thresholds.


* `jitscope init-jfc [--out jit-diag.jfc] [--include-cpu-time]`
* Writes a curated `jit-diag.jfc` (0 ms compilation threshold, inlining/failure/deopt/codecache/execution/allocation enabled; optional experimental `jdk.CPUTimeSample` toggle for JDK 25+ Linux).


* `jitscope mcp`
* Launches the local STDIO MCP server (delegates to `jitscope-mcp`).



### MCP Server (`jitscope-mcp`, STDIO transport)

Expose exactly five read-only tools (each returning structured JSON matching `jitscope-core` DTOs plus a concise Markdown summary):

1. `check_recording_config(jfrPath)`
2. `analyze_recording(jfrPath, logCompilationPath?, topN?)`
3. `inspect_method(jfrPath, methodPattern, logCompilationPath?)`
4. `find_pathologies(jfrPath, types?, minSeverity?, logCompilationPath?)`
5. `diff_recordings(baselineJfrPath, candidateJfrPath, minSampleShareDelta?)`

Reject nonexistent paths cleanly with structured MCP tool errors; never accept arbitrary shell commands or JVM flags from MCP tool inputs.

---

## 6. Phased Execution Plan & Verification Gates

Execute phases strictly in order. **Do not advance to the next phase until all tests and verification gates for the current phase pass.**

### Phase 0: Environment Probe & Empirical Workload Spike

1. Check available JDKs (`java -version`, `javac -version`, `mvn -version`, and any secondary JDK 21/25 paths on the machine).
2. Scaffold the multi-module Maven build (`pom.xml` + 4 modules) and Apache-2.0 `LICENSE`.
3. Implement the three zero-dependency deterministic workloads in `jitscope-workloads`:
* `DeoptStorm`: Warm a monomorphic interface call site to Tier 4 (~200k iterations, writing to a `public static volatile long sink`), then swap in 3 additional implementations and flip a cold guard flag to trigger repeated `class_check` / `unstable_if` deopts and recompilations.
* `InlineCliff`: Run two comparable modes (`--mode=inlineable` vs `--mode=cliff`): allocate short-lived `record Pair(long a, long b)` inside a hot helper; in `cliff` mode, force a megamorphic call site or oversized callee so inlining fails and `jdk.ObjectAllocationSample` spikes.
* `CodeCacheCliff`: Designed to run with `-XX:ReservedCodeCacheSize=10m` (tune size dynamically if needed on the host JDK), dynamically loading/invoking copies of a worker class via custom `ClassLoader`s retained in a `List` until `jdk.CodeCacheFull` or code-cache pressure fires.


4. Create `jitscope-core/src/main/resources/jfc/jit-diag.jfc`.
5. **Verification Gate 0:** Run all three workloads with `-XX:StartFlightRecording=settings=...,filename=...` on the local JDK. Verify via `jfr summary` / a quick probe test that:
* `DeoptStorm` emits `jdk.Compilation` (level 4) + `jdk.Deoptimization` events.
* `InlineCliff` emits `jdk.CompilerInlining` failures + `jdk.ObjectAllocationSample` + `"Inlined"` vs `"JIT compiled"` frame differences.
* `CodeCacheCliff` emits `jdk.CodeCacheStatistics` pressure / `jdk.CodeCacheFull`.
* Tune workload iteration counts/flags until all three trigger reliably in under 10 seconds each.



### Phase 1: `jitscope-core` Ingestion, Config Audit & Lifecycle Builder

1. Implement `model/*`, `JfrLoader`, `ConfigAuditor`, and `LifecycleBuilder`.
2. Handle all Section 3 invariants (`succeded` typo, `callee` struct, separate `ExecutionSample` vs `CPUTimeSample`, `compileId` scoping, `RecordedFrame.getType()`).
3. **Verification Gate 1:** Unit + integration tests generating real `.jfr` files from `jitscope-workloads` (plus a recording captured with default JFR settings to verify `ConfigAuditor` flags `INSUFFICIENT`). Run `mvn test`.

### Phase 2: Pathology Detectors & Regression Diff Engine

1. Implement `HotUnderoptimizedDetector`, `DeoptStormDetector`, `InliningAllocationDetector`, `CodeCachePressureDetector`, and `DiffEngine`.
2. Ensure every finding includes `EvidenceLevel`, raw evidence lines, and concrete recommendations.
3. **Verification Gate 2:** Integration tests in `jitscope-core` must run real workloads to produce `.jfr` recordings and assert:
* `DeoptStorm` recording triggers `DEOPT_STORM` finding linked to the exact target method.
* `InlineCliff` (`cliff` mode) triggers `INLINING_ALLOCATION_CLIFF` finding.
* `InlineCliff` (`inlineable` vs `cliff`, or default vs `-XX:TieredStopAtLevel=1`) passed to `DiffEngine` detects the exact regressed method with tier/inlining/sample-share evidence.
* `CodeCacheCliff` recording triggers `CODE_CACHE_PRESSURE` finding.
* Run `mvn verify`.



### Phase 3: `jitscope-cli` & Report Renderers

1. Implement `MarkdownRenderer` and `JsonRenderer` in `jitscope-core`.
2. Implement Picocli commands (`analyze`, `method`, `diff`, `config-check`, `init-jfc`, `mcp`) in `jitscope-cli`.
3. Configure Maven Shade plugin to produce a single runnable fat JAR (`jitscope-cli/target/jitscope.jar`).
4. **Verification Gate 3:** Execute CLI commands end-to-end against real workload `.jfr` files, verify exit codes, validate JSON output parses cleanly, and verify Markdown reports are readable and self-contained. Run `mvn verify`.

### Phase 4: `jitscope-mcp` STDIO Server

1. Wire `io.modelcontextprotocol.sdk:mcp` (STDIO transport) registering the 5 tools in Section 5.
2. Verify zero stray stdout output before/during JSON-RPC messaging.
3. **Verification Gate 4:** Write an automated process-level integration test that spawns `java -jar jitscope.jar mcp` (or invokes the STDIO transport), sends real MCP JSON-RPC requests (`initialize`, `notifications/initialized`, `tools/list`, and `tools/call` for `check_recording_config`, `analyze_recording`, `inspect_method`, `find_pathologies`, `diff_recordings` using real workload `.jfr` files), and asserts valid protocol framing and accurate findings. Run `mvn verify`.

### Phase 5: Optional `LogCompilation` Enricher, Docs & CI

1. Implement `LogCompilationParser` (StAX streaming XML) in `jitscope-core` to ingest `-XX:+UnlockDiagnosticVMOptions -XX:+LogCompilation` logs, correlate `compile_id`, extract `make_not_entrant` and `inline_fail` entries, and upgrade matching findings/lifecycle events to `OBSERVED`.
2. Add an integration test capturing both `.jfr` and `hotspot_pid*.log` from `DeoptStorm` / `InlineCliff` and verifying enrichment.
3. Write a concise, high-signal `README.md` (quick start, sample real output from the bundled workloads, MCP client config snippets for Claude Code / Codex / OpenCode, and known limitations).
4. Add `.github/workflows/ci.yml` running `mvn clean verify` on a matrix of Temurin JDK 21 and JDK 25 on `ubuntu-latest`.

---

## 7. Coding & Quality Standards

* Prefer immutable Java `record`s, sealed interfaces, and `Optional` over nullable mutable POJOs.
* Keep methods small and testable; no Spring Boot, no database, no reflection magic.
* Never silently swallow JFR parsing exceptions—record malformed/missing fields in recording warnings so diagnostics stay transparent.
* Integration tests must generate real `.jfr` recordings cleanly in `@TempDir` (never commit large binary `.jfr` blobs to Git).
* Do not declare completion until `mvn clean verify` succeeds with 0 skipped tests and all 5 CLI commands + all 5 MCP tools are verified against real recordings.

```

### Verified Dependency & Runtime Notes
* **Java MCP SDK:** `io.modelcontextprotocol.sdk:mcp-bom` and `io.modelcontextprotocol.sdk:mcp` are at **`2.2.0`** (released November 14, 2026). The convenience `mcp` artifact bundles `mcp-core` with Jackson 3.x (`mcp-json-jackson3`), while `mcp-json-jackson2` is available if you prefer Jackson 2.x.
* **Picocli:** Latest stable release is **`4.7.7`**.
* **JDK 25 LTS JFR Behavior:** JDK 25 includes JEP 509 (experimental Linux `jdk.CPUTimeSample`), JEP 518 (Cooperative Sampling), and JEP 520 (`jdk.MethodTiming` / `jdk.MethodTrace`), all of which are accounted for in the sampling invariants and CI matrix above.

To kick off an autonomous run with your CLI agent, start with:
> `Read AGENTS.md completely. Execute Phase 0 first, show me the real event counts captured from the three workloads on my local JDK, and then proceed phase by phase through Phase 5 without skipping verification gates.`

```
