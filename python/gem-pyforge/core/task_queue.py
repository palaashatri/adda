import threading
import queue
from typing import Callable, Any
from concurrent.futures import ThreadPoolExecutor
from core.logger import logger

class TaskQueue:
    """Handles async task execution using a ThreadPoolExecutor."""
    def __init__(self, max_workers: int = 2):
        self.executor = ThreadPoolExecutor(max_workers=max_workers)
        self.active_tasks = {}
        self.lock = threading.Lock()

    def submit_task(self, task_id: str, func: Callable, *args, **kwargs) -> Any:
        """Submits a task to the queue."""
        logger.info(f"Submitting task {task_id}")
        future = self.executor.submit(func, *args, **kwargs)
        with self.lock:
            self.active_tasks[task_id] = future
        return future

    def cancel_task(self, task_id: str):
        """Attempts to cancel a task."""
        with self.lock:
            future = self.active_tasks.get(task_id)
            if future:
                future.cancel()
                logger.info(f"Cancelled task {task_id}")
            else:
                logger.warning(f"Task {task_id} not found for cancellation.")

task_queue = TaskQueue()
