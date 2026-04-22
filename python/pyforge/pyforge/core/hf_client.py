from huggingface_hub import HfApi, snapshot_download, hf_hub_download
import threading
import os
from pyforge.core.settings import settings

class HFClient:
    def __init__(self):
        self.api = HfApi()
        self.category_mapping = {
            "LLM": "text-generation",
            "Image": "text-to-image",
            "Video": "text-to-video",
            "Audio": "text-to-audio",
            "TTS": "text-to-speech",
            "STT": "automatic-speech-recognition"
        }

    def search_models(self, query=None, category=None, limit=20):
        filter_args = {}
        if category and category in self.category_mapping:
            filter_args["pipeline_tag"] = self.category_mapping[category]
        
        models = self.api.list_models(
            search=query,
            sort="downloads",
            direction=-1,
            limit=limit,
            **filter_args
        )
        
        results = []
        for model in models:
            results.append({
                "id": model.modelId,
                "author": model.author,
                "downloads": model.downloads,
                "likes": model.likes,
                "tags": model.tags,
                "lastModified": model.lastModified,
                "pipeline_tag": model.pipeline_tag,
            })
        return results

    def download_model(self, model_id, callback=None):
        """
        Download a model from HuggingFace.
        If callback is provided, it will be called with (current, total).
        """
        cache_dir = settings.get("model_cache")
        
        def run_download():
            try:
                # For simplicity, we use snapshot_download to get the entire model repo
                snapshot_download(
                    repo_id=model_id,
                    cache_dir=cache_dir,
                    local_dir=os.path.join(cache_dir, model_id.replace("/", "--")),
                    local_dir_use_symlinks=False,
                    # progress_update is handled by huggingface_hub internally with tqdm
                    # to capture it we'd need more complex setup, but snapshot_download
                    # returns the local path when done.
                )
                if callback:
                    callback(100, 100, success=True, model_id=model_id)
            except Exception as e:
                print(f"Error downloading {model_id}: {e}")
                if callback:
                    callback(0, 0, success=False, error=str(e), model_id=model_id)

        thread = threading.Thread(target=run_download)
        thread.start()
        return thread

hf_client = HFClient()
