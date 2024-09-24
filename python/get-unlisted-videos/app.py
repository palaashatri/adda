#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Unlisted YouTube Videos Scraper

This script is designed to scrape unlisted YouTube videos and store their metadata in a CSV file.

Features:
* Retrieves unlisted YouTube videos
* Stores video metadata (title, link, author, upload date, view count) in a CSV file
* Paginates indefinitely to fetch all available results

Requirements:
* Python 3.x
* `pytube` and `youtube-dl` libraries installed (`pip install pytube youtube-dl`)
* YouTube API key (optional but recommended)

Usage:
1. Replace `YOUR_API_KEY_HERE` with your actual YouTube API key, if you have one.
2. Run the script using Python: `python unlisted_videos_scraper.py`
3. The script will generate a CSV file named `unlisted_videos.csv` in the same directory as the script.

Limitations:
* This script is designed to fetch unlisted videos only and may not work with other types of content (e.g., playlists, live streams).
* YouTube's API has limits on the number of requests and data returned, so be mindful of these restrictions when working with large datasets.
* The script may take some time to complete depending on the number of results and your internet connection.

Contributing:
If you'd like to contribute or report issues, please create an issue on GitHub or submit a pull request.

License:
This code is released under the MIT License. See `LICENSE` for details.

Acknowledgments:
Thanks to the `pytube` and `youtube-dl` libraries for making this script possible!

"""

import youtube_dl
from pytube import SearchQuery
import csv

# Set your API key (optional but recommended)
YOUTUBE_API_KEY = "YOUR_API_KEY_HERE"

def get_unlisted_videos():
    query = SearchQuery()
    query.set_video_type('unlisted')
    unlisted_videos = []

    page_num = 1
    while True:
        query.set_page(page_num)
        results = query.all_videos()

        for result in results:
            if result.title:  # Skip videos without titles
                video_data = {
                    'Video ID': result.video_id,
                    'Title': result.title,
                    'Link': f"https://www.youtube.com/watch?v={result.video_id}",
                    'Author': result.author,
                    'Upload Date': result.upload_date,
                    'View Count': result.view_count
                }
                unlisted_videos.append(video_data)

        if len(results) < 100:  # Check if we've reached the end of the results
            break

        page_num += 1

    return unlisted_videos

def write_to_csv(unlisted_videos):
    # Open the CSV file in write mode
    with open('unlisted_videos.csv', 'w', newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=['Video ID', 'Title', 'Link', 'Author', 'Upload Date', 'View Count'])

        # Write the header row
        writer.writeheader()

        # Write each video's data to the CSV file
        for video in unlisted_videos:
            writer.writerow(video)

# Main script
if __name__ == "__main__":
    unlisted_videos = get_unlisted_videos()

    write_to_csv(unlisted_videos)
