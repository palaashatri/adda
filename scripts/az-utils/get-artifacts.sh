#!/bin/bash

function usage() {
    echo "Usage: $0 -f <folder_name> -l <url>"
    echo "  -f <folder_name>  Create a folder with the specified name."
    echo "  -l <url>         Download and unzip a file from the specified URL into the folder."
    echo "  -h               Display this help message."
}
# Check if no arguments are passed
if [ $# -eq 0 ]; then
    usage
    exit 1
fi
# Parse command line arguments
while getopts ":f:l:h" opt; do
    case $opt in
        f)
            folder_name="$OPTARG"
            mkdir -p "$folder_name"
            echo "Folder '$folder_name' created."
            ;;
        l)
            url="$OPTARG"
            if [ -z "$folder_name" ]; then
                echo "Error: Folder name is required. Use -f to specify a folder name."
                exit 1
            fi
            zip_file="${url##*/}"
            wget "$url" -O "$zip_file"
            unzip "$zip_file" -d "$folder_name"
            rm "$zip_file"
            echo "Downloaded and unzipped '$zip_file' into '$folder_name'."
            ;;
        h)
            usage
            exit 0
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            usage
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument." >&2
            usage
            exit 1
            ;;
    esac
done
# Check if folder name is provided
if [ -z "$folder_name" ]; then
    echo "Error: Folder name is required. Use -f to specify a folder name."
    exit 1
fi
# Check if URL is provided
if [ -z "$url" ]; then
    echo "Error: URL is required. Use -l to specify a URL."
    exit 1
fi

# if there are any .tgz or .tar.gz files inside the created directory or any of its subdirectories, extract them
find "$folder_name" -type f \( -name "*.tgz" -o -name "*.tar.gz" \) | while read -r file; do
    tar -xzf "$file" -C "$folder_name"
    rm "$file"
done
# if there are any .zip files inside the created directory or any of its subdirectories, extract them
find "$folder_name" -type f -name "*.zip" | while read -r file; do
    unzip "$file" -d "$folder_name"
    rm "$file"
done
# if there are any .tar files inside the created directory or any of its subdirectories, extract them
find "$folder_name" -type f -name "*.tar" | while read -r file; do
    tar -xf "$file" -C "$folder_name"
    rm "$file"
done
