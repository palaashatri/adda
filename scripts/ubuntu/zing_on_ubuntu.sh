#!/bin/bash

echo "============================================================================================================================"
echo "Quick setup script to install Azul Zing JDK 21 (Stream Build) on Ubuntu machine"
echo "Based on instructions published at : https://docs.azul.com/prime/prime-quick-start-apt "
echo "Stream Builds are produced once a month, do not receive any security backports, and are free for development and evaluation."
echo "============================================================================================================================"

set +ex

curl -s https://repos.azul.com/azul-repo.key | sudo gpg --dearmor -o /usr/share/keyrings/azul.gpg
echo "deb [signed-by=/usr/share/keyrings/azul.gpg] https://repos.azul.com/zing/ubuntu $(lsb_release -sc) main" | sudo tee /etc/apt/sources.list.d/zing.list
sudo apt update --allow-unauthenticated --allow-insecure-repositories
sudo apt install zing-jdk21.0.0 -y

echo "============================================================================================================================"
echo "May need to set JAVA_HOME. In case its not set, use : export JAVA_HOME=/opt/zing/zing-<jdk-version>"
echo "============================================================================================================================"
echo "Printing \"java -version\" ... "
java -version
echo "============================================================================================================================"
