"""I5/I6 — Custom DRC rules and simulation analyses written in Python."""

from dataclasses import dataclass, field
from typing import Callable, Dict, List


@dataclass
class DrcRule:
    name: str
    fn: Callable  # takes (view, lib, tech) -> list[str] violations
    layer: str = ""


@dataclass
class SimAnalysis:
    name: str
    fn: Callable  # takes (runner, opts) -> result dict


_drc_rules: List[DrcRule] = []
_analyses: Dict[str, SimAnalysis] = {}


def register_drc_rule(rule: DrcRule):
    _drc_rules.append(rule)


def list_drc_rules() -> List[DrcRule]:
    return list(_drc_rules)


def register_analysis(analysis: SimAnalysis):
    _analyses[analysis.name] = analysis


def get_analysis(name: str) -> SimAnalysis:
    return _analyses[name]


def list_analyses() -> List[SimAnalysis]:
    return list(_analyses.values())
