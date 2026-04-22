import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from pyforge.engines.stt_engine import STTEngine
from tkinter import filedialog
import threading
import os

class STTPanel(ttk.Frame):
    def __init__(self, master, stt_engine=None, **kwargs):
        super().__init__(master, **kwargs)
        
        # Initialize or use existing STT Engine
        self.stt_engine = stt_engine if stt_engine else STTEngine()
        
        # Settings Variables
        self.language = ttk.StringVar(master=self, value="auto")
        self.model_size = ttk.StringVar(master=self, value="base")
        
        self.setup_ui()

    def setup_ui(self):
        # Action Buttons Area
        actions_frame = ttk.Frame(self, padding=10)
        actions_frame.pack(fill=X, side=TOP)
        
        self.upload_btn = ttk.Button(
            actions_frame, 
            text="Upload Audio File", 
            command=self.upload_audio, 
            bootstyle="primary"
        )
        self.upload_btn.pack(side=LEFT, padx=5)
        
        self.record_btn = ttk.Button(
            actions_frame, 
            text="Record (Mock)", 
            command=self.mock_record, 
            bootstyle="danger-outline"
        )
        self.record_btn.pack(side=LEFT, padx=5)
        
        # Transcription Display Area
        ttk.Label(self, text="Transcription:", font=("Helvetica", 10, "bold"), padding=(10, 5, 10, 0)).pack(anchor=W)
        
        self.display_frame = ttk.Frame(self, padding=10)
        self.display_frame.pack(fill=BOTH, expand=True)
        
        self.transcription_text = ttk.Text(self.display_frame, font=("Helvetica", 11), wrap=WORD)
        self.transcription_text.pack(fill=BOTH, expand=True)
        
        self.status_label = ttk.Label(self, text="Ready", padding=10)
        self.status_label.pack(side=BOTTOM, anchor=W)

    def setup_sidebar(self, sidebar_frame):
        """Adds STT-specific controls to the sidebar."""
        ttk.Label(
            sidebar_frame, 
            text="STT Parameters", 
            font=("Helvetica", 11, "bold")
        ).pack(pady=(20, 10), padx=10, anchor=W)
        
        # Language
        ttk.Label(sidebar_frame, text="Language (auto for detect):").pack(padx=10, anchor=W)
        languages = ["auto", "en", "es", "fr", "de", "it", "pt", "pl", "tr", "ru", "nl", "cs", "ar", "zh", "ja", "ko"]
        lang_combo = ttk.Combobox(sidebar_frame, textvariable=self.language, values=languages, state="readonly")
        lang_combo.pack(fill=X, padx=10, pady=(0, 10))
        
        # Model Size
        ttk.Label(sidebar_frame, text="Whisper Model Size:").pack(padx=10, anchor=W)
        model_sizes = ["tiny", "base", "small", "medium", "large"]
        size_combo = ttk.Combobox(sidebar_frame, textvariable=self.model_size, values=model_sizes, state="readonly")
        size_combo.pack(fill=X, padx=10, pady=(0, 10))
        
        def on_size_change(*args):
            self.status_label.config(text=f"Note: Model will be loaded on next transcription: {self.model_size.get()}")
        self.model_size.trace_add("write", on_size_change)

    def upload_audio(self):
        file_path = filedialog.askopenfilename(
            filetypes=[("Audio Files", "*.mp3 *.wav *.m4a *.flac *.ogg"), ("All Files", "*.*")]
        )
        if file_path:
            self.start_transcription(file_path)

    def mock_record(self):
        self.status_label.config(text="Recording... (This is a mock, no actual audio is being recorded)")
        self.after(2000, lambda: self.status_label.config(text="Recording finished (Mock). Please upload a file to transcribe."))

    def start_transcription(self, file_path):
        self.upload_btn.config(state=DISABLED)
        self.status_label.config(text=f"Transcribing {os.path.basename(file_path)}... please wait.")
        self.transcription_text.delete("1.0", END)
        
        # Start transcription in a separate thread
        thread = threading.Thread(target=self._transcribe_thread, args=(file_path,))
        thread.start()

    def _transcribe_thread(self, file_path):
        try:
            # Re-load model if size changed
            if not self.stt_engine.model or self.stt_engine.model_size != self.model_size.get():
                self.stt_engine.load_model(self.model_size.get())
            
            lang = self.language.get()
            if lang == "auto":
                lang = None
                
            text = self.stt_engine.transcribe(file_path, language=lang)
            
            if text:
                self.after(0, lambda: self._update_ui_success(text))
            else:
                self.after(0, lambda: self.status_label.config(text="Error: Transcription failed."))
        except Exception as e:
            self.after(0, lambda: self.status_label.config(text=f"Error: {str(e)}"))
        finally:
            self.after(0, lambda: self.upload_btn.config(state=NORMAL))

    def _update_ui_success(self, text):
        self.transcription_text.insert(END, text)
        self.status_label.config(text="Transcription complete.")
