"""Simulation helpers for aurora-eda.

These utilities write SPICE netlists and optionally invoke an external
SPICE-compatible simulator (ngspice by default).  They are pure-Python
wrappers that delegate heavy lifting to the aurora C++ core when the
compiled extension is available, or fall back to text-based SPICE
generation otherwise.
"""

from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional


@dataclass
class SimWaveform:
    """A single named waveform from a simulation run."""
    name: str
    x: List[float] = field(default_factory=list)  # time or frequency
    y: List[float] = field(default_factory=list)   # voltage, current, etc.


@dataclass
class SimResult:
    """Aggregated results from a simulator run."""
    success: bool = False
    error_message: str = ""
    raw_output: str = ""
    waveforms: List[SimWaveform] = field(default_factory=list)
    dc_operating_point: Dict[str, float] = field(default_factory=dict)


def _parse_dc_op(raw: str) -> Dict[str, float]:
    """Parse lines of the form '<name> = <value>' from simulator output."""
    result: Dict[str, float] = {}
    for line in raw.splitlines():
        eq = line.find("=")
        if eq < 0:
            continue
        name = line[:eq].strip()
        if not name:
            continue
        try:
            result[name] = float(line[eq + 1:].strip())
        except ValueError:
            pass
    return result


def run_spice(
    netlist: str,
    *,
    simulator: str = "ngspice",
    extra_control: str = "",
    workdir: Optional[Path] = None,
) -> SimResult:
    """Run a SPICE netlist string through an external simulator.

    Args:
        netlist:       Full SPICE netlist text (including .subckt/.ends or flat circuit).
        simulator:     Simulator executable name or path (default: ngspice).
        extra_control: Additional lines inserted into the .control block.
        workdir:       Directory for temporary files; uses a system temp dir if None.

    Returns:
        SimResult with success flag, raw output, and parsed operating point.
    """
    owns_workdir = workdir is None
    if owns_workdir:
        workdir = Path(tempfile.mkdtemp(prefix="aurora_sim_"))

    try:
        netlist_path = workdir / "aurora_netlist.spice"
        log_path = workdir / "aurora_sim.log"

        # Write netlist
        netlist_path.write_text(netlist, encoding="utf-8")

        # Check the simulator is available
        sim_exe = shutil.which(simulator)
        if sim_exe is None:
            return SimResult(
                success=False,
                error_message=f"Simulator '{simulator}' not found on PATH",
            )

        cmd = [sim_exe, "-b", "-o", str(log_path), str(netlist_path)]
        proc = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=120,
        )

        raw = proc.stdout + proc.stderr
        if log_path.exists():
            raw += "\n" + log_path.read_text(encoding="utf-8", errors="replace")

        dc_op = _parse_dc_op(raw)
        return SimResult(
            success=proc.returncode == 0,
            error_message="" if proc.returncode == 0 else f"Simulator exited {proc.returncode}",
            raw_output=raw,
            dc_operating_point=dc_op,
        )

    except subprocess.TimeoutExpired:
        return SimResult(success=False, error_message="Simulator timed out")
    except Exception as exc:  # noqa: BLE001
        return SimResult(success=False, error_message=str(exc))
    finally:
        if owns_workdir and workdir.exists():
            shutil.rmtree(workdir, ignore_errors=True)


def write_spice_header(cell_name: str, pins: List[str]) -> str:
    """Return a SPICE .subckt header line."""
    return f".subckt {cell_name} {' '.join(pins)}"


def write_spice_footer() -> str:
    return ".ends"
