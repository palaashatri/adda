import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from pyforge.engines.tts_engine import TTSEngine
import threading
import os
import tempfile

class TTSPanel(ttk.Frame):
    def __init__(self, master, tts_engine=None, **kwargs):
        super().__init__(master, **kwargs)
        
        # Initialize or use existing TTS Engine
        self.tts_engine = tts_engine if tts_engine else TTSEngine()
        
        # Settings Variables
        self.voice = ttk.StringVar(master=self, value="Default")
        self.language = ttk.StringVar(master=self, value="en")
        self.speed = ttk.DoubleVar(master=self, value=1.0)
        
        self.setup_ui()

    def setup_ui(self):
        # Input area at top
        input_frame = ttk.Frame(self, padding=10)
        input_frame.pack(fill=X, side=TOP)
        
        ttk.Label(input_frame, text="Text to Speak:", font=("Helvetica", 10, "bold")).pack(anchor=W)
        self.text_input = ttk.Text(input_frame, height=6, font=("Helvetica", 10))
        self.text_input.pack(fill=X, pady=(0, 10))
        
        self.generate_btn = ttk.Button(
            input_frame, 
            text="Generate Speech", 
            command=self.generate_speech, 
            bootstyle="primary"
        )
        self.generate_btn.pack(side=RIGHT)
        
        # TTS Status Area
        self.display_frame = ttk.Frame(self, padding=10, bootstyle="dark")
        self.display_frame.pack(fill=BOTH, expand=True, padx=10, pady=10)
        
        self.status_label = ttk.Label(
            self.display_frame, 
            text="TTS generation status will appear here.\nGenerated speech will be saved as a .wav file.", 
            anchor=CENTER, 
            justify=CENTER,
            font=("Helvetica", 12)
        )
        self.status_label.pack(fill=BOTH, expand=True)

    def setup_sidebar(self, sidebar_frame):
        """Adds TTS-specific controls to the sidebar."""
        ttk.Label(
            sidebar_frame, 
            text="TTS Parameters", 
            font=("Helvetica", 11, "bold")
        ).pack(pady=(20, 10), padx=10, anchor=W)
        
        # Language
        ttk.Label(sidebar_frame, text="Language:").pack(padx=10, anchor=W)
        languages = ["en", "es", "fr", "de", "it", "pt", "pl", "tr", "ru", "nl", "cs", "ar", "zh-cn", "ja", "ko"]
        lang_combo = ttk.Combobox(sidebar_frame, textvariable=self.language, values=languages, state="readonly")
        lang_combo.pack(fill=X, padx=10, pady=(0, 10))
        
        # Speed
        ttk.Label(sidebar_frame, text="Speed:").pack(padx=10, anchor=W)
        speed_frame = ttk.Frame(sidebar_frame)
        speed_frame.pack(fill=X, padx=10, pady=(0, 10))
        ttk.Scale(speed_frame, from_=0.5, to=2.0, variable=self.speed, orient=HORIZONTAL).pack(side=LEFT, fill=X, expand=True)
        
        def update_speed_label(*args):
             speed_val_label.config(text=f"{self.speed.get():.1f}x")
        
        speed_val_label = ttk.Label(speed_frame, text=f"{self.speed.get():.1f}x", width=4)
        speed_val_label.pack(side=RIGHT, padx=(5, 0))
        self.speed.trace_add("write", update_speed_label)
        
        # Refresh Voices Button
        ttk.Button(
            sidebar_frame, 
            text="Refresh Voices", 
            command=self.refresh_voices, 
            bootstyle="outline-secondary"
        ).pack(fill=X, padx=10, pady=(10, 5))
        
        # Voice (Speaker) - dropdown will be populated after loading model
        ttk.Label(sidebar_frame, text="Voice:").pack(padx=10, anchor=W)
        self.voice_combo = ttk.Combobox(sidebar_frame, textvariable=self.voice, state="readonly")
        self.voice_combo.pack(fill=X, padx=10, pady=(0, 10))

    def refresh_voices(self):
        if not self.tts_engine.tts:
             self.status_label.config(text="Loading TTS model to fetch voices...")
             threading.Thread(target=self._load_and_refresh).start()
        else:
             speakers = self.tts_engine.get_speakers()
             if speakers:
                  self.voice_combo.config(values=speakers)
                  self.voice.set(speakers[0])
             else:
                  self.voice_combo.config(values=["Default"])
                  self.voice.set("Default")

    def _load_and_refresh(self):
        self.tts_engine.load_model()
        self.after(0, self.refresh_voices)

    def generate_speech(self):
        text = self.text_input.get("1.0", END).strip()
        if not text:
            return
            
        self.generate_btn.config(state=DISABLED)
        self.status_label.config(text="Generating speech... please wait.")
        
        # Start generation in a separate thread
        thread = threading.Thread(target=self._generate_thread, args=(text,))
        thread.start()

    def _generate_thread(self, text):
        try:
            # Re-load model if needed
            if not self.tts_engine.tts:
                self.tts_engine.load_model()
                
            temp_wav = tempfile.NamedTemporaryFile(suffix=".wav", delete=False)
            output_path = temp_wav.name
            
            # Close temp file so TTS can write to it
            temp_wav.close()
            
            speaker = self.voice.get()
            if speaker == "Default":
                 speaker = None
                 
            success = self.tts_engine.generate(
                text=text,
                speaker=speaker,
                language=self.language.get(),
                output_path=output_path,
                speed=self.speed.get()
            )
            
            if success:
                print(f"Speech saved to {output_path}")
                self.after(0, lambda: self.status_label.config(text=f"Success!\nSpeech saved to:\n{output_path}"))
            else:
                self.after(0, lambda: self.status_label.config(text="Error: Failed to generate speech."))
                
        except Exception as e:
            self.after(0, lambda: self.status_label.config(text=f"Error during generation:\n{str(e)}"))
        finally:
            self.after(0, lambda: self.generate_btn.config(state=NORMAL))
