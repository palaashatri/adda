#!/bin/bash
for model in $(ollama list | awk 'NR>1 {print $1}'); do
echo "Updating model: $model"
ollama pull "$model"
echo "--"
done
echo "All models updated."
