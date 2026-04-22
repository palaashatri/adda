import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from tkinter import filedialog
from pyforge.core.settings import settings
import torch

class SettingsPanel(ttk.Frame):
    def __init__(self, parent, api_server=None):
        super().__init__(parent)
        self.api_server = api_server
        self.setup_ui()

    def setup_ui(self):
        # Header
        header = ttk.Label(self, text="Settings", font=("Helvetica", 18, "bold"))
        header.pack(pady=20, padx=20, anchor=W)

        # Settings Container
        container = ttk.Frame(self)
        container.pack(fill=BOTH, expand=True, padx=40)

        # --- Theme Selection ---
        theme_frame = ttk.Frame(container)
        theme_frame.pack(fill=X, pady=10)
        
        ttk.Label(theme_frame, text="Theme", font=("Helvetica", 12)).pack(side=LEFT)
        self.theme_var = ttk.StringVar(master=self, value=settings.get("theme"))
        theme_combo = ttk.Combobox(theme_frame, textvariable=self.theme_var, values=["darkly", "flatly", "cosmo", "superhero"], width=200)
        theme_combo.pack(side=RIGHT)
        theme_combo.bind("<<ComboboxSelected>>", self.on_theme_change)

        # --- Model Cache Path ---
        cache_frame = ttk.Frame(container)
        cache_frame.pack(fill=X, pady=10)
        
        ttk.Label(cache_frame, text="Model Cache Path", font=("Helvetica", 12)).pack(side=LEFT)
        self.cache_var = ttk.StringVar(master=self, value=settings.get("model_cache"))
        ttk.Entry(cache_frame, textvariable=self.cache_var).pack(side=LEFT, fill=X, expand=True, padx=10)
        ttk.Button(cache_frame, text="Browse", command=self.browse_cache).pack(side=RIGHT)

        # --- API Server Toggle ---
        api_frame = ttk.Frame(container)
        api_frame.pack(fill=X, pady=10)
        
        ttk.Label(api_frame, text="Unified Local API Server", font=("Helvetica", 12)).pack(side=LEFT)
        self.api_status_label = ttk.Label(api_frame, text="Stopped", bootstyle="danger")
        self.api_status_label.pack(side=LEFT, padx=10)
        
        self.api_btn = ttk.Button(api_frame, text="Start API", command=self.toggle_api_server)
        self.api_btn.pack(side=RIGHT)
        
        if self.api_server and self.api_server.server_thread and self.api_server.server_thread.is_alive():
            self.api_status_label.config(text="Running", bootstyle="success")
            self.api_btn.config(text="API Running (Restart app to stop)")

        # --- GPU / Device Selection ---
        device_frame = ttk.Frame(container)
        device_frame.pack(fill=X, pady=10)
        
        ttk.Label(device_frame, text="Compute Device", font=("Helvetica", 12)).pack(side=LEFT)
        
        devices = ["cpu"]
        if torch.cuda.is_available():
            devices.append("cuda")
        if torch.backends.mps.is_available():
            devices.append("mps")
            
        self.device_var = ttk.StringVar(master=self, value="cuda" if torch.cuda.is_available() else ("mps" if torch.backends.mps.is_available() else "cpu"))
        device_combo = ttk.Combobox(device_frame, textvariable=self.device_var, values=devices, width=200)
        device_combo.pack(side=RIGHT)

        # --- Save Button ---
        ttk.Button(container, text="Save All Settings", bootstyle="success", command=self.save_settings).pack(pady=30)

    def on_theme_change(self, event=None):
        theme = self.theme_var.get()
        settings.set("theme", theme)
        # Apply theme immediately if possible (requires window reference)
        # In this simple implementation, it might need an app restart or 
        # a more direct way to call style.theme_use()

    def browse_cache(self):
        path = filedialog.askdirectory()
        if path:
            self.cache_var.set(path)
            settings.set("model_cache", path)

    def toggle_api_server(self):
        if not self.api_server:
            return
            
        if self.api_server.server_thread and self.api_server.server_thread.is_alive():
            # In this simple prototype, stopping is hard, so we just inform
            return
            
        success = self.api_server.start(port=settings.get("api_port"))
        if success:
            self.api_status_label.config(text="Running", bootstyle="success")
            self.api_btn.config(text="API Running (Restart app to stop)")
            settings.set("api_server_enabled", True)

    def save_settings(self):
        settings.set("theme", self.theme_var.get())
        settings.set("model_cache", self.cache_var.get())
        # Other settings can be saved here
        print("Settings saved.")
