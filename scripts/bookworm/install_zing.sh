#!/bin/bash
set -ex

JDK_VERSION=21.0.0

curl -s https://repos.azul.com/azul-repo.key | sudo gpg --dearmor -o /usr/share/keyrings/azul.gpg
echo "deb [signed-by=/usr/share/keyrings/azul.gpg] https://repos.azul.com/zing/debian bookworm main" | sudo tee /etc/apt/sources.list.d/zing.list
sudo apt update
sudo apt install zing-jdk$JDK_VERSION -y
