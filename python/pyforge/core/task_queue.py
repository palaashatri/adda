"""Threaded task queue with real cancellation, ETA tracking, and priority."""
from __future__ import annotations

import os
import threading
import time
from concurrent.futures import Future, ThreadPoolExecutor
from dataclasses import dataclass
from typing import Any, Callable, Optional

from core.logger import logger
from core.scheduler import CancellationToken


@dataclass
class _TaskRecord:
    future: Future
    token: CancellationToken
    submitted_at: float
    started_at: Optional[float] = None
    finished_at: Optional[float] = None


def _default_max_workers() -> int:
    """Dynamic worker count: cpu_count() capped at 4 to avoid thrashing."""
    return max(1, min(4, os.cpu_count() or 2))


class TaskQueue:
    """ThreadPoolExecutor wrapper with cancellation tokens + ETA tracking."""

    def __init__(self, max_workers: Optional[int] = None) -> None:
        workers = max_workers if max_workers is not None else _default_max_workers()
        self.executor = ThreadPoolExecutor(max_workers=workers)
        self._tasks: dict[str, _TaskRecord] = {}
        self._lock = threading.Lock()
        self._avg_duration: dict[str, float] = {}

    def submit_task(
        self,
        task_id: str,
        func: Callable[..., Any],
        *args: Any,
        cancel_token: Optional[CancellationToken] = None,
        **kwargs: Any,
    ) -> tuple[Future, CancellationToken]:
        """Submit `func(*args, **kwargs)`. Returns (future, cancel_token)."""
        token = cancel_token or CancellationToken()
        logger.info(f"Submitting task {task_id}")

        def runner() -> Any:
            with self._lock:
                rec = self._tasks.get(task_id)
                if rec is not None:
                    rec.started_at = time.time()
            try:
                return func(*args, **kwargs)
            finally:
                with self._lock:
                    rec = self._tasks.get(task_id)
                    if rec is not None:
                        rec.finished_at = time.time()
                        if rec.started_at is not None:
                            duration = rec.finished_at - rec.started_at
                            kind = task_id.split("_")[0]
                            prev = self._avg_duration.get(kind)
                            self._avg_duration[kind] = (
                                duration if prev is None else (prev * 0.7 + duration * 0.3)
                            )

        future = self.executor.submit(runner)
        with self._lock:
            self._tasks[task_id] = _TaskRecord(
                future=future, token=token, submitted_at=time.time()
            )
        return future, token

    def cancel_task(self, task_id: str) -> bool:
        """Request cancellation. Returns True if a task was found."""
        with self._lock:
            rec = self._tasks.get(task_id)
        if rec is None:
            logger.warning(f"Task {task_id} not found for cancellation.")
            return False
        rec.token.cancel()
        rec.future.cancel()
        logger.info(f"Cancellation requested for {task_id}")
        return True

    def estimate_eta(self, kind: str) -> Optional[float]:
        """Return an EWMA of recent durations for tasks of this kind."""
        return self._avg_duration.get(kind)


task_queue = TaskQueue()
