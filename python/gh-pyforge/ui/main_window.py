from __future__ import annotations

import json
from concurrent.futures import Future
from typing import Any

import customtkinter as ctk

from core.config import Config
from core.task_queue import CancellationToken, TaskQueue
from engines import EngineRequest, EngineResponse
from engines.engine_audio import run as audio_engine
from engines.engine_image import run as image_engine
from engines.engine_llm import run as llm_engine
from engines.engine_speech import run as speech_engine
from engines.engine_video import run as video_engine
from models.registry import ModelRegistry
from ui.components.history_panel import HistoryPanel
from ui.components.model_selector import ModelSelector
from ui.components.output_viewer import OutputViewer
from ui.components.progress_bar import ProgressPanel
from ui.components.prompt_box import PromptBox
from ui.tabs.tab_audio import AudioTab
from ui.tabs.tab_image import ImageTab
from ui.tabs.tab_llm import LLMTab
from ui.tabs.tab_settings import SettingsTab
from ui.tabs.tab_speech import SpeechTab
from ui.tabs.tab_video import VideoTab


class MainWindow(ctk.CTk):
    """Main application window containing the PyForge user interface."""

    def __init__(self, config: Config) -> None:
        super().__init__()
        self.config = config
        self.task_queue = TaskQueue(max_workers=3)
        self.registry = ModelRegistry(config)
        self.current_future: Future[Any] | None = None
        self.cancel_token: CancellationToken | None = None
        self._progress_value = 0.0

        self._configure_grid()
        self._build_layout()
        self._ensure_default_models()

    def _configure_grid(self) -> None:
        self.grid_rowconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=0)
        self.grid_rowconfigure(2, weight=0)
        self.grid_columnconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=2)
        self.grid_columnconfigure(2, weight=1)

    def _build_layout(self) -> None:
        self.model_selector = ModelSelector(self, self.config)
        self.model_selector.grid(row=0, column=0, sticky="nsew", padx=12, pady=12)

        self.center_frame = ctk.CTkFrame(self)
        self.center_frame.grid(row=0, column=1, sticky="nsew", padx=12, pady=12)
        self.center_frame.grid_rowconfigure(0, weight=0)
        self.center_frame.grid_rowconfigure(1, weight=1)
        self.center_frame.grid_columnconfigure(0, weight=1)

        self.prompt_box = PromptBox(self.center_frame)
        self.prompt_box.grid(row=0, column=0, sticky="nsew", pady=(0, 12))

        self.tab_view = ctk.CTkTabview(self.center_frame, width=640)
        self.tab_view.grid(row=1, column=0, sticky="nsew")
        self._load_tabs()

        self.output_viewer = OutputViewer(self)
        self.output_viewer.grid(row=0, column=2, sticky="nsew", padx=12, pady=12)

        self.settings_panel = SettingsTab(self, self.config)
        self.settings_panel.grid(row=1, column=0, sticky="nsew", padx=12, pady=(0, 12))

        buttons_frame = ctk.CTkFrame(self)
        buttons_frame.grid(row=1, column=1, sticky="s", pady=(0, 12))
        buttons_frame.grid_columnconfigure(0, weight=1)

        self.generate_button = ctk.CTkButton(buttons_frame, text="Generate", command=self._on_generate, width=220)
        self.generate_button.grid(row=0, column=0, sticky="ew", pady=(0, 8))

        self.cancel_button = ctk.CTkButton(buttons_frame, text="Cancel", command=self._on_cancel, width=220)
        self.cancel_button.grid(row=1, column=0, sticky="ew")

        self.history_panel = HistoryPanel(self)
        self.history_panel.grid(row=1, column=2, sticky="nsew", padx=12, pady=(0, 12))

        self.progress_panel = ProgressPanel(self)
        self.progress_panel.grid(row=2, column=0, columnspan=3, sticky="ew", padx=12, pady=(0, 12))

    def _load_tabs(self) -> None:
        for name, tab_class in [
            ("Image", ImageTab),
            ("Video", VideoTab),
            ("Audio", AudioTab),
            ("LLM", LLMTab),
            ("Speech", SpeechTab),
            ("Settings", SettingsTab),
        ]:
            tab = tab_class(self.tab_view, self.config)
            self.tab_view.add(name)
            tab.place(in_=self.tab_view.tab(name), relx=0, rely=0, relwidth=1, relheight=1)

    def _ensure_default_models(self) -> None:
        missing = []
        for modality in self.config.default_models:
            default_model = self.registry.get_default_model(modality)
            if not (default_model.local_path / "model.json").exists():
                missing.append(modality)

        if not missing:
            return

        self._show_download_modal(missing)

    def _show_download_modal(self, modalities: list[str]) -> None:
        window = ctk.CTkToplevel(self)
        window.title("PyForge needs default models")
        window.geometry("540x320")
        window.grab_set()

        ctk.CTkLabel(
            window,
            text=(
                "PyForge needs to download default models before first launch.\n"
                "The download will be performed sequentially and stored locally."
            ),
            wraplength=500,
            justify="left",
        ).grid(row=0, column=0, sticky="nw", padx=16, pady=(16, 8))

        list_text = ctk.CTkTextbox(window, height=120)
        list_text.grid(row=1, column=0, sticky="nsew", padx=16, pady=(0, 12))
        list_text.insert("0.0", "\n".join(modalities))
        list_text.configure(state="disabled")

        self.download_progress = ctk.CTkProgressBar(window)
        self.download_progress.grid(row=2, column=0, sticky="ew", padx=16, pady=(0, 12))

        action_frame = ctk.CTkFrame(window)
        action_frame.grid(row=3, column=0, sticky="ew", padx=16, pady=(0, 16))
        action_frame.grid_columnconfigure((0, 1), weight=1)

        ctk.CTkButton(action_frame, text="Download", command=lambda: self._download_defaults(modalities, window)).grid(
            row=0, column=0, sticky="ew", padx=(0, 8)
        )
        ctk.CTkButton(action_frame, text="Exit", command=self.destroy).grid(row=0, column=1, sticky="ew", padx=(8, 0))

    def _download_defaults(self, modalities: list[str], window: ctk.CTkToplevel) -> None:
        self.download_progress.set(0.0)

        def progress(value: float) -> None:
            self.download_progress.set(value)

        for modality in modalities:
            model = self.registry.get_default_model(modality)
            self.registry.download_model(model, progress_cb=progress)

        window.destroy()
        self.model_selector._refresh_models()

    def _on_generate(self) -> None:
        prompt = self.prompt_box.get_prompt().strip()
        if not prompt:
            self.output_viewer.set_output("Enter a prompt before generation.")
            return

        modality = self.tab_view.get().lower()
        model = self.registry.get_active_model(modality)
        request = EngineRequest(prompt=prompt, model=model.name, modality=modality, parameters={})
        handler = self._get_engine_handler(modality)
        if handler is None:
            self.output_viewer.set_output(f"Unsupported modality: {modality}")
            return

        self.cancel_token = CancellationToken()
        self.current_future = self.task_queue.submit(handler, request, self.cancel_token)
        self._progress_value = 0.0
        self.progress_panel.set_progress(0.0)
        self.output_viewer.set_output(f"Starting {modality} task with model {model.name}...\n")
        self._poll_task()

    def _on_cancel(self) -> None:
        if self.cancel_token:
            self.cancel_token.cancel()
            self.output_viewer.set_output("Task cancellation requested.")

    def _poll_task(self) -> None:
        if not self.current_future:
            return
        if self.current_future.done():
            try:
                response = self.current_future.result()
                self._display_response(response)
            except Exception as error:
                self.output_viewer.set_output(f"Task failed: {error}")
            finally:
                self.current_future = None
                self.cancel_token = None
            return

        self._progress_value = min(self._progress_value + 0.05, 1.0)
        self.progress_panel.set_progress(self._progress_value)
        self.after(200, self._poll_task)

    def _display_response(self, response: EngineResponse) -> None:
        if response.success:
            output = response.output
            if isinstance(output, dict):
                display = json.dumps(output, indent=2)
            else:
                display = str(output)
            self.output_viewer.set_output(f"Success:\n{display}")
            self.history_panel.add_entry(
                prompt=self.prompt_box.get_prompt(),
                model=response.metadata.get("model", "unknown"),
                modality=response.metadata.get("modality", self.tab_view.get().lower()),
                preview=str(output),
                settings=response.metadata,
            )
        else:
            self.output_viewer.set_output(f"Error: {response.error}")

    def _get_engine_handler(self, modality: str):
        return {
            "image": image_engine,
            "video": video_engine,
            "audio": audio_engine,
            "speech": speech_engine,
            "llm": llm_engine,
        }.get(modality)

    def destroy(self) -> None:
        self.task_queue.shutdown(wait=False)
        super().destroy()
