"""I9 — Schematic automation helpers."""

from dataclasses import dataclass, field
from typing import Dict, List, Tuple


@dataclass
class WireSeg:
    p1: Tuple[int, int]
    p2: Tuple[int, int]
    net: str = ""


@dataclass
class InstanceSpec:
    name: str
    master: str
    x: int
    y: int
    params: Dict[str, str] = field(default_factory=dict)


@dataclass
class SchematicBuilder:
    """Pure-Python schematic accumulator. Emits a netlist text on demand."""
    name: str
    wires: List[WireSeg] = field(default_factory=list)
    instances: List[InstanceSpec] = field(default_factory=list)
    nets: List[str] = field(default_factory=list)

    def add_wire(self, p1, p2, net=""):
        self.wires.append(WireSeg(p1, p2, net))
        if net and net not in self.nets:
            self.nets.append(net)

    def add_instance(self, name, master, x, y, **params):
        self.instances.append(InstanceSpec(name, master, x, y, params))

    def to_netlist(self) -> str:
        out = [f".SUBCKT {self.name} " + " ".join(self.nets)]
        for inst in self.instances:
            param_str = " ".join(f"{k}={v}" for k, v in inst.params.items())
            out.append(f"X{inst.name} {inst.master} {param_str}".rstrip())
        out.append(f".ENDS {self.name}")
        return "\n".join(out)
