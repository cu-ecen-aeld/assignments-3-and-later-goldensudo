#!/bin/bash

if [ $# -ne 2 ]; then 
    echo "Error: you must provide two arguments."
    exit 1
fi

# Extract arguments
writefile="$1"
writestr="$2"

# Check if the destination directory exists, and if not, create it
destination_dir="$(dirname "$writefile")"
if [ ! -d "$destination_dir" ]; then
    mkdir -p "$destination_dir"
    if [ $? -ne 0 ]; then 
        echo "Error: Failed to create the destination directory"
        exit 1
    fi
fi

# Write the content to the specified file
echo -n "$writestr" > "$writefile"
if [ $? -ne 0 ]; then
    echo "Error: Failed to write to the file."
    exit 1
fi

echo "File '$writefile' created with content '$writestr'"
