import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from pyforge.engines.llm_engine import LLMEngine
from pyforge.ui.components.scrollable_text import ScrollableText

class ChatPanel(ttk.Frame):
    def __init__(self, master, llm_engine=None, **kwargs):
        super().__init__(master, **kwargs)
        
        # Initialize or use existing LLM Engine
        self.llm_engine = llm_engine if llm_engine else LLMEngine()
        
        # Settings Variables
        self.temperature = ttk.DoubleVar(master=self, value=0.7)
        self.max_tokens = ttk.IntVar(master=self, value=512)
        
        self.setup_ui()

    def setup_ui(self):
        # Chat history area
        self.history = ScrollableText(self, height=20)
        self.history.pack(fill=BOTH, expand=True, padx=10, pady=10)
        
        # Input area
        self.input_frame = ttk.Frame(self, padding=10)
        self.input_frame.pack(fill=X, side=BOTTOM)
        
        self.input_field = ttk.Entry(self.input_frame)
        self.input_field.pack(side=LEFT, fill=X, expand=True, padx=(0, 10))
        self.input_field.bind("<Return>", lambda event: self.send_message())
        
        self.send_button = ttk.Button(
            self.input_frame, 
            text="Send", 
            command=self.send_message, 
            bootstyle="primary"
        )
        self.send_button.pack(side=RIGHT)

        # Welcome message
        self.history.append_text("System: Welcome to PyForge Chat! Load a model from the Downloads tab to begin.\n\n")

    def setup_sidebar(self, sidebar_frame):
        """Adds LLM-specific controls to the sidebar."""
        # Clear existing sidebar content (excluding the header if it exists)
        # Note: In PyForge, the sidebar header is usually managed by MainWindow, 
        # so we just add our components here.
        
        ttk.Label(
            sidebar_frame, 
            text="LLM Parameters", 
            font=("Helvetica", 11, "bold")
        ).pack(pady=(20, 10), padx=10, anchor=W)
        
        # Temperature
        ttk.Label(sidebar_frame, text="Temperature:").pack(padx=10, anchor=W)
        temp_frame = ttk.Frame(sidebar_frame)
        temp_frame.pack(fill=X, padx=10, pady=(0, 10))
        
        temp_scale = ttk.Scale(
            temp_frame, 
            from_=0.0, 
            to=2.0, 
            variable=self.temperature, 
            orient=HORIZONTAL
        )
        temp_scale.pack(side=LEFT, fill=X, expand=True)
        
        temp_label = ttk.Label(temp_frame, textvariable=self.temperature, width=4)
        temp_label.pack(side=RIGHT, padx=(5, 0))

        # Max Tokens
        ttk.Label(sidebar_frame, text="Max Tokens:").pack(padx=10, anchor=W)
        max_tokens_spin = ttk.Spinbox(
            sidebar_frame, 
            from_=1, 
            to=8192, 
            textvariable=self.max_tokens
        )
        max_tokens_spin.pack(fill=X, padx=10, pady=(0, 20))

    def send_message(self):
        message = self.input_field.get().strip()
        if not message:
            return
            
        # Display user message
        self.history.append_text(f"User: {message}\n\n")
        self.input_field.delete(0, END)
        
        # Handle response
        if not self.llm_engine.model:
            # Mock response if no model is loaded
            response = "System (Mock): No local model is currently loaded. Please download/select a model to get real responses."
        else:
            try:
                # Synchronous generation for simplicity (could be threaded for responsiveness)
                response = self.llm_engine.generate(
                    message, 
                    temperature=self.temperature.get(),
                    max_tokens=self.max_tokens.get()
                )
            except Exception as e:
                response = f"Error generating response: {str(e)}"
            
        self.history.append_text(f"AI: {response}\n\n")
