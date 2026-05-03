# **INSTRUCTIONS.md — PyForge (Python + Tkinter Edition)**  
*A deterministic blueprint for generating the entire PyForge application from scratch.*

---

# 1. **Project Overview**

PyForge is a **local, offline, monolithic AI studio** built using **Python + Tkinter**, providing:

- Text → Image  
- Image → Image  
- Image Upscaling  
- Text → Video  
- Text → Audio  
- Speech → Text  
- Text → Speech  
- LLM inference  
- Model management + HuggingFace search  
- History gallery with metadata  

The UI is built with **Tkinter + customtkinter**, with automatic dark mode and a minimalist layout.

---

# 2. **Repository Structure**

Copilot must generate code following this exact structure:

```
pyforge/
    app.py
    main.py

    core/
        __init__.py
        config.py
        logger.py
        device.py
        scheduler.py
        task_queue.py

    ui/
        __init__.py
        theme.py
        main_window.py
        tabs/
            __init__.py
            tab_image.py
            tab_video.py
            tab_audio.py
            tab_llm.py
            tab_speech.py
            tab_settings.py
        components/
            __init__.py
            prompt_box.py
            output_viewer.py
            history_panel.py
            model_selector.py
            progress_bar.py

    engines/
        __init__.py
        engine_image.py
        engine_video.py
        engine_audio.py
        engine_llm.py
        engine_speech.py

    models/
        __init__.py
        registry.py
        downloader.py
        metadata.py

    assets/
        icons/
        themes/

    scripts/
        benchmark.py
        cleanup.py

    requirements.txt
    README.md
    LICENSE
```

---

# 3. **Coding Rules (MANDATORY)**

### 3.1 General
- Python 3.10+  
- No external UI frameworks except `tkinter` and `customtkinter`  
- No global state except `config`  
- All engines must expose:

```python
def run(request: EngineRequest) -> EngineResponse:
    ...
```

### 3.2 UI Rules
- Use `grid()` only  
- No absolute positioning  
- All UI components must be reusable classes  
- Dark mode via customtkinter theme  
- Window must be resizable and responsive  

### 3.3 Engine Rules
- Engines must never block the UI thread  
- All heavy tasks must run through `task_queue`  
- Engines must support cancellation  
- Engines must return:

```python
class EngineResponse:
    success: bool
    output: Any
    metadata: dict
    error: Optional[str]
```

### 3.4 Model Registry Rules
- All models stored under:

```
~/.pyforge/models/
```

- Registry must track:
  - name  
  - type  
  - size  
  - local path  
  - HuggingFace ID  
  - version  
  - tags  

---

# 4. **Default Models (One per Modality)**

PyForge must ship with **exactly one default model per modality**:

| Modality | Default Model |
|---------|----------------|
| **Image** | `runwayml/stable-diffusion-v1-5` |
| **Video** | `genmo/mochi-1-preview` |
| **Audio (TTS)** | `suno/bark` |
| **Speech-to-Text** | `openai/whisper-small` or `faster-whisper-small` |

### Default Model Rules
- Download automatically on first launch if missing  
- Store under:

```
~/.pyforge/models/default/<modality>/
```

- Must be pre‑selected in UI  
- User can override with downloaded models  

---

# 5. **Dynamic Model Discovery (HuggingFace Search)**

PyForge must support **searching HuggingFace** for any model.

### Requirements
- Search bar in Model Downloader tab  
- Query HuggingFace API for:
  - model name  
  - tags  
  - size  
  - downloads  
  - last updated  
- Categorize into:
  - image  
  - video  
  - audio  
  - speech  
  - llm  

### Download Rules
- Support:
  - resume  
  - retry  
  - checksum  
  - progress bar  
  - cancellation  
- Store under:

```
~/.pyforge/models/<modality>/<model-name>/
```

### Metadata File
Every downloaded model must generate:

```
model.json
{
  "name": "...",
  "type": "...",
  "hf_id": "...",
  "version": "...",
  "size": ...,
  "tags": [...],
  "local_path": "...",
  "download_date": "..."
}
```

---

# 6. **Model Registry Behavior**

Registry must expose:

```python
def get_default_model(modality: str) -> ModelInfo
def get_installed_models(modality: str) -> list[ModelInfo]
def search_huggingface(query: str, modality: str) -> list[ModelInfo]
def download_model(model: ModelInfo, progress_cb) -> bool
def set_active_model(modality: str, model: ModelInfo)
def get_active_model(modality: str) -> ModelInfo
```

Registry must:
- Track default models separately  
- Allow switching active model  
- Validate model compatibility  
- Cache loaded models  

---

# 7. **UI Specification**

### 7.1 Main Window Layout

```
+-------------------------------------------------------------+
|  Model Selector | Prompt Box | Output Preview               |
|                 |            |                             |
+-------------------------------------------------------------+
|  Settings Panel | Generate Button | History Strip           |
+-------------------------------------------------------------+
```

### 7.2 Tabs
- Image  
- Video  
- Audio  
- LLM  
- Speech  
- Settings  

### 7.3 Model Selector UI
- Dropdown: default + installed models  
- Button: “Search Models…”  
- Toggle: “Set as Default”  

### 7.4 Model Downloader UI
- Search bar  
- Category filter  
- Results list  
- Download button  
- Progress bar  
- Model details panel  

---

# 8. **Engine Specifications**

### 8.1 Image Engine
- Use `diffusers`  
- Support:
  - txt2img  
  - img2img  
  - inpainting  
  - upscaling (Real‑ESRGAN)  

### 8.2 Video Engine
- AnimateDiff  
- Mochi  
- Pyramid Flow  

### 8.3 Audio Engine
- Bark  
- AudioCraft  
- RVC (optional)  

### 8.4 LLM Engine
- llama.cpp (GGUF)  
- transformers (HF models)  

### 8.5 Speech Engine
- Whisper.cpp  
- Faster‑Whisper  

---

# 9. **Task Queue**

Must include:
- ThreadPoolExecutor  
- Queue  
- Cancellation tokens  
- Progress callbacks  
- ETA estimation  

---

# 10. **History System**

Each entry must include:
- preview  
- prompt  
- seed  
- model  
- settings  
- timestamp  

Stored under:

```
~/.pyforge/history/
```

---

# 11. **Startup Behavior**

On first launch:
1. Check if default models exist  
2. If missing → show modal:  
   ```
   PyForge needs to download 4 default models.
   Total size: ~3 GB
   [Download] [Exit]
   ```
3. Download sequentially  
4. Save metadata  
5. Launch UI  

---

# 12. **Packaging Rules**

Generate packaging scripts for:
- Windows `.exe`  
- macOS `.app`  
- Linux `.AppImage`  

---

# 13. **Coding Style**

- PEP8  
- Type hints everywhere  
- Docstrings for every class/function  
- No unused imports  
- No wildcard imports  
- No inline lambdas in UI code  

---

# 14. **Bootstrapping**

`main.py` must:
- initialize config  
- initialize theme  
- create main window  
- load tabs  
- start Tkinter mainloop  

---

# 15. **What Copilot Must Generate**

When this file is present, Copilot must generate:

- all folders  
- all modules  
- all UI classes  
- all engine implementations  
- model registry  
- downloader  
- history system  
- task queue  
- main window  
- packaging scripts  
- README.md  
- requirements.txt  

Everything must be deterministic and follow this blueprint exactly.

