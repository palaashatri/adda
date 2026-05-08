import customtkinter as ctk
import threading
from core.logger import logger
from core.config import DEFAULT_MODELS
from models.registry import registry
from models.downloader import downloader
from ui.theme import apply_theme
from ui.main_window import MainWindow

class PyForgeApp:
    def __init__(self):
        apply_theme()
        self.window = MainWindow()

    def check_default_models(self):
        """Checks if default models are present, prompts to download if not."""
        missing = []
        for modality in ["image", "video", "audio", "speech", "llm"]:
            if modality in DEFAULT_MODELS:
                if not registry.get_default_model(modality):
                    missing.append((modality, DEFAULT_MODELS[modality]))
                    
        if missing:
            self._show_download_modal(missing)
        else:
            self.window.mainloop()

    def _show_download_modal(self, missing):
        modal = ctk.CTkToplevel(self.window)
        modal.title("Download Default Models")
        modal.geometry("400x250")
        modal.transient(self.window)
        modal.grab_set()
        
        lbl = ctk.CTkLabel(modal, text=f"PyForge needs to download {len(missing)} default models.\nTotal size: ~3 GB", justify="center")
        lbl.pack(pady=20)
        
        progress = ctk.CTkProgressBar(modal)
        progress.pack(pady=10, padx=20, fill="x")
        progress.set(0)
        
        status_lbl = ctk.CTkLabel(modal, text="Waiting...")
        status_lbl.pack(pady=5)
        
        def start_download():
            btn_dl.configure(state="disabled")
            def do_downloads():
                for i, (modality, hf_id) in enumerate(missing):
                    status_lbl.configure(text=f"Downloading {hf_id}...")
                    
                    def p_cb(val):
                        progress.set(val/100.0)
                        
                    success = downloader.download_model(hf_id, modality, is_default=True, progress_cb=p_cb)
                    if not success:
                        logger.error(f"Failed to download default model: {hf_id}")
                
                modal.destroy()
                # Refresh all tabs
                for tab in [self.window.tab_image, self.window.tab_video, self.window.tab_audio, self.window.tab_speech, self.window.tab_llm]:
                    tab.model_selector.refresh()
                self.window.mainloop()
                
            threading.Thread(target=do_downloads, daemon=True).start()
            
        def exit_app():
            modal.destroy()
            self.window.destroy()

        btn_frame = ctk.CTkFrame(modal, fg_color="transparent")
        btn_frame.pack(pady=10)
        
        btn_dl = ctk.CTkButton(btn_frame, text="Download", command=start_download)
        btn_dl.pack(side="left", padx=10)
        
        btn_exit = ctk.CTkButton(btn_frame, text="Exit", command=exit_app)
        btn_exit.pack(side="right", padx=10)

    def run(self):
        # We start by checking models. If present, it directly calls mainloop.
        # Avoid calling mainloop directly here if we need to show a modal first.
        self.window.after(100, self.check_default_models)
        self.window.mainloop()
