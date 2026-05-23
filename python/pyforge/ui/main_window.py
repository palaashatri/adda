"""Main application window: 7-tab notebook with optional first-run banner."""
from __future__ import annotations

import customtkinter as ctk

from core.config import APP_TITLE, DEFAULT_MODELS
from models.registry import registry
from ui.tabs.tab_audio import TabAudio
from ui.tabs.tab_history import TabHistory
from ui.tabs.tab_image import TabImage
from ui.tabs.tab_llm import TabLLM
from ui.tabs.tab_settings import TabSettings
from ui.tabs.tab_speech import TabSpeech
from ui.tabs.tab_video import TabVideo


TAB_NAMES = ("Image", "Video", "Audio", "Speech", "LLM", "Downloader", "History")


class MainWindow(ctk.CTk):
    """Resizable dark-mode window with all PyForge tabs."""

    def __init__(self) -> None:
        super().__init__()
        self.title(APP_TITLE)
        self.geometry("1100x780")
        self.minsize(900, 640)

        self.grid_rowconfigure(1, weight=1)
        self.grid_columnconfigure(0, weight=1)

        self._banner: ctk.CTkFrame | None = None
        self._maybe_show_first_run_banner()

        self.tabview = ctk.CTkTabview(self)
        self.tabview.grid(row=1, column=0, sticky="nsew", padx=10, pady=10)
        for name in TAB_NAMES:
            self.tabview.add(name)
            self.tabview.tab(name).grid_rowconfigure(0, weight=1)
            self.tabview.tab(name).grid_columnconfigure(0, weight=1)

        self.tab_image = TabImage(self.tabview.tab("Image"))
        self.tab_image.grid(row=0, column=0, sticky="nsew")

        self.tab_video = TabVideo(self.tabview.tab("Video"))
        self.tab_video.grid(row=0, column=0, sticky="nsew")

        self.tab_audio = TabAudio(self.tabview.tab("Audio"))
        self.tab_audio.grid(row=0, column=0, sticky="nsew")

        self.tab_speech = TabSpeech(self.tabview.tab("Speech"))
        self.tab_speech.grid(row=0, column=0, sticky="nsew")

        self.tab_llm = TabLLM(self.tabview.tab("LLM"))
        self.tab_llm.grid(row=0, column=0, sticky="nsew")

        self.tab_settings = TabSettings(self.tabview.tab("Downloader"))
        self.tab_settings.grid(row=0, column=0, sticky="nsew")

        self.tab_history = TabHistory(self.tabview.tab("History"))
        self.tab_history.grid(row=0, column=0, sticky="nsew")

    # --- first-run banner --------------------------------------------------

    def _maybe_show_first_run_banner(self) -> None:
        """Non-blocking strip telling the user which default models are missing."""
        missing = [
            m for m in DEFAULT_MODELS if not registry.get_default_model(m)
        ]
        if not missing:
            return
        banner = ctk.CTkFrame(self, fg_color="gray25")
        banner.grid(row=0, column=0, sticky="ew", padx=10, pady=(10, 0))
        banner.grid_columnconfigure(0, weight=1)
        text = (
            "No default models installed yet — open the **Downloader** tab to fetch what you need. "
            f"Missing: {', '.join(missing)}."
        )
        ctk.CTkLabel(banner, text=text, anchor="w", wraplength=900).grid(
            row=0, column=0, sticky="ew", padx=10, pady=6
        )
        ctk.CTkButton(banner, text="✕", width=30, command=banner.destroy).grid(
            row=0, column=1, padx=5, pady=4
        )
        self._banner = banner
