"""Speech tab: ASR (file pick + transcribe) and TTS (text → speech) modes."""
from __future__ import annotations

from tkinter import filedialog
from typing import Optional

import customtkinter as ctk

from core.scheduler import EngineRequest
from core.task_queue import task_queue
from engines.engine_speech import engine_speech
from engines.engine_tts import engine_tts
from ui.components.history_panel import HistoryPanel, save_entry
from ui.components.model_selector import ModelSelector
from ui.components.output_viewer import OutputViewer
from ui.components.progress_bar import ProgressBar


class TabSpeech(ctk.CTkFrame):
    """Side-by-side ASR + TTS modes, gated by a top toggle."""

    def __init__(self, master, **kwargs) -> None:
        super().__init__(master, **kwargs)
        self.grid_columnconfigure(0, weight=3)
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(2, weight=1)

        self.model_selector = ModelSelector(self, modality="speech")
        self.model_selector.grid(row=0, column=0, columnspan=2, sticky="ew", padx=10, pady=5)

        mode_frame = ctk.CTkFrame(self)
        mode_frame.grid(row=1, column=0, columnspan=2, sticky="ew", padx=10, pady=5)
        ctk.CTkLabel(mode_frame, text="Mode:").grid(row=0, column=0, padx=4, pady=4)
        self.mode_var = ctk.StringVar(value="asr")
        ctk.CTkRadioButton(
            mode_frame, text="Speech → Text", variable=self.mode_var, value="asr",
            command=self._on_mode,
        ).grid(row=0, column=1, padx=4, pady=4)
        ctk.CTkRadioButton(
            mode_frame, text="Text → Speech", variable=self.mode_var, value="tts",
            command=self._on_mode,
        ).grid(row=0, column=2, padx=4, pady=4)

        left = ctk.CTkFrame(self, fg_color="transparent")
        left.grid(row=2, column=0, sticky="nsew", padx=10, pady=5)
        left.grid_rowconfigure(0, weight=1)
        left.grid_columnconfigure(0, weight=1)

        self.viewer = OutputViewer(left)
        self.viewer.grid(row=0, column=0, sticky="nsew", pady=(0, 10))

        self.progress = ProgressBar(left)
        self.progress.grid(row=1, column=0, sticky="ew", pady=(0, 10))
        self.progress.grid_remove()

        # ASR controls
        self.asr_frame = ctk.CTkFrame(left)
        self.asr_frame.grid(row=2, column=0, sticky="ew")
        self.asr_frame.grid_columnconfigure(0, weight=1)
        self.path_entry = ctk.CTkEntry(self.asr_frame, placeholder_text="Audio file…")
        self.path_entry.grid(row=0, column=0, padx=5, pady=5, sticky="ew")
        ctk.CTkButton(self.asr_frame, text="Browse…", command=self._browse).grid(
            row=0, column=1, padx=5, pady=5
        )
        ctk.CTkButton(self.asr_frame, text="Transcribe", command=self._run_asr).grid(
            row=0, column=2, padx=5, pady=5
        )

        # TTS controls
        self.tts_frame = ctk.CTkFrame(left)
        self.tts_frame.grid_columnconfigure(0, weight=1)
        self.tts_textbox = ctk.CTkTextbox(self.tts_frame, height=80)
        self.tts_textbox.grid(row=0, column=0, padx=5, pady=5, sticky="ew")
        ctk.CTkButton(self.tts_frame, text="Synthesize", command=self._run_tts).grid(
            row=0, column=1, padx=5, pady=5, sticky="e"
        )

        self.cancel_btn = ctk.CTkButton(
            left, text="Cancel", fg_color="firebrick", command=self._cancel, state="disabled"
        )
        self.cancel_btn.grid(row=3, column=0, sticky="e", pady=(4, 0))

        self.history = HistoryPanel(self, modality="speech")
        self.history.grid(row=2, column=1, sticky="nsew", padx=(0, 10), pady=5)

        self._current_task_id: Optional[str] = None
        self._current_token = None
        self._on_mode()

    # --- mode --------------------------------------------------------------

    def _on_mode(self) -> None:
        if self.mode_var.get() == "asr":
            self.tts_frame.grid_remove()
            self.asr_frame.grid()
            self.model_selector.modality = "speech"
        else:
            self.asr_frame.grid_remove()
            self.tts_frame.grid(row=2, column=0, sticky="ew")
            self.model_selector.modality = "audio"
        self.model_selector.refresh()

    def _browse(self) -> None:
        path = filedialog.askopenfilename(
            filetypes=[("Audio", "*.wav *.mp3 *.flac *.ogg *.m4a")]
        )
        if path:
            self.path_entry.delete(0, "end")
            self.path_entry.insert(0, path)

    def _cancel(self) -> None:
        if self._current_token is not None:
            self._current_token.cancel()
        if self._current_task_id:
            task_queue.cancel_task(self._current_task_id)

    # --- ASR ---------------------------------------------------------------

    def _run_asr(self) -> None:
        audio = self.path_entry.get().strip()
        if not audio:
            self.viewer.show_text("Pick an audio file first.")
            return
        if self.model_selector.combo.get() in ("", "No models found"):
            self.viewer.show_text("No speech model installed. Use the Downloader tab.")
            return
        request = EngineRequest(
            prompt=audio, model_id=self.model_selector.combo.get(), modality="speech"
        )
        self.viewer.show_text("Transcribing…")
        self.progress.grid()
        self.progress.update_progress(0.1)
        self.cancel_btn.configure(state="normal")
        self._current_task_id = f"speech_asr_{id(request)}"
        future, token = task_queue.submit_task(self._current_task_id, self._run_engine_asr, request)
        request.cancel_token = token
        self._current_token = token
        future.add_done_callback(lambda _f: self.master.after(0, self._reset_buttons))

    def _run_engine_asr(self, request: EngineRequest) -> None:
        response = engine_speech.run(request)
        if response.success:
            self.master.after(0, self._asr_success, response.output, request, response.metadata)
        else:
            self.master.after(0, self._on_error, response.error)

    def _asr_success(self, text: str, request: EngineRequest, metadata: dict) -> None:
        self.progress.grid_remove()
        self.viewer.show_text(text)
        entry = save_entry(
            modality="speech",
            prompt=request.prompt,
            model_name=metadata.get("model", ""),
            settings={"backend": metadata.get("backend", "")},
            output_text=text,
        )
        self.history.add_history(entry)

    # --- TTS ---------------------------------------------------------------

    def _run_tts(self) -> None:
        text = self.tts_textbox.get("1.0", "end-1c").strip()
        if not text:
            self.viewer.show_text("Type something to synthesize first.")
            return
        if self.model_selector.combo.get() in ("", "No models found"):
            self.viewer.show_text("No audio model installed. Use the Downloader tab.")
            return
        request = EngineRequest(prompt=text, model_id=self.model_selector.combo.get(), modality="audio")
        self.viewer.show_text("Synthesizing…")
        self.progress.grid()
        self.progress.update_progress(0.1)
        self.cancel_btn.configure(state="normal")
        self._current_task_id = f"speech_tts_{id(request)}"
        future, token = task_queue.submit_task(self._current_task_id, self._run_engine_tts, request)
        request.cancel_token = token
        self._current_token = token
        future.add_done_callback(lambda _f: self.master.after(0, self._reset_buttons))

    def _run_engine_tts(self, request: EngineRequest) -> None:
        response = engine_tts.run(request)
        if response.success:
            self.master.after(0, self._tts_success, response.output, request, response.metadata)
        else:
            self.master.after(0, self._on_error, response.error)

    def _tts_success(self, audio_path: str, request: EngineRequest, metadata: dict) -> None:
        self.progress.grid_remove()
        self.viewer.show_audio(audio_path)
        entry = save_entry(
            modality="speech",
            prompt=request.prompt,
            model_name=metadata.get("model", ""),
            settings={"backend": "tts"},
            output_path=audio_path,
        )
        self.history.add_history(entry)

    # --- common ------------------------------------------------------------

    def _on_error(self, error: str) -> None:
        self.progress.grid_remove()
        self.viewer.show_text(f"Error: {error}")

    def _reset_buttons(self) -> None:
        self.cancel_btn.configure(state="disabled")
        self._current_task_id = None
        self._current_token = None
