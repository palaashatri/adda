"""LLM tab: text generation."""
from __future__ import annotations

from typing import Optional

import customtkinter as ctk

from core.scheduler import EngineRequest
from core.task_queue import task_queue
from engines.engine_llm import engine_llm
from ui.components.history_panel import HistoryPanel, save_entry
from ui.components.model_selector import ModelSelector
from ui.components.output_viewer import OutputViewer
from ui.components.progress_bar import ProgressBar
from ui.components.prompt_box import PromptBox


class TabLLM(ctk.CTkFrame):
    """LLM prompt → text panel with token/temperature controls."""

    def __init__(self, master, **kwargs) -> None:
        super().__init__(master, **kwargs)
        self.grid_columnconfigure(0, weight=3)
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(2, weight=1)

        self.model_selector = ModelSelector(self, modality="llm")
        self.model_selector.grid(row=0, column=0, columnspan=2, sticky="ew", padx=10, pady=5)

        ctrl = ctk.CTkFrame(self)
        ctrl.grid(row=1, column=0, columnspan=2, sticky="ew", padx=10, pady=5)
        for c in range(6):
            ctrl.grid_columnconfigure(c, weight=1)
        ctk.CTkLabel(ctrl, text="Max tokens:").grid(row=0, column=0, padx=4, pady=4, sticky="e")
        self.max_tokens_entry = ctk.CTkEntry(ctrl, width=80)
        self.max_tokens_entry.insert(0, "128")
        self.max_tokens_entry.grid(row=0, column=1, padx=4, pady=4, sticky="w")
        ctk.CTkLabel(ctrl, text="Temp:").grid(row=0, column=2, padx=4, pady=4, sticky="e")
        self.temp_entry = ctk.CTkEntry(ctrl, width=80)
        self.temp_entry.insert(0, "0.8")
        self.temp_entry.grid(row=0, column=3, padx=4, pady=4, sticky="w")
        self.cancel_btn = ctk.CTkButton(
            ctrl, text="Cancel", fg_color="firebrick", command=self._cancel, state="disabled"
        )
        self.cancel_btn.grid(row=0, column=5, padx=4, pady=4, sticky="ew")

        left = ctk.CTkFrame(self, fg_color="transparent")
        left.grid(row=2, column=0, sticky="nsew", padx=10, pady=5)
        left.grid_rowconfigure(0, weight=1)
        left.grid_columnconfigure(0, weight=1)

        self.viewer = OutputViewer(left)
        self.viewer.grid(row=0, column=0, sticky="nsew", pady=(0, 10))

        self.progress = ProgressBar(left)
        self.progress.grid(row=1, column=0, sticky="ew", pady=(0, 10))
        self.progress.grid_remove()

        self.prompt_box = PromptBox(left, on_submit=self._generate)
        self.prompt_box.grid(row=2, column=0, sticky="ew")

        self.history = HistoryPanel(self, modality="llm")
        self.history.grid(row=2, column=1, sticky="nsew", padx=(0, 10), pady=5)

        self._current_task_id: Optional[str] = None
        self._current_token = None

    def _cancel(self) -> None:
        if self._current_token is not None:
            self._current_token.cancel()
        if self._current_task_id:
            task_queue.cancel_task(self._current_task_id)

    def _generate(self, prompt: str) -> None:
        if self.model_selector.combo.get() in ("", "No models found"):
            self.viewer.show_text("Error: no LLM installed. Use the Downloader tab.")
            return
        try:
            max_new_tokens = int(self.max_tokens_entry.get() or 128)
            temperature = float(self.temp_entry.get() or 0.8)
        except ValueError:
            max_new_tokens, temperature = 128, 0.8

        request = EngineRequest(
            prompt=prompt, model_id=self.model_selector.combo.get(), modality="llm",
            max_new_tokens=max_new_tokens, temperature=temperature,
        )
        self.viewer.show_text("Generating text…")
        self.progress.grid()
        self.progress.update_progress(0.1)
        self.cancel_btn.configure(state="normal")
        self._current_task_id = f"llm_gen_{id(request)}"
        future, token = task_queue.submit_task(self._current_task_id, self._run_engine, request)
        request.cancel_token = token
        self._current_token = token
        future.add_done_callback(lambda _f: self.master.after(0, self._reset_buttons))

    def _run_engine(self, request: EngineRequest) -> None:
        response = engine_llm.run(request)
        if response.success:
            self.master.after(0, self._on_success, response.output, request, response.metadata)
        else:
            self.master.after(0, self._on_error, response.error)

    def _on_success(self, text: str, request: EngineRequest, metadata: dict) -> None:
        self.progress.grid_remove()
        self.viewer.show_text(text)
        entry = save_entry(
            modality="llm",
            prompt=request.prompt,
            model_name=metadata.get("model", ""),
            settings=dict(request.kwargs),
            output_text=text,
        )
        self.history.add_history(entry)

    def _on_error(self, error: str) -> None:
        self.progress.grid_remove()
        self.viewer.show_text(f"Error: {error}")

    def _reset_buttons(self) -> None:
        self.cancel_btn.configure(state="disabled")
        self._current_task_id = None
        self._current_token = None
