import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from pyforge.engines.audio_engine import AudioEngine
import threading
import os
import tempfile

class AudioPanel(ttk.Frame):
    def __init__(self, master, **kwargs):
        super().__init__(master, **kwargs)
        
        # Initialize Audio Engine
        self.audio_engine = AudioEngine()
        
        # Settings Variables
        self.duration = ttk.IntVar(master=self, value=10)
        self.model_name = ttk.StringVar(master=self, value="facebook/musicgen-small")
        
        self.setup_ui()

    def setup_ui(self):
        # Input area at top
        input_frame = ttk.Frame(self, padding=10)
        input_frame.pack(fill=X, side=TOP)
        
        ttk.Label(input_frame, text="Music/Audio Prompt:", font=("Helvetica", 10, "bold")).pack(anchor=W)
        self.prompt_text = ttk.Text(input_frame, height=4, font=("Helvetica", 10))
        self.prompt_text.pack(fill=X, pady=(0, 10))
        
        self.generate_btn = ttk.Button(
            input_frame, 
            text="Generate Audio", 
            command=self.generate_audio, 
            bootstyle="primary"
        )
        self.generate_btn.pack(side=RIGHT)
        
        # Audio Status Area
        self.display_frame = ttk.Frame(self, padding=10, bootstyle="dark")
        self.display_frame.pack(fill=BOTH, expand=True, padx=10, pady=10)
        
        self.status_label = ttk.Label(
            self.display_frame, 
            text="Music generation status will appear here.\nGenerated files will be saved to your temp directory.", 
            anchor=CENTER, 
            justify=CENTER,
            font=("Helvetica", 12)
        )
        self.status_label.pack(fill=BOTH, expand=True)

    def setup_sidebar(self, sidebar_frame):
        """Adds Audio-specific controls to the sidebar."""
        ttk.Label(
            sidebar_frame, 
            text="Audio Parameters", 
            font=("Helvetica", 11, "bold")
        ).pack(pady=(20, 10), padx=10, anchor=W)
        
        # Duration
        ttk.Label(sidebar_frame, text="Duration (seconds):").pack(padx=10, anchor=W)
        dur_frame = ttk.Frame(sidebar_frame)
        dur_frame.pack(fill=X, padx=10, pady=(0, 10))
        ttk.Scale(dur_frame, from_=1, to=30, variable=self.duration, orient=HORIZONTAL).pack(side=LEFT, fill=X, expand=True)
        ttk.Label(dur_frame, textvariable=self.duration, width=3).pack(side=RIGHT, padx=(5, 0))
        
        # Model Selection
        ttk.Label(sidebar_frame, text="Model Size:").pack(padx=10, anchor=W)
        models = ["facebook/musicgen-small", "facebook/musicgen-medium", "facebook/musicgen-large"]
        model_combo = ttk.Combobox(sidebar_frame, textvariable=self.model_name, values=models, state="readonly")
        model_combo.pack(fill=X, padx=10, pady=(0, 10))
        
        def on_model_change(*args):
            self.status_label.config(text=f"Note: Model will be loaded on next generation:\n{self.model_name.get()}")
        self.model_name.trace_add("write", on_model_change)

    def generate_audio(self):
        prompt = self.prompt_text.get("1.0", END).strip()
        if not prompt:
            return
            
        self.generate_btn.config(state=DISABLED)
        self.status_label.config(text="Generating audio... please wait.")
        
        # Start generation in a separate thread
        thread = threading.Thread(target=self._generate_thread, args=(prompt,))
        thread.start()

    def _generate_thread(self, prompt):
        try:
            # Re-load model if changed
            if not self.audio_engine.model or self.audio_engine.model_id != self.model_name.get():
                self.audio_engine.load_model(self.model_name.get())
                
            wav = self.audio_engine.generate(
                prompt=prompt,
                duration=self.duration.get()
            )
            
            if wav is not None:
                # Save the audio to a temporary file
                temp_audio = tempfile.NamedTemporaryFile(suffix="", delete=False)
                output_path = temp_audio.name
                success = self.audio_engine.save_audio(wav, output_path)
                
                if success:
                    final_path = output_path + ".wav"
                    print(f"Audio saved to {final_path}")
                    self.after(0, lambda p=final_path: self.status_label.config(text=f"Success!\nAudio saved to:\n{p}"))
                else:
                    self.after(0, lambda: self.status_label.config(text="Error: Failed to save audio file."))
            else:
                self.after(0, lambda: self.status_label.config(text="Error: No audio generated.\nCheck console for details."))
                
        except Exception as e:
            self.after(0, lambda: self.status_label.config(text=f"Error during generation:\n{str(e)}"))
        finally:
            self.after(0, lambda: self.generate_btn.config(state=NORMAL))
