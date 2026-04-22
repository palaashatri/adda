import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from pyforge.ui.panels.chat_panel import ChatPanel
from pyforge.ui.panels.image_panel import ImagePanel
from pyforge.ui.panels.video_panel import VideoPanel
from pyforge.ui.panels.audio_panel import AudioPanel
from pyforge.ui.panels.tts_panel import TTSPanel
from pyforge.ui.panels.stt_panel import STTPanel
from pyforge.ui.panels.downloads_panel import DownloadsPanel
from pyforge.ui.panels.settings_panel import SettingsPanel
from pyforge.ui.theme import theme_engine
from pyforge.engines.llm_engine import LLMEngine
from pyforge.engines.image_engine import ImageEngine
from pyforge.engines.tts_engine import TTSEngine
from pyforge.engines.stt_engine import STTEngine
from pyforge.core.api_server import APIServer
from pyforge.core.settings import settings

class MainWindow(ttk.Window):
    def __init__(self):
        super().__init__(title="PyForge - Local AI Creative Studio", themename=settings.get("theme"), size=(1200, 800))
        self.panels = {}
        self.active_panel = None
        
        # Initialize Engines
        self.llm_engine = LLMEngine()
        self.image_engine = ImageEngine()
        self.tts_engine = TTSEngine()
        self.stt_engine = STTEngine()
        
        # Initialize API Server
        self.api_server = APIServer(
            llm_engine=self.llm_engine,
            image_engine=self.image_engine,
            tts_engine=self.tts_engine,
            stt_engine=self.stt_engine
        )
        
        self.setup_ui()
        # Default to Chat
        self.on_tab_click("Chat")
        
        # Start API server if enabled in settings
        if settings.get("api_server_enabled"):
            self.api_server.start(port=settings.get("api_port"))

    def setup_ui(self):
        # Top Navigation Bar
        self.nav_frame = ttk.Frame(self, bootstyle="secondary")
        self.nav_frame.pack(side=TOP, fill=X)
        
        self.tabs = ["Chat", "Image", "Video", "Audio", "TTS", "STT", "Downloads", "Settings"]
        self.tab_buttons = {}
        
        for tab in self.tabs:
            btn = ttk.Button(self.nav_frame, text=tab, bootstyle="link-light", command=lambda t=tab: self.on_tab_click(t))
            btn.pack(side=LEFT, padx=5, pady=5)
            self.tab_buttons[tab] = btn

        # Main Layout (Sidebar + Content)
        self.main_container = ttk.Frame(self)
        self.main_container.pack(fill=BOTH, expand=True)

        # Left Sidebar (Contextual)
        self.sidebar = ttk.Frame(self.main_container, width=250, bootstyle="light")
        self.sidebar.pack(side=LEFT, fill=Y)
        
        # Main Content Canvas
        self.content_area = ttk.Frame(self.main_container, bootstyle="default")
        self.content_area.pack(side=LEFT, fill=BOTH, expand=True)

    def on_tab_click(self, tab_name):
        print(f"Switching to tab: {tab_name}")
        
        # Clear sidebar
        for widget in self.sidebar.winfo_children():
            widget.destroy()
        
        # Clear active panel if it exists
        if self.active_panel:
            self.active_panel.pack_forget()

        if tab_name not in self.panels:
            if tab_name == "Chat":
                self.panels[tab_name] = ChatPanel(self.content_area, llm_engine=self.llm_engine)
            elif tab_name == "Image":
                self.panels[tab_name] = ImagePanel(self.content_area, image_engine=self.image_engine)
            elif tab_name == "Video":
                self.panels[tab_name] = VideoPanel(self.content_area)
            elif tab_name == "Audio":
                self.panels[tab_name] = AudioPanel(self.content_area)
            elif tab_name == "TTS":
                self.panels[tab_name] = TTSPanel(self.content_area, tts_engine=self.tts_engine)
            elif tab_name == "STT":
                self.panels[tab_name] = STTPanel(self.content_area, stt_engine=self.stt_engine)
            elif tab_name == "Downloads":
                self.panels[tab_name] = DownloadsPanel(self.content_area)
            elif tab_name == "Settings":
                self.panels[tab_name] = SettingsPanel(self.content_area, api_server=self.api_server)
            else:
                # Placeholder for other panels
                placeholder = ttk.Frame(self.content_area)
                ttk.Label(placeholder, text=f"{tab_name} Panel (Coming Soon)", font=("Helvetica", 14)).pack(pady=20)
                self.panels[tab_name] = placeholder

        self.active_panel = self.panels[tab_name]
        self.active_panel.pack(fill=BOTH, expand=True)
        
        # Setup sidebar if the panel has a setup_sidebar method
        if hasattr(self.active_panel, 'setup_sidebar'):
            self.active_panel.setup_sidebar(self.sidebar)
        else:
            ttk.Label(self.sidebar, text=f"{tab_name} Settings", font=("Helvetica", 12, "bold")).pack(pady=10, padx=10)


if __name__ == "__main__":
    app = MainWindow()
    app.mainloop()
