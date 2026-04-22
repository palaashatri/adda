import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from ttkbootstrap.scrolled import ScrolledText

class ScrollableText(ttk.Frame):
    def __init__(self, master, **kwargs):
        super().__init__(master)
        
        self.text_area = ScrolledText(
            self,
            autohide=True,
            wrap=WORD,
            **kwargs
        )
        self.text_area.pack(fill=BOTH, expand=True)
        
    def append_text(self, text, tag=None):
        self.text_area.text.configure(state=NORMAL)
        self.text_area.text.insert(END, text, tag)
        self.text_area.text.see(END)
        self.text_area.text.configure(state=DISABLED)

    def clear(self):
        self.text_area.text.configure(state=NORMAL)
        self.text_area.text.delete('1.0', END)
        self.text_area.text.configure(state=DISABLED)

    def set_text(self, text):
        self.text_area.text.configure(state=NORMAL)
        self.text_area.text.delete('1.0', END)
        self.text_area.text.insert(END, text)
        self.text_area.text.configure(state=DISABLED)
        self.text_area.text.see(END)
