from fastapi import FastAPI, HTTPException, Body, File, UploadFile, Form
from pydantic import BaseModel
from typing import List, Optional, Union, Dict, Any
import uvicorn
import threading
import time
import os
import base64
from io import BytesIO

class ChatCompletionMessage(BaseModel):
    role: str
    content: str

class ChatCompletionRequest(BaseModel):
    model: str
    messages: List[ChatCompletionMessage]
    temperature: Optional[float] = 0.7
    top_p: Optional[float] = 0.9
    max_tokens: Optional[int] = 512
    stop: Optional[Union[str, List[str]]] = None
    stream: Optional[bool] = False

class ImageGenerationRequest(BaseModel):
    prompt: str
    model: Optional[str] = None
    n: Optional[int] = 1
    size: Optional[str] = "512x512"
    response_format: Optional[str] = "url"

class AudioSpeechRequest(BaseModel):
    model: str
    input: str
    voice: Optional[str] = "alloy"
    response_format: Optional[str] = "mp3"
    speed: Optional[float] = 1.0

class APIServer:
    def __init__(self, llm_engine=None, image_engine=None, tts_engine=None, stt_engine=None):
        self.app = FastAPI(title="PyForge API")
        self.llm_engine = llm_engine
        self.image_engine = image_engine
        self.tts_engine = tts_engine
        self.stt_engine = stt_engine
        self.server_thread = None
        self.should_stop = False
        self._setup_routes()

    def _setup_routes(self):
        @self.app.post("/v1/chat/completions")
        async def chat_completions(request: ChatCompletionRequest):
            if not self.llm_engine:
                raise HTTPException(status_code=503, detail="LLM Engine not initialized")
            
            # Combine messages into a single prompt for llama.cpp if needed, 
            # or use a more sophisticated template. For now, simple concat.
            prompt = ""
            for msg in request.messages:
                prompt += f"{msg.role}: {msg.content}\n"
            prompt += "assistant: "
            
            response_text = self.llm_engine.generate(
                prompt,
                temperature=request.temperature,
                top_p=request.top_p,
                max_tokens=request.max_tokens,
                stop=request.stop
            )
            
            return {
                "id": "chatcmpl-" + str(int(time.time())),
                "object": "chat.completion",
                "created": int(time.time()),
                "model": request.model,
                "choices": [
                    {
                        "index": 0,
                        "message": {
                            "role": "assistant",
                            "content": response_text
                        },
                        "finish_reason": "stop"
                    }
                ],
                "usage": {
                    "prompt_tokens": -1,
                    "completion_tokens": -1,
                    "total_tokens": -1
                }
            }

        @self.app.post("/v1/images/generations")
        async def image_generations(request: ImageGenerationRequest):
            if not self.image_engine:
                raise HTTPException(status_code=503, detail="Image Engine not initialized")
            
            # Parse size
            width, height = 512, 512
            if request.size:
                try:
                    width, height = map(int, request.size.split('x'))
                except:
                    pass
            
            image = self.image_engine.generate(
                prompt=request.prompt,
                width=width,
                height=height
            )
            
            if not image:
                raise HTTPException(status_code=500, detail="Image generation failed")
            
            # Convert to base64
            buffered = BytesIO()
            image.save(buffered, format="PNG")
            img_str = base64.b64encode(buffered.getvalue()).decode()
            
            return {
                "created": int(time.time()),
                "data": [
                    {
                        "b64_json": img_str
                    }
                ]
            }

        @self.app.post("/v1/audio/speech")
        async def audio_speech(request: AudioSpeechRequest):
            if not self.tts_engine:
                raise HTTPException(status_code=503, detail="TTS Engine not initialized")
            
            temp_file = "api_output.wav"
            success = self.tts_engine.generate(
                text=request.input,
                speaker=request.voice,
                speed=request.speed,
                output_path=temp_file
            )
            
            if not success or not os.path.exists(temp_file):
                raise HTTPException(status_code=500, detail="Speech generation failed")
            
            with open(temp_file, "rb") as f:
                audio_data = f.read()
            
            # In a real OpenAI API, this returns the audio file directly.
            # For simplicity in this mock-compatible API, we'll return it as a response.
            from fastapi.responses import Response
            return Response(content=audio_data, media_type="audio/wav")

        @self.app.post("/v1/audio/transcriptions")
        async def audio_transcriptions(file: UploadFile = File(...), model: str = Form("whisper-1")):
            if not self.stt_engine:
                raise HTTPException(status_code=503, detail="STT Engine not initialized")
            
            # Save uploaded file to a temporary location
            import tempfile
            with tempfile.NamedTemporaryFile(delete=False, suffix=os.path.splitext(file.filename)[1]) as tmp:
                tmp.write(await file.read())
                tmp_path = tmp.name
            
            try:
                text = self.stt_engine.transcribe(tmp_path)
                return {"text": text}
            finally:
                if os.path.exists(tmp_path):
                    os.remove(tmp_path)

    def start(self, host="0.0.0.0", port=8000):
        if self.server_thread and self.server_thread.is_alive():
            return False
            
        def run_server():
            config = uvicorn.Config(self.app, host=host, port=port, log_level="info")
            server = uvicorn.Server(config)
            server.run()

        self.server_thread = threading.Thread(target=run_server, daemon=True)
        self.server_thread.start()
        return True

    def stop(self):
        # Uvicorn doesn't provide a simple way to stop from another thread easily without more ceremony
        # For this prototype, we'll just let it run or suggest a restart of the app.
        # Real implementation would use a signal or a more controllable server instance.
        pass
