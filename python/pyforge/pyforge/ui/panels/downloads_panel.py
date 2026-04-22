import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from ttkbootstrap.scrolled import ScrolledFrame
import threading
from pyforge.core.hf_client import hf_client
from pyforge.core.model_registry import model_registry

class DownloadsPanel(ttk.Frame):
    def __init__(self, master, **kwargs):
        super().__init__(master, **kwargs)
        self.setup_ui()
        self.refresh_local_models()

    def setup_ui(self):
        # Main layout: Left (Search & Results), Right (Local Models)
        self.main_container = ttk.Panedwindow(self, orient=HORIZONTAL)
        self.main_container.pack(fill=BOTH, expand=True, padx=10, pady=10)

        # --- Search Section (Left) ---
        self.search_frame = ttk.Frame(self.main_container, padding=10)
        self.main_container.add(self.search_frame, weight=3)

        ttk.Label(self.search_frame, text="Search HuggingFace Models", font=("Helvetica", 12, "bold")).pack(anchor=W, pady=(0, 10))

        # Search Controls
        self.controls_frame = ttk.Frame(self.search_frame)
        self.controls_frame.pack(fill=X, pady=(0, 10))

        self.search_query = ttk.StringVar(master=self)
        self.search_entry = ttk.Entry(self.controls_frame, textvariable=self.search_query)
        self.search_entry.pack(side=LEFT, fill=X, expand=True, padx=(0, 5))
        self.search_entry.bind("<Return>", lambda e: self.search_hf())

        self.category_var = ttk.StringVar(master=self, value="LLM")
        self.category_menu = ttk.Combobox(
            self.controls_frame, 
            textvariable=self.category_var, 
            values=["LLM", "Image", "Video", "Audio", "TTS", "STT"],
            width=10,
            state="readonly"
        )
        self.category_menu.pack(side=LEFT, padx=5)

        self.search_btn = ttk.Button(self.controls_frame, text="Search", command=self.search_hf, bootstyle="primary")
        self.search_btn.pack(side=LEFT, padx=5)

        # Search Results
        self.results_frame = ScrolledFrame(self.search_frame, autohide=True)
        self.results_frame.pack(fill=BOTH, expand=True)

        # --- Local Models Section (Right) ---
        self.local_frame = ttk.Frame(self.main_container, padding=10)
        self.main_container.add(self.local_frame, weight=2)

        ttk.Label(self.local_frame, text="Local Models", font=("Helvetica", 12, "bold")).pack(anchor=W, pady=(0, 10))
        
        self.local_models_list = ScrolledFrame(self.local_frame, autohide=True)
        self.local_models_list.pack(fill=BOTH, expand=True)

    def search_hf(self):
        query = self.search_query.get()
        category = self.category_var.get()
        
        # Clear existing results
        for widget in self.results_frame.winfo_children():
            widget.destroy()
            
        loading_label = ttk.Label(self.results_frame, text="Searching...")
        loading_label.pack(pady=20)

        def perform_search():
            try:
                results = hf_client.search_models(query=query, category=category)
                self.after(0, lambda results=results: self.display_results(results))
            except Exception as e:
                self.after(0, lambda e=e: ttk.Label(self.results_frame, text=f"Error: {e}", bootstyle="danger").pack())
            finally:
                self.after(0, loading_label.destroy)

        threading.Thread(target=perform_search).start()

    def display_results(self, results):
        if not results:
            ttk.Label(self.results_frame, text="No models found.").pack(pady=20)
            return

        for model in results:
            card = ttk.Frame(self.results_frame, bootstyle="secondary", padding=10)
            card.pack(fill=X, pady=5, padx=5)
            
            # Model Info
            info_frame = ttk.Frame(card, bootstyle="secondary")
            info_frame.pack(side=LEFT, fill=BOTH, expand=True)
            
            ttk.Label(info_frame, text=model['id'], font=("Helvetica", 10, "bold"), bootstyle="inverse-secondary").pack(anchor=W)
            ttk.Label(info_frame, text=f"Downloads: {model['downloads']} | Likes: {model['likes']}", font=("Helvetica", 8), bootstyle="inverse-secondary").pack(anchor=W)
            
            # Download Button
            btn_state = "disabled" if model_registry.is_model_downloaded(model['id']) else "normal"
            btn_text = "Downloaded" if model_registry.is_model_downloaded(model['id']) else "Download"
            
            dl_btn = ttk.Button(
                card, 
                text=btn_text, 
                command=lambda m=model: self.start_download(m),
                bootstyle="success-outline",
                state=btn_state
            )
            dl_btn.pack(side=RIGHT, padx=5)

    def start_download(self, model_metadata):
        model_id = model_metadata['id']
        
        # Create progress indicator
        progress_popup = ttk.Toplevel(self)
        progress_popup.title(f"Downloading {model_id}")
        progress_popup.geometry("400x150")
        progress_popup.transient(self)
        
        ttk.Label(progress_popup, text=f"Downloading {model_id}...", wraplength=350).pack(pady=20)
        progress = ttk.Progressbar(progress_popup, mode='indeterminate', length=300)
        progress.pack(pady=10)
        progress.start()

        def on_complete(current, total, success=True, error=None, model_id=None):
            progress.stop()
            progress_popup.destroy()
            if success:
                model_registry.add_model(model_id, model_metadata)
                self.after(0, self.refresh_local_models)
                self.after(0, self.search_hf) # Refresh search results to update button state
            else:
                from tkinter import messagebox
                messagebox.showerror("Download Failed", f"Failed to download {model_id}: {error}")

        hf_client.download_model(model_id, callback=on_complete)

    def refresh_local_models(self):
        for widget in self.local_models_list.winfo_children():
            widget.destroy()
            
        models = model_registry.get_local_models()
        if not models:
            ttk.Label(self.local_models_list, text="No local models found.").pack(pady=20)
            return

        for model_id, metadata in models.items():
            card = ttk.Frame(self.local_models_list, bootstyle="secondary", padding=10)
            card.pack(fill=X, pady=5, padx=5)
            
            info_frame = ttk.Frame(card, bootstyle="secondary")
            info_frame.pack(side=LEFT, fill=BOTH, expand=True)
            
            ttk.Label(info_frame, text=model_id, font=("Helvetica", 10, "bold"), bootstyle="inverse-secondary").pack(anchor=W)
            ttk.Label(info_frame, text=f"Category: {metadata.get('pipeline_tag', 'N/A')}", font=("Helvetica", 8), bootstyle="inverse-secondary").pack(anchor=W)
            
            delete_btn = ttk.Button(
                card, 
                text="Delete", 
                command=lambda m=model_id: self.delete_model(m),
                bootstyle="danger-outline"
            )
            delete_btn.pack(side=RIGHT, padx=5)

    def delete_model(self, model_id):
        from tkinter import messagebox
        if messagebox.askyesno("Delete Model", f"Are you sure you want to delete {model_id}?"):
            model_registry.remove_model(model_id)
            self.refresh_local_models()
            self.search_hf() # Update search buttons

    def setup_sidebar(self, sidebar_frame):
        """Adds help text to the sidebar for the Downloads panel."""
        ttk.Label(
            sidebar_frame, 
            text="Model Manager", 
            font=("Helvetica", 11, "bold")
        ).pack(pady=(20, 10), padx=10, anchor=W)
        
        help_text = (
            "Search for models on HuggingFace and download them locally.\n\n"
            "Supported Categories:\n"
            "- LLM: Text generation\n"
            "- Image: Stable Diffusion, etc.\n"
            "- Video: LTX-Video, etc.\n"
            "- Audio: MusicGen, etc.\n"
            "- TTS: Text-to-Speech\n"
            "- STT: Speech-to-Text"
        )
        
        ttk.Label(
            sidebar_frame, 
            text=help_text, 
            wraplength=180, 
            justify=LEFT,
            font=("Helvetica", 9)
        ).pack(padx=10, anchor=W)
