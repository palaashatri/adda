import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from PIL import Image, ImageTk
from pyforge.engines.video_engine import VideoEngine
import threading
import os
import tempfile

class VideoPanel(ttk.Frame):
    def __init__(self, master, **kwargs):
        super().__init__(master, **kwargs)
        
        # Initialize Video Engine
        self.video_engine = VideoEngine()
        
        # Settings Variables
        self.frames = ttk.IntVar(master=self, value=16)
        self.fps = ttk.IntVar(master=self, value=8)
        self.steps = ttk.IntVar(master=self, value=30)
        self.guidance_scale = ttk.DoubleVar(master=self, value=7.5)
        self.resolution = ttk.StringVar(master=self, value="512x512")
        self.seed = ttk.StringVar(master=self, value="-1")
        
        self.current_frames = None
        self.tk_image = None # For previewing the first frame
        
        self.setup_ui()

    def setup_ui(self):
        # Input area at top
        input_frame = ttk.Frame(self, padding=10)
        input_frame.pack(fill=X, side=TOP)
        
        ttk.Label(input_frame, text="Video Prompt:", font=("Helvetica", 10, "bold")).pack(anchor=W)
        self.prompt_text = ttk.Text(input_frame, height=4, font=("Helvetica", 10))
        self.prompt_text.pack(fill=X, pady=(0, 10))
        
        self.generate_btn = ttk.Button(
            input_frame, 
            text="Generate Video", 
            command=self.generate_video, 
            bootstyle="primary"
        )
        self.generate_btn.pack(side=RIGHT)
        
        # Video Display Area
        self.display_frame = ttk.Frame(self, padding=10, bootstyle="dark")
        self.display_frame.pack(fill=BOTH, expand=True, padx=10, pady=10)
        
        self.preview_label = ttk.Label(
            self.display_frame, 
            text="Video preview will appear here\n(A representative frame will be shown)", 
            anchor=CENTER, 
            justify=CENTER,
            font=("Helvetica", 12)
        )
        self.preview_label.pack(fill=BOTH, expand=True)

    def setup_sidebar(self, sidebar_frame):
        """Adds Video-specific controls to the sidebar."""
        ttk.Label(
            sidebar_frame, 
            text="Video Parameters", 
            font=("Helvetica", 11, "bold")
        ).pack(pady=(20, 10), padx=10, anchor=W)
        
        # Frames (Duration)
        ttk.Label(sidebar_frame, text="Number of Frames:").pack(padx=10, anchor=W)
        frames_frame = ttk.Frame(sidebar_frame)
        frames_frame.pack(fill=X, padx=10, pady=(0, 10))
        ttk.Scale(frames_frame, from_=8, to=48, variable=self.frames, orient=HORIZONTAL).pack(side=LEFT, fill=X, expand=True)
        ttk.Label(frames_frame, textvariable=self.frames, width=3).pack(side=RIGHT, padx=(5, 0))
        
        # FPS
        ttk.Label(sidebar_frame, text="FPS:").pack(padx=10, anchor=W)
        fps_frame = ttk.Frame(sidebar_frame)
        fps_frame.pack(fill=X, padx=10, pady=(0, 10))
        ttk.Scale(fps_frame, from_=1, to=24, variable=self.fps, orient=HORIZONTAL).pack(side=LEFT, fill=X, expand=True)
        ttk.Label(fps_frame, textvariable=self.fps, width=3).pack(side=RIGHT, padx=(5, 0))
        
        # Steps
        ttk.Label(sidebar_frame, text="Steps:").pack(padx=10, anchor=W)
        steps_frame = ttk.Frame(sidebar_frame)
        steps_frame.pack(fill=X, padx=10, pady=(0, 10))
        ttk.Scale(steps_frame, from_=1, to=100, variable=self.steps, orient=HORIZONTAL).pack(side=LEFT, fill=X, expand=True)
        ttk.Label(steps_frame, textvariable=self.steps, width=3).pack(side=RIGHT, padx=(5, 0))
        
        # Resolution
        ttk.Label(sidebar_frame, text="Resolution:").pack(padx=10, anchor=W)
        res_options = ["256x256", "512x512", "768x768"]
        res_combo = ttk.Combobox(sidebar_frame, textvariable=self.resolution, values=res_options, state="readonly")
        res_combo.pack(fill=X, padx=10, pady=(0, 10))
        
        # Seed
        ttk.Label(sidebar_frame, text="Seed (-1 for random):").pack(padx=10, anchor=W)
        ttk.Entry(sidebar_frame, textvariable=self.seed).pack(fill=X, padx=10, pady=(0, 20))

    def generate_video(self):
        prompt = self.prompt_text.get("1.0", END).strip()
        if not prompt:
            return
            
        self.generate_btn.config(state=DISABLED)
        self.preview_label.config(text="Generating video... please wait. This can take several minutes.")
        
        # Start generation in a separate thread
        thread = threading.Thread(target=self._generate_thread, args=(prompt,))
        thread.start()

    def _generate_thread(self, prompt):
        try:
            # Parse resolution
            res = self.resolution.get().split('x')
            width, height = int(res[0]), int(res[1])
            
            # Parse seed
            try:
                seed_str = self.seed.get().strip()
                seed_val = int(seed_str)
            except ValueError:
                seed_val = -1
                
            frames = self.video_engine.generate(
                prompt=prompt,
                num_frames=self.frames.get(),
                num_inference_steps=self.steps.get(),
                guidance_scale=self.guidance_scale.get(),
                width=width,
                height=height,
                seed=seed_val if seed_val != -1 else None
            )
            
            if frames:
                # Save the video to a temporary file
                temp_video = tempfile.NamedTemporaryFile(suffix=".mp4", delete=False)
                self.video_engine.save_video(frames, temp_video.name, fps=self.fps.get())
                print(f"Video saved to {temp_video.name}")
                
                self.after(0, lambda: self.update_video_display(frames))
            else:
                self.after(0, lambda: self.preview_label.config(text="Error: No video generated.\nEnsure a model is loaded."))
                
        except Exception as e:
            self.after(0, lambda: self.preview_label.config(text=f"Error during generation:\n{str(e)}"))
        finally:
            self.after(0, lambda: self.generate_btn.config(state=NORMAL))

    def update_video_display(self, frames):
        self.current_frames = frames
        # Display the middle frame as a preview
        mid_idx = len(frames) // 2
        pil_image = Image.fromarray(frames[mid_idx])
        
        # Resize to fit preview area
        canvas_width = self.preview_label.winfo_width()
        canvas_height = self.preview_label.winfo_height()
        if canvas_width < 10 or canvas_height < 10:
            canvas_width, canvas_height = 512, 512
            
        img_width, img_height = pil_image.size
        ratio = min(canvas_width / img_width, canvas_height / img_height)
        new_width = max(10, int(img_width * ratio))
        new_height = max(10, int(img_height * ratio))
        
        resized_image = pil_image.resize((new_width, new_height), Image.Resampling.LANCZOS)
        self.tk_image = ImageTk.PhotoImage(resized_image)
        
        self.preview_label.config(image=self.tk_image, text="")
