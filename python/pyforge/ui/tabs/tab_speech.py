import customtkinter as ctk
from ui.components.prompt_box import PromptBox
from ui.components.output_viewer import OutputViewer
from ui.components.history_panel import HistoryPanel
from ui.components.model_selector import ModelSelector
from ui.components.progress_bar import ProgressBar
from engines.engine_speech import engine_speech
from core.scheduler import EngineRequest
from core.task_queue import task_queue

class TabSpeech(ctk.CTkFrame):
    def __init__(self, master, **kwargs):
        super().__init__(master, **kwargs)
        
        self.grid_columnconfigure(0, weight=3)
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(1, weight=1)

        self.model_selector = ModelSelector(self, modality="speech")
        self.model_selector.grid(row=0, column=0, columnspan=2, sticky="ew", padx=10, pady=5)

        left_frame = ctk.CTkFrame(self, fg_color="transparent")
        left_frame.grid(row=1, column=0, sticky="nsew", padx=10, pady=5)
        left_frame.grid_rowconfigure(0, weight=1)
        left_frame.grid_columnconfigure(0, weight=1)

        self.viewer = OutputViewer(left_frame)
        self.viewer.grid(row=0, column=0, sticky="nsew", pady=(0, 10))

        self.progress = ProgressBar(left_frame)
        self.progress.grid(row=1, column=0, sticky="ew", pady=(0, 10))
        self.progress.grid_remove()

        # Custom input for audio file path
        self.input_frame = ctk.CTkFrame(left_frame)
        self.input_frame.grid(row=2, column=0, sticky="ew")
        self.input_frame.grid_columnconfigure(0, weight=1)
        
        self.path_entry = ctk.CTkEntry(self.input_frame, placeholder_text="Enter path to audio file...")
        self.path_entry.grid(row=0, column=0, padx=5, pady=5, sticky="ew")
        
        self.btn = ctk.CTkButton(self.input_frame, text="Transcribe", command=self._generate)
        self.btn.grid(row=0, column=1, padx=5, pady=5)

        self.history = HistoryPanel(self)
        self.history.grid(row=1, column=1, sticky="nsew", padx=(0, 10), pady=5)

    def _generate(self):
        prompt = self.path_entry.get()
        if not prompt.strip():
            return
            
        self.viewer.show_text("Transcribing...")
        self.progress.grid()
        self.progress.set(0)
        
        request = EngineRequest(prompt=prompt, model_id="", modality="speech")
        task_queue.submit_task("speech_gen", self._run_engine, request)

    def _run_engine(self, request):
        response = engine_speech.run(request)
        if response.success:
            self.master.after(0, self._on_success, response.output, request.prompt)
        else:
            self.master.after(0, self._on_error, response.error)

    def _on_success(self, output, prompt):
        self.progress.grid_remove()
        self.viewer.show_text(output)
        self.history.add_history(f"Speech: {prompt[:20]}...")

    def _on_error(self, error):
        self.progress.grid_remove()
        self.viewer.show_text(f"Error: {error}")
