#!/bin/bash

find / -not \( -path '/System/Volumes/*' -prune \) \( -iname '*com.gog.*' -o -iname '*gog.com*' \) -prune 2>/dev/null
echo "Successfully removed GOG Galaxy!"
