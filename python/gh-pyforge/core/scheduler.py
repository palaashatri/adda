from __future__ import annotations

import threading
import time
from datetime import datetime, timedelta
from queue import Queue
from typing import Callable


class TaskScheduler:
    """A small scheduler for deferred background work."""

    def __init__(self) -> None:
        self._queue: Queue[tuple[datetime, Callable[[], None]]] = Queue()
        self._thread = threading.Thread(target=self._run_loop, daemon=True)
        self._stop_requested = threading.Event()
        self._thread.start()

    def schedule(self, delay_seconds: float, callback: Callable[[], None]) -> None:
        target = datetime.utcnow() + timedelta(seconds=delay_seconds)
        self._queue.put((target, callback))

    def shutdown(self) -> None:
        self._stop_requested.set()
        self._queue.put((datetime.utcnow(), lambda: None))
        self._thread.join(timeout=2)

    def _run_loop(self) -> None:
        while not self._stop_requested.is_set():
            try:
                target, callback = self._queue.get(timeout=0.5)
            except Exception:
                continue
            now = datetime.utcnow()
            if target > now:
                time.sleep((target - now).total_seconds())
            try:
                callback()
            except Exception:
                pass
