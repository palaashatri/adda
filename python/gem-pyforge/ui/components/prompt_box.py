import customtkinter as ctk

class PromptBox(ctk.CTkFrame):
    def __init__(self, master, on_submit, **kwargs):
        super().__init__(master, **kwargs)
        self.on_submit = on_submit

        self.grid_columnconfigure(0, weight=1)
        
        self.textbox = ctk.CTkTextbox(self, height=80)
        self.textbox.grid(row=0, column=0, padx=5, pady=5, sticky="ew")
        
        self.submit_btn = ctk.CTkButton(self, text="Generate", command=self._submit)
        self.submit_btn.grid(row=0, column=1, padx=5, pady=5, sticky="e")

    def _submit(self):
        prompt = self.textbox.get("1.0", "end-1c")
        if prompt.strip():
            self.on_submit(prompt)
