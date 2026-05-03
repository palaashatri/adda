import customtkinter as ctk
from ui.components.prompt_box import PromptBox
from ui.components.output_viewer import OutputViewer
from ui.components.history_panel import HistoryPanel
from ui.components.model_selector import ModelSelector
from ui.components.progress_bar import ProgressBar
from engines.engine_video import engine_video
from core.scheduler import EngineRequest
from core.task_queue import task_queue

class TabVideo(ctk.CTkFrame):
    def __init__(self, master, **kwargs):
        super().__init__(master, **kwargs)
        
        self.grid_columnconfigure(0, weight=3)
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(1, weight=1)

        self.model_selector = ModelSelector(self, modality="video")
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

        self.prompt_box = PromptBox(left_frame, on_submit=self._generate)
        self.prompt_box.grid(row=2, column=0, sticky="ew")

        self.history = HistoryPanel(self)
        self.history.grid(row=1, column=1, sticky="nsew", padx=(0, 10), pady=5)

    def _generate(self, prompt: str):
        self.viewer.show_text("Generating video (simulated)...")
        self.progress.grid()
        self.progress.set(0)
        
        request = EngineRequest(prompt=prompt, model_id="", modality="video")
        task_queue.submit_task("video_gen", self._run_engine, request)

    def _run_engine(self, request):
        response = engine_video.run(request)
        if response.success:
            self.master.after(0, self._on_success, response.output, request.prompt)
        else:
            self.master.after(0, self._on_error, response.error)

    def _on_success(self, output, prompt):
        self.progress.grid_remove()
        self.viewer.show_text(f"Video saved at: {output}")
        self.history.add_history(f"Video: {prompt[:20]}...")

    def _on_error(self, error):
        self.progress.grid_remove()
        self.viewer.show_text(f"Error: {error}")
