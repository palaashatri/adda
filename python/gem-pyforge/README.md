# PyForge

A local, offline, monolithic AI studio built using Python + Tkinter.

## Modalities
- Text → Image
- Image → Image (Not Fully Implemented in UI)
- Image Upscaling (Not Fully Implemented in UI)
- Text → Video (Simulated)
- Text → Audio
- Speech → Text
- Text → Speech (Not Fully Implemented in UI)
- LLM inference

## Setup

1. Install requirements:
   ```bash
   pip install -r requirements.txt
   ```

2. Run the application:
   ```bash
   python main.py
   ```

On first launch, it will prompt to download default models.

## Packaging
To package PyForge into an executable, you can use PyInstaller:
```bash
pyinstaller --name PyForge --windowed --onefile main.py
```
