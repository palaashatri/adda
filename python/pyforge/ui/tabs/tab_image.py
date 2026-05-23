"""Image tab: txt2img / img2img / inpaint / upscale with full controls."""
from __future__ import annotations

import random
from tkinter import filedialog
from typing import Optional

import customtkinter as ctk
from PIL import Image

from core.scheduler import EngineRequest
from core.task_queue import task_queue
from engines.engine_image import engine_image
from ui.components.history_panel import HistoryPanel, save_entry
from ui.components.model_selector import ModelSelector
from ui.components.output_viewer import OutputViewer
from ui.components.progress_bar import ProgressBar
from ui.components.prompt_box import PromptBox


IMAGE_MODES = ("txt2img", "img2img", "inpaint", "upscale")


class TabImage(ctk.CTkFrame):
    """Full image-generation panel with mode/seed/steps/guidance/dimension controls."""

    def __init__(self, master, **kwargs) -> None:
        super().__init__(master, **kwargs)
        self.grid_columnconfigure(0, weight=3)
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(2, weight=1)

        self.model_selector = ModelSelector(self, modality="image")
        self.model_selector.grid(row=0, column=0, columnspan=2, sticky="ew", padx=10, pady=5)

        self._build_controls()

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

        self.history = HistoryPanel(self, modality="image", on_select=self._restore_history)
        self.history.grid(row=2, column=1, sticky="nsew", padx=(0, 10), pady=5)

        self._init_image: Optional[Image.Image] = None
        self._mask_image: Optional[Image.Image] = None
        self._current_task_id: Optional[str] = None
        self._current_token = None

    # --- UI scaffolding ----------------------------------------------------

    def _build_controls(self) -> None:
        frame = ctk.CTkFrame(self)
        frame.grid(row=1, column=0, columnspan=2, sticky="ew", padx=10, pady=5)
        for c in range(8):
            frame.grid_columnconfigure(c, weight=1)

        ctk.CTkLabel(frame, text="Mode:").grid(row=0, column=0, padx=4, pady=4, sticky="e")
        self.mode_combo = ctk.CTkComboBox(frame, values=list(IMAGE_MODES), command=self._on_mode)
        self.mode_combo.set("txt2img")
        self.mode_combo.grid(row=0, column=1, padx=4, pady=4, sticky="w")

        ctk.CTkLabel(frame, text="Seed:").grid(row=0, column=2, padx=4, pady=4, sticky="e")
        self.seed_entry = ctk.CTkEntry(frame, placeholder_text="random", width=80)
        self.seed_entry.grid(row=0, column=3, padx=4, pady=4, sticky="w")
        ctk.CTkButton(frame, text="🎲", width=30, command=self._random_seed).grid(
            row=0, column=4, padx=2, pady=4, sticky="w"
        )

        ctk.CTkLabel(frame, text="Steps:").grid(row=0, column=5, padx=4, pady=4, sticky="e")
        self.steps_var = ctk.IntVar(value=20)
        self.steps_slider = ctk.CTkSlider(
            frame, from_=5, to=100, number_of_steps=95, variable=self.steps_var
        )
        self.steps_slider.grid(row=0, column=6, padx=4, pady=4, sticky="ew")
        self.steps_lbl = ctk.CTkLabel(frame, text="20")
        self.steps_lbl.grid(row=0, column=7, padx=4, pady=4, sticky="w")
        self.steps_var.trace_add("write", lambda *_: self.steps_lbl.configure(text=str(self.steps_var.get())))

        ctk.CTkLabel(frame, text="Guidance:").grid(row=1, column=0, padx=4, pady=4, sticky="e")
        self.guidance_var = ctk.DoubleVar(value=7.5)
        self.guidance_slider = ctk.CTkSlider(
            frame, from_=1.0, to=20.0, number_of_steps=190, variable=self.guidance_var
        )
        self.guidance_slider.grid(row=1, column=1, columnspan=2, padx=4, pady=4, sticky="ew")
        self.guidance_lbl = ctk.CTkLabel(frame, text="7.5")
        self.guidance_lbl.grid(row=1, column=3, padx=4, pady=4, sticky="w")
        self.guidance_var.trace_add(
            "write",
            lambda *_: self.guidance_lbl.configure(text=f"{self.guidance_var.get():.1f}"),
        )

        ctk.CTkLabel(frame, text="W×H:").grid(row=1, column=4, padx=4, pady=4, sticky="e")
        self.width_entry = ctk.CTkEntry(frame, width=70, placeholder_text="512")
        self.width_entry.insert(0, "512")
        self.width_entry.grid(row=1, column=5, padx=2, pady=4, sticky="w")
        self.height_entry = ctk.CTkEntry(frame, width=70, placeholder_text="512")
        self.height_entry.insert(0, "512")
        self.height_entry.grid(row=1, column=6, padx=2, pady=4, sticky="w")

        self.upload_btn = ctk.CTkButton(frame, text="Load image…", command=self._load_init_image)
        self.upload_btn.grid(row=1, column=7, padx=4, pady=4, sticky="w")
        self.mask_btn = ctk.CTkButton(frame, text="Load mask…", command=self._load_mask_image)
        self.mask_btn.grid(row=0, column=7, padx=4, pady=4, sticky="w")
        self.cancel_btn = ctk.CTkButton(
            frame, text="Cancel", fg_color="firebrick", command=self._cancel
        )
        self.cancel_btn.grid(row=2, column=7, padx=4, pady=4, sticky="ew")
        self.cancel_btn.configure(state="disabled")

        self._on_mode("txt2img")

    # --- event handlers ----------------------------------------------------

    def _on_mode(self, mode: str) -> None:
        needs_image = mode in ("img2img", "inpaint", "upscale")
        needs_mask = mode == "inpaint"
        self.upload_btn.configure(state="normal" if needs_image else "disabled")
        self.mask_btn.configure(state="normal" if needs_mask else "disabled")

    def _random_seed(self) -> None:
        self.seed_entry.delete(0, "end")
        self.seed_entry.insert(0, str(random.randint(0, 2**31 - 1)))

    def _load_init_image(self) -> None:
        path = filedialog.askopenfilename(filetypes=[("Image", "*.png *.jpg *.jpeg *.webp *.bmp")])
        if path:
            self._init_image = Image.open(path).convert("RGB")
            self.viewer.show_image(self._init_image)

    def _load_mask_image(self) -> None:
        path = filedialog.askopenfilename(filetypes=[("Image", "*.png *.jpg *.jpeg *.webp *.bmp")])
        if path:
            self._mask_image = Image.open(path).convert("L")

    def _cancel(self) -> None:
        if self._current_token is not None:
            self._current_token.cancel()
        if self._current_task_id:
            task_queue.cancel_task(self._current_task_id)

    # --- generation --------------------------------------------------------

    def _generate(self, prompt: str) -> None:
        if self.model_selector.combo.get() in ("", "No models found"):
            self.viewer.show_text("Error: no image model installed. Use the Downloader tab.")
            return

        mode = self.mode_combo.get()
        seed_raw = self.seed_entry.get().strip()
        seed = int(seed_raw) if seed_raw.isdigit() else None

        try:
            width = int(self.width_entry.get() or 512)
            height = int(self.height_entry.get() or 512)
        except ValueError:
            width, height = 512, 512

        kwargs = {
            "mode": mode,
            "steps": self.steps_var.get(),
            "guidance": self.guidance_var.get(),
            "seed": seed,
            "width": width,
            "height": height,
        }
        if mode != "txt2img":
            kwargs["init_image"] = self._init_image
        if mode == "inpaint":
            kwargs["mask_image"] = self._mask_image

        request = EngineRequest(
            prompt=prompt,
            model_id=self.model_selector.combo.get(),
            modality="image",
            **kwargs,
        )

        self.viewer.show_text(f"Generating ({mode})…")
        self.progress.grid()
        self.progress.update_progress(0.05)
        self.cancel_btn.configure(state="normal")
        self._current_task_id = f"image_gen_{id(request)}"

        future, token = task_queue.submit_task(
            self._current_task_id, self._run_engine, request
        )
        request.cancel_token = token
        self._current_token = token
        future.add_done_callback(lambda _f: self.master.after(0, self._reset_buttons))

    def _run_engine(self, request: EngineRequest) -> None:
        self.master.after(0, self.progress.update_progress, 0.5)
        response = engine_image.run(request)
        if response.success:
            self.master.after(0, self._on_success, response.output, request, response.metadata)
        else:
            self.master.after(0, self._on_error, response.error)

    def _on_success(self, image: Image.Image, request: EngineRequest, metadata: dict) -> None:
        self.progress.grid_remove()
        self.viewer.show_image(image)
        try:
            from pathlib import Path
            import tempfile, uuid
            out = Path(tempfile.gettempdir()) / f"pyforge_image_{uuid.uuid4().hex[:8]}.png"
            image.save(out)
            preview_path = str(out)
        except Exception:
            preview_path = ""
        entry = save_entry(
            modality="image",
            prompt=request.prompt,
            model_name=metadata.get("model", ""),
            settings={k: v for k, v in request.kwargs.items() if k not in ("init_image", "mask_image")},
            seed=metadata.get("seed"),
            preview_path=preview_path,
        )
        self.history.add_history(entry)

    def _on_error(self, error: str) -> None:
        self.progress.grid_remove()
        self.viewer.show_text(f"Error: {error}")

    def _reset_buttons(self) -> None:
        self.cancel_btn.configure(state="disabled")
        self._current_task_id = None
        self._current_token = None

    # --- history restore ---------------------------------------------------

    def _restore_history(self, entry: dict) -> None:
        self.prompt_box.set_prompt(entry.get("prompt", ""))
        settings = entry.get("settings", {})
        if "mode" in settings:
            self.mode_combo.set(settings["mode"])
            self._on_mode(settings["mode"])
        if "steps" in settings:
            self.steps_var.set(int(settings["steps"]))
        if "guidance" in settings:
            self.guidance_var.set(float(settings["guidance"]))
        if "seed" in settings and settings["seed"] is not None:
            self.seed_entry.delete(0, "end")
            self.seed_entry.insert(0, str(settings["seed"]))
        if "width" in settings:
            self.width_entry.delete(0, "end")
            self.width_entry.insert(0, str(settings["width"]))
        if "height" in settings:
            self.height_entry.delete(0, "end")
            self.height_entry.insert(0, str(settings["height"]))
