from __future__ import annotations

import threading
from concurrent.futures import Executor, Future, ThreadPoolExecutor
from queue import Queue
from typing import Any, Callable


class CancellationToken:
    """A token that can be used to cancel long-running engine tasks."""

    def __init__(self) -> None:
        self._cancelled = threading.Event()

    def cancel(self) -> None:
        self._cancelled.set()

    @property
    def is_cancelled(self) -> bool:
        return self._cancelled.is_set()

    def throw_if_cancelled(self) -> None:
        if self.is_cancelled:
            raise RuntimeError("Task cancelled")


ProgressCallback = Callable[[float], None]


class TaskQueue:
    """A thread-backed queue for non-blocking engine execution."""

    def __init__(self, max_workers: int = 4) -> None:
        self._executor: Executor = ThreadPoolExecutor(max_workers=max_workers)
        self._pending: Queue[Future[Any]] = Queue()

    def submit(
        self,
        work: Callable[..., Any],
        *args: Any,
        progress_callback: ProgressCallback | None = None,
    ) -> Future[Any]:
        future = self._executor.submit(work, *args)
        self._pending.put(future)
        return future

    def shutdown(self, wait: bool = True) -> None:
        self._executor.shutdown(wait=wait)

    def cancel(self, future: Future[Any]) -> None:
        future.cancel()
