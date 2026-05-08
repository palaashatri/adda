import customtkinter as ctk
from core.config import APP_TITLE
from ui.tabs.tab_image import TabImage
from ui.tabs.tab_video import TabVideo
from ui.tabs.tab_audio import TabAudio
from ui.tabs.tab_llm import TabLLM
from ui.tabs.tab_speech import TabSpeech
from ui.tabs.tab_settings import TabSettings

class MainWindow(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title(APP_TITLE)
        self.geometry("1024x768")
        self.minsize(800, 600)
        
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(0, weight=1)

        self.tabview = ctk.CTkTabview(self)
        self.tabview.grid(row=0, column=0, sticky="nsew", padx=10, pady=10)

        # Add tabs
        self.tabview.add("Image")
        self.tabview.add("Video")
        self.tabview.add("Audio")
        self.tabview.add("Speech")
        self.tabview.add("LLM")
        self.tabview.add("Downloader")

        # Configure tab inner grids
        for name in ["Image", "Video", "Audio", "Speech", "LLM", "Downloader"]:
            self.tabview.tab(name).grid_rowconfigure(0, weight=1)
            self.tabview.tab(name).grid_columnconfigure(0, weight=1)

        # Initialize tab contents
        self.tab_image = TabImage(self.tabview.tab("Image"))
        self.tab_image.grid(row=0, column=0, sticky="nsew")

        self.tab_video = TabVideo(self.tabview.tab("Video"))
        self.tab_video.grid(row=0, column=0, sticky="nsew")

        self.tab_audio = TabAudio(self.tabview.tab("Audio"))
        self.tab_audio.grid(row=0, column=0, sticky="nsew")

        self.tab_speech = TabSpeech(self.tabview.tab("Speech"))
        self.tab_speech.grid(row=0, column=0, sticky="nsew")

        self.tab_llm = TabLLM(self.tabview.tab("LLM"))
        self.tab_llm.grid(row=0, column=0, sticky="nsew")

        self.tab_settings = TabSettings(self.tabview.tab("Downloader"))
        self.tab_settings.grid(row=0, column=0, sticky="nsew")
