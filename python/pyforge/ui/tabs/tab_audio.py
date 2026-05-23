"""Audio (TTS) tab."""
from __future__ import annotations

from typing import Optional

import customtkinter as ctk

from core.scheduler import EngineRequest
from core.task_queue import task_queue
from engines.engine_audio import engine_audio
from ui.components.history_panel import HistoryPanel, save_entry
from ui.components.model_selector import ModelSelector
from ui.components.output_viewer import OutputViewer
from ui.components.progress_bar import ProgressBar
from ui.components.prompt_box import PromptBox


class TabAudio(ctk.CTkFrame):
    """Text-to-audio generation tab."""

    def __init__(self, master, **kwargs) -> None:
        super().__init__(master, **kwargs)
        self.grid_columnconfigure(0, weight=3)
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(1, weight=1)

        self.model_selector = ModelSelector(self, modality="audio")
        self.model_selector.grid(row=0, column=0, columnspan=2, sticky="ew", padx=10, pady=5)

        left = ctk.CTkFrame(self, fg_color="transparent")
        left.grid(row=1, column=0, sticky="nsew", padx=10, pady=5)
        left.grid_rowconfigure(0, weight=1)
        left.grid_columnconfigure(0, weight=1)

        self.viewer = OutputViewer(left)
        self.viewer.grid(row=0, column=0, sticky="nsew", pady=(0, 10))

        self.progress = ProgressBar(left)
        self.progress.grid(row=1, column=0, sticky="ew", pady=(0, 10))
        self.progress.grid_remove()

        self.prompt_box = PromptBox(left, on_submit=self._generate)
        self.prompt_box.grid(row=2, column=0, sticky="ew")

        cancel = ctk.CTkButton(
            left, text="Cancel", fg_color="firebrick", command=self._cancel, state="disabled"
        )
        cancel.grid(row=3, column=0, sticky="e", pady=(4, 0))
        self.cancel_btn = cancel

        self.history = HistoryPanel(self, modality="audio")
        self.history.grid(row=1, column=1, sticky="nsew", padx=(0, 10), pady=5)

        self._current_task_id: Optional[str] = None
        self._current_token = None

    def _cancel(self) -> None:
        if self._current_token is not None:
            self._current_token.cancel()
        if self._current_task_id:
            task_queue.cancel_task(self._current_task_id)

    def _generate(self, prompt: str) -> None:
        if self.model_selector.combo.get() in ("", "No models found"):
            self.viewer.show_text("Error: no audio model installed. Use the Downloader tab.")
            return

        request = EngineRequest(prompt=prompt, model_id=self.model_selector.combo.get(), modality="audio")
        self.viewer.show_text("Generating audio…")
        self.progress.grid()
        self.progress.update_progress(0.05)
        self.cancel_btn.configure(state="normal")
        self._current_task_id = f"audio_gen_{id(request)}"
        future, token = task_queue.submit_task(self._current_task_id, self._run_engine, request)
        request.cancel_token = token
        self._current_token = token
        future.add_done_callback(lambda _f: self.master.after(0, self._reset_buttons))

    def _run_engine(self, request: EngineRequest) -> None:
        response = engine_audio.run(request)
        if response.success:
            self.master.after(0, self._on_success, response.output, request, response.metadata)
        else:
            self.master.after(0, self._on_error, response.error)

    def _on_success(self, output_path: str, request: EngineRequest, metadata: dict) -> None:
        self.progress.grid_remove()
        self.viewer.show_audio(output_path)
        entry = save_entry(
            modality="audio",
            prompt=request.prompt,
            model_name=metadata.get("model", ""),
            settings=dict(request.kwargs),
            output_path=output_path,
        )
        self.history.add_history(entry)

    def _on_error(self, error: str) -> None:
        self.progress.grid_remove()
        self.viewer.show_text(f"Error: {error}")

    def _reset_buttons(self) -> None:
        self.cancel_btn.configure(state="disabled")
        self._current_task_id = None
        self._current_token = None
