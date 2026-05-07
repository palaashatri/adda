"""Example PCells shipped with aurora-eda.

Usage::

    from aurora.pdk.registry import register_pcell
    from aurora.examples.nmos_pcell import NmosPcell

    register_pcell("NMOS", NmosPcell)
"""

from aurora.examples.nmos_pcell import NmosPcell

__all__ = ["NmosPcell"]
