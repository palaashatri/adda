import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from PIL import Image, ImageTk
from pyforge.engines.image_engine import ImageEngine
import threading

class ImagePanel(ttk.Frame):
    def __init__(self, master, image_engine=None, **kwargs):
        super().__init__(master, **kwargs)
        
        # Initialize or use existing Image Engine
        self.image_engine = image_engine if image_engine else ImageEngine()
        
        # Settings Variables
        self.steps = ttk.IntVar(master=self, value=30)
        self.guidance_scale = ttk.DoubleVar(master=self, value=7.5)
        self.seed = ttk.StringVar(master=self, value="-1")
        self.resolution = ttk.StringVar(master=self, value="512x512")
        
        self.current_pil_image = None
        self.tk_image = None # Keep reference to avoid garbage collection
        
        self.setup_ui()

    def setup_ui(self):
        # Input area at top
        input_frame = ttk.Frame(self, padding=10)
        input_frame.pack(fill=X, side=TOP)
        
        ttk.Label(input_frame, text="Prompt:", font=("Helvetica", 10, "bold")).pack(anchor=W)
        self.prompt_text = ttk.Text(input_frame, height=3, font=("Helvetica", 10))
        self.prompt_text.pack(fill=X, pady=(0, 10))
        
        ttk.Label(input_frame, text="Negative Prompt:", font=("Helvetica", 10, "bold")).pack(anchor=W)
        self.negative_prompt_text = ttk.Text(input_frame, height=2, font=("Helvetica", 10))
        self.negative_prompt_text.pack(fill=X, pady=(0, 10))
        
        self.generate_btn = ttk.Button(
            input_frame, 
            text="Generate Image", 
            command=self.generate_image, 
            bootstyle="primary"
        )
        self.generate_btn.pack(side=RIGHT)
        
        # Image Display Area
        self.display_frame = ttk.Frame(self, padding=10, bootstyle="dark")
        self.display_frame.pack(fill=BOTH, expand=True, padx=10, pady=10)
        
        self.image_label = ttk.Label(
            self.display_frame, 
            text="Generated image will appear here\n(Ensure a model is loaded)", 
            anchor=CENTER, 
            justify=CENTER,
            font=("Helvetica", 12)
        )
        self.image_label.pack(fill=BOTH, expand=True)
        
        # Bind resize event to update image display
        self.image_label.bind("<Configure>", self.on_resize)

    def setup_sidebar(self, sidebar_frame):
        """Adds Image-specific controls to the sidebar."""
        ttk.Label(
            sidebar_frame, 
            text="Image Parameters", 
            font=("Helvetica", 11, "bold")
        ).pack(pady=(20, 10), padx=10, anchor=W)
        
        # Steps
        ttk.Label(sidebar_frame, text="Steps:").pack(padx=10, anchor=W)
        steps_frame = ttk.Frame(sidebar_frame)
        steps_frame.pack(fill=X, padx=10, pady=(0, 10))
        
        ttk.Scale(
            steps_frame, 
            from_=1, 
            to=100, 
            variable=self.steps, 
            orient=HORIZONTAL
        ) .pack(side=LEFT, fill=X, expand=True)
        
        ttk.Label(steps_frame, textvariable=self.steps, width=3).pack(side=RIGHT, padx=(5, 0))
        
        # Guidance Scale
        ttk.Label(sidebar_frame, text="Guidance Scale:").pack(padx=10, anchor=W)
        gs_frame = ttk.Frame(sidebar_frame)
        gs_frame.pack(fill=X, padx=10, pady=(0, 10))
        
        ttk.Scale(
            gs_frame, 
            from_=1.0, 
            to=20.0, 
            variable=self.guidance_scale, 
            orient=HORIZONTAL
        ).pack(side=LEFT, fill=X, expand=True)
        
        # Custom formatting for guidance scale display
        gs_val_label = ttk.Label(gs_frame, text=f"{self.guidance_scale.get():.1f}", width=4)
        gs_val_label.pack(side=RIGHT, padx=(5, 0))
        
        def update_gs_label(*args):
            gs_val_label.config(text=f"{self.guidance_scale.get():.1f}")
        self.guidance_scale.trace_add("write", update_gs_label)
        
        # Resolution
        ttk.Label(sidebar_frame, text="Resolution:").pack(padx=10, anchor=W)
        res_options = ["256x256", "512x512", "768x768", "1024x1024"]
        res_combo = ttk.Combobox(
            sidebar_frame, 
            textvariable=self.resolution, 
            values=res_options, 
            state="readonly"
        )
        res_combo.pack(fill=X, padx=10, pady=(0, 10))
        
        # Seed
        ttk.Label(sidebar_frame, text="Seed (-1 for random):").pack(padx=10, anchor=W)
        ttk.Entry(sidebar_frame, textvariable=self.seed).pack(fill=X, padx=10, pady=(0, 20))

    def generate_image(self):
        prompt = self.prompt_text.get("1.0", END).strip()
        negative_prompt = self.negative_prompt_text.get("1.0", END).strip()
        
        if not prompt:
            return
            
        self.generate_btn.config(state=DISABLED)
        self.image_label.config(text="Generating image... please wait.")
        
        # Start generation in a separate thread
        thread = threading.Thread(target=self._generate_thread, args=(prompt, negative_prompt))
        thread.start()

    def _generate_thread(self, prompt, negative_prompt):
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
                
            image = self.image_engine.generate(
                prompt=prompt,
                negative_prompt=negative_prompt,
                num_inference_steps=self.steps.get(),
                guidance_scale=self.guidance_scale.get(),
                width=width,
                height=height,
                seed=seed_val if seed_val != -1 else None
            )
            
            if image:
                self.after(0, lambda: self.update_image_display(image))
            else:
                self.after(0, lambda: self.image_label.config(text="Error: No image generated.\nEnsure a model is loaded in the Downloads/Settings tab."))
                
        except Exception as e:
            self.after(0, lambda: self.image_label.config(text=f"Error during generation:\n{str(e)}"))
        finally:
            self.after(0, lambda: self.generate_btn.config(state=NORMAL))

    def update_image_display(self, pil_image):
        self.current_pil_image = pil_image
        self.render_image()

    def on_resize(self, event):
        if self.current_pil_image:
            self.render_image()

    def render_image(self):
        if not self.current_pil_image:
            return
            
        # Get current display area size
        canvas_width = self.image_label.winfo_width()
        canvas_height = self.image_label.winfo_height()
        
        # Use a default size if window is not yet fully rendered
        if canvas_width < 10 or canvas_height < 10:
            canvas_width, canvas_height = 512, 512
            
        # Resize image to fit display area while maintaining aspect ratio
        img_width, img_height = self.current_pil_image.size
        ratio = min(canvas_width / img_width, canvas_height / img_height)
        new_width = max(10, int(img_width * ratio))
        new_height = max(10, int(img_height * ratio))
        
        resized_image = self.current_pil_image.resize((new_width, new_height), Image.Resampling.LANCZOS)
        self.tk_image = ImageTk.PhotoImage(resized_image)
        
        self.image_label.config(image=self.tk_image, text="")
