---

# **GEMINI.MD — PyForge: Local AI Creative Studio**

**Version:** 1.0  
**Author:** Palaash  
**Purpose:** Define the full architecture, UI, workflows, and implementation plan for **PyForge**, a Python‑based, offline‑first AI studio supporting LLMs, text‑to‑image, text‑to‑video, text‑to‑audio, TTS, and STT.

---

# 1. **Vision**
PyForge is a **sleek, minimalist, cross‑platform local AI studio** built in Python.  
It combines the capabilities of:

- **LM Studio** (local LLMs + parameter controls)  
- **DiffusionBee** (image generation)  
- **LTX‑Video / Open-Sora** (video generation)  
- **AudioCraft / MusicGen** (music generation)  
- **XTTS / F5-TTS** (speech synthesis)  
- **Whisper / Distil-Whisper** (speech-to-text)  

All running **locally**, with optional HuggingFace downloads.

---

# 2. **Technology Stack**

### **Language**
- Python 3.11+

### **UI Framework**
- **PyTk** (Tkinter)  
- Custom theming engine for:
  - Auto dark/light mode  
  - Minimalist, responsive layout  
  - Smooth animations (optional via `ttkbootstrap` or custom canvas)

### **AI Runtimes**
- **PyTorch** (primary)
- **Diffusers** (image/video)
- **Transformers** (LLMs)
- **llama.cpp-python** (fast local LLMs)
- **Whisper / faster-whisper** (STT)
- **AudioCraft / MusicGen** (music)
- **XTTS / F5-TTS** (TTS)
- **LTX-Video** (video)

### **Model Management**
- HuggingFace Hub API  
- Local model registry  
- Categorized model browser  
- Download queue + progress bars  

---

# 3. **Application Architecture**

```
PyForge/
 ├── pyforge/
 │    ├── ui/
 │    │    ├── main_window.py
 │    │    ├── theme.py
 │    │    ├── components/
 │    │    └── panels/
 │    ├── core/
 │    │    ├── model_registry.py
 │    │    ├── hf_client.py
 │    │    ├── settings.py
 │    │    ├── gpu_manager.py
 │    │    └── api_server.py
 │    ├── engines/
 │    │    ├── llm_engine.py
 │    │    ├── image_engine.py
 │    │    ├── video_engine.py
 │    │    ├── audio_engine.py
 │    │    ├── tts_engine.py
 │    │    └── stt_engine.py
 │    └── utils/
 ├── assets/
 ├── models/
 ├── requirements.txt
 └── main.py
```

---

# 4. **UI Design Specification**

## **4.1 Design Principles**
- Minimalist  
- Flat, clean surfaces  
- Rounded corners  
- Soft shadows  
- Auto dark/light mode  
- Responsive layout  
- No clutter  

## **4.2 Layout**
### **Top Navigation Bar**
Tabs:
- **Chat**
- **Image**
- **Video**
- **Audio**
- **TTS**
- **STT**
- **Downloads**
- **Settings**

### **Left Sidebar (Contextual)**
Changes depending on active panel:
- LLM parameters  
- Image generation settings  
- Video generation settings  
- Audio/music parameters  
- TTS voice settings  
- STT language settings  

### **Main Canvas**
- Chat window  
- Image preview  
- Video preview  
- Audio waveform  
- Download list  
- Settings dashboard  

---

# 5. **Model Download Manager**

## **5.1 Categories**
PyForge queries HuggingFace and categorizes models:

### **LLMs**
- Llama  
- Qwen  
- Mistral  
- Gemma  
- Phi  
- Others  

### **Image Models**
- Stable Diffusion  
- SDXL  
- Flux  
- ERNIE-Image  
- Kandinsky  

### **Video Models**
- LTX-Video  
- Open-Sora  
- CogVideoX  

### **Audio Models**
- MusicGen  
- AudioCraft  
- Riffusion  

### **TTS Models**
- XTTS  
- F5-TTS  
- Bark  

### **STT Models**
- Whisper  
- Distil-Whisper  

## **5.2 Download Panel Features**
- Search bar  
- Category filters  
- Model cards:
  - Name  
  - Size  
  - Tags  
  - License  
  - GPU requirements  
- Download button  
- Progress bar  
- Pause/resume  
- Cancel  
- Local model folder viewer  

---

# 6. **Engines**

## **6.1 LLM Engine**
Supports:
- llama.cpp models  
- Transformers models  

Features:
- Context length slider  
- Temperature  
- Top‑p  
- Top‑k  
- Max tokens  
- System prompt  
- Stop sequences  
- GPU/CPU toggle  

## **6.2 Image Engine**
Uses:
- Diffusers pipelines  
- SD 1.5 / SDXL / Flux / ERNIE-Image  

Features:
- Prompt  
- Negative prompt  
- Steps  
- Guidance scale  
- Resolution  
- Seed  
- img2img  
- Upscaling  

## **6.3 Video Engine**
Uses:
- LTX-Video  
- CogVideoX  
- Open-Sora  

Features:
- Prompt  
- Duration  
- FPS  
- Motion strength  
- Seed  

## **6.4 Audio Engine (Music)**
Uses:
- MusicGen  
- AudioCraft  

Features:
- Prompt  
- Duration  
- Style presets  

## **6.5 TTS Engine**
Uses:
- XTTS  
- F5-TTS  

Features:
- Voice selection  
- Speed  
- Emotion  
- Output format  

## **6.6 STT Engine**
Uses:
- Whisper  
- faster-whisper  

Features:
- Language  
- Model size  
- Real-time mode  

---

# 7. **Unified Local API**
PyForge exposes an OpenAI‑compatible API:

### **Chat**
`POST /v1/chat/completions`

### **Image**
`POST /v1/images/generations`

### **Audio (TTS)**
`POST /v1/audio/speech`

### **Video**
`POST /v1/video/generations`

### **Speech-to-Text**
`POST /v1/audio/transcriptions`

This allows integration with:
- Cursor  
- VSCode  
- Obsidian  
- Anything that supports OpenAI API  

---

# 8. **Settings Panel**
- Theme (auto / light / dark)  
- GPU selection  
- Model cache location  
- API server toggle  
- Max VRAM usage  
- Logging level  

---

# 9. **Packaging**
Use:
- PyInstaller  
- One-file mode for Windows  
- .app bundle for macOS  
- AppImage for Linux  

---

# 10. **7‑Day Build Plan**

### **Day 1**
- Project skeleton  
- UI framework  
- Theme engine  

### **Day 2**
- LLM engine  
- Chat panel  

### **Day 3**
- Image engine  
- Image panel  

### **Day 4**
- Video engine  
- Video panel  

### **Day 5**
- Audio + TTS + STT engines  
- Panels  

### **Day 6**
- Model download manager  
- HuggingFace integration  

### **Day 7**
- Unified API  
- Packaging  
- Testing  

---

# 11. **Future Extensions**
- Node‑graph editor (ComfyUI‑like)  
- Plugin system  
- Custom C++ UI toolkit migration  
- GPU acceleration via TensorRT  

---

# **End of GEMINI.MD**

---

