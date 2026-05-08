import customtkinter as ctk
from models.downloader import downloader
from ui.components.progress_bar import ProgressBar
import threading

class TabSettings(ctk.CTkFrame):
    def __init__(self, master, **kwargs):
        super().__init__(master, **kwargs)
        
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)

        lbl = ctk.CTkLabel(self, text="Model Downloader", font=ctk.CTkFont(size=20, weight="bold"))
        lbl.grid(row=0, column=0, padx=10, pady=(10,0), sticky="w")

        # Search Frame
        search_frame = ctk.CTkFrame(self)
        search_frame.grid(row=1, column=0, padx=10, pady=10, sticky="nsew")
        search_frame.grid_columnconfigure(1, weight=1)
        
        ctk.CTkLabel(search_frame, text="Modality:").grid(row=0, column=0, padx=5, pady=5)
        self.modality_combo = ctk.CTkComboBox(search_frame, values=["image", "video", "audio", "speech", "llm"])
        self.modality_combo.grid(row=0, column=1, padx=5, pady=5, sticky="w")
        
        self.search_entry = ctk.CTkEntry(search_frame, placeholder_text="Search HuggingFace...")
        self.search_entry.grid(row=1, column=0, columnspan=2, padx=5, pady=5, sticky="ew")
        
        self.search_btn = ctk.CTkButton(search_frame, text="Search", command=self._search)
        self.search_btn.grid(row=1, column=2, padx=5, pady=5)

        self.results_textbox = ctk.CTkTextbox(search_frame)
        self.results_textbox.grid(row=2, column=0, columnspan=3, padx=5, pady=5, sticky="nsew")
        search_frame.grid_rowconfigure(2, weight=1)

        # Download Frame
        dl_frame = ctk.CTkFrame(self)
        dl_frame.grid(row=2, column=0, padx=10, pady=10, sticky="ew")
        dl_frame.grid_columnconfigure(0, weight=1)

        self.dl_entry = ctk.CTkEntry(dl_frame, placeholder_text="Model ID to download (e.g. runwayml/stable-diffusion-v1-5)")
        self.dl_entry.grid(row=0, column=0, padx=5, pady=5, sticky="ew")
        
        self.dl_btn = ctk.CTkButton(dl_frame, text="Download", command=self._download)
        self.dl_btn.grid(row=0, column=1, padx=5, pady=5)

        self.progress = ProgressBar(dl_frame)
        self.progress.grid(row=1, column=0, columnspan=2, sticky="ew", padx=5, pady=5)

    def _search(self):
        query = self.search_entry.get()
        modality = self.modality_combo.get()
        self.results_textbox.delete("1.0", "end")
        self.results_textbox.insert("end", "Searching...\n")
        
        def do_search():
            results = downloader.search_huggingface(query, modality)
            self.master.after(0, self._show_results, results)
            
        threading.Thread(target=do_search, daemon=True).start()
        
    def _show_results(self, results):
        self.results_textbox.delete("1.0", "end")
        if not results:
            self.results_textbox.insert("end", "No results found.\n")
            return
            
        for r in results:
            self.results_textbox.insert("end", f"ID: {r['id']}\nDownloads: {r['downloads']}\nTags: {r['tags'][:3]}\n---\n")

    def _download(self):
        hf_id = self.dl_entry.get()
        modality = self.modality_combo.get()
        if not hf_id:
            return
            
        self.progress.set(0)
        self.dl_btn.configure(state="disabled")
        
        def progress_cb(val):
            # Not a real progress, just jumping to 100 for simplicity in this MVP
            self.master.after(0, self.progress.set, val/100.0)

        def do_download():
            success = downloader.download_model(hf_id, modality, progress_cb=progress_cb)
            self.master.after(0, self._download_complete, success)
            
        threading.Thread(target=do_download, daemon=True).start()

    def _download_complete(self, success):
        self.dl_btn.configure(state="normal")
        if success:
            self.results_textbox.insert("end", "\nDownload complete!")
        else:
            self.results_textbox.insert("end", "\nDownload failed.")
