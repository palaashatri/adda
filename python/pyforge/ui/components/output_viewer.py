"""Output viewer: image / text / audio / video preview with platform-native playback."""
from __future__ import annotations

import os
import platform
import subprocess
from pathlib import Path
from typing import Optional

import customtkinter as ctk
from PIL import Image


def open_with_default_app(path: str) -> None:
    """Open `path` with the OS default application (cross-platform)."""
    if not path:
        return
    system = platform.system()
    try:
        if system == "Windows":
            os.startfile(path)  # type: ignore[attr-defined]
        elif system == "Darwin":
            subprocess.Popen(["open", path])
        else:
            subprocess.Popen(["xdg-open", path])
    except Exception:
        pass


class OutputViewer(ctk.CTkFrame):
    """Switchable preview pane: text, PIL image, audio (with Play button), or video file."""

    def __init__(self, master, **kwargs) -> None:
        super().__init__(master, **kwargs)
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(0, weight=1)

        self.label = ctk.CTkLabel(
            self, text="Output Preview", fg_color="gray20", corner_radius=8
        )
        self.label.grid(row=0, column=0, sticky="nsew", padx=10, pady=10)

        self.action_frame = ctk.CTkFrame(self, fg_color="transparent")
        self.action_frame.grid(row=1, column=0, sticky="ew")
        self.action_btn: Optional[ctk.CTkButton] = None
        self._current_path: Optional[str] = None

    def _clear_action(self) -> None:
        if self.action_btn is not None:
            self.action_btn.destroy()
            self.action_btn = None
        self._current_path = None

    def show_image(self, image: Image.Image) -> None:
        """Display a PIL image, downscaled to fit ~512px on its longer side."""
        self._clear_action()
        max_side = 512
        w, h = image.size
        if max(w, h) > max_side:
            scale = max_side / float(max(w, h))
            preview = image.resize((int(w * scale), int(h * scale)))
        else:
            preview = image
        ctk_image = ctk.CTkImage(
            light_image=preview, dark_image=preview, size=preview.size
        )
        self.label.configure(image=ctk_image, text="")

    def show_text(self, text: str) -> None:
        self._clear_action()
        self.label.configure(image="", text=text, justify="left", anchor="nw", wraplength=500)

    def show_audio(self, audio_path: str) -> None:
        """Show a metadata blurb plus a Play button that opens the OS player."""
        self._clear_action()
        self.label.configure(
            image="",
            text=f"Audio saved at:\n{audio_path}",
            justify="left",
            anchor="nw",
            wraplength=500,
        )
        self._current_path = audio_path
        self.action_btn = ctk.CTkButton(
            self.action_frame,
            text="▶ Play",
            command=lambda: open_with_default_app(audio_path),
        )
        self.action_btn.grid(row=0, column=0, sticky="w", padx=10, pady=4)

    def show_video(self, video_path: str) -> None:
        """Either preview a GIF inline or fall back to an Open button."""
        self._clear_action()
        p = Path(video_path) if video_path else None
        if p and p.suffix.lower() == ".gif" and p.exists():
            try:
                img = Image.open(p)
                self.show_image(img)
                self._current_path = video_path
                self.action_btn = ctk.CTkButton(
                    self.action_frame,
                    text="▶ Open externally",
                    command=lambda: open_with_default_app(video_path),
                )
                self.action_btn.grid(row=0, column=0, sticky="w", padx=10, pady=4)
                return
            except Exception:
                pass
        self.label.configure(
            image="",
            text=f"Video saved at:\n{video_path}",
            justify="left",
            anchor="nw",
            wraplength=500,
        )
        if video_path:
            self._current_path = video_path
            self.action_btn = ctk.CTkButton(
                self.action_frame,
                text="▶ Open",
                command=lambda: open_with_default_app(video_path),
            )
            self.action_btn.grid(row=0, column=0, sticky="w", padx=10, pady=4)

    def clear(self) -> None:
        self._clear_action()
        self.label.configure(image="", text="Output Preview")
