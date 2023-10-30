#!/bin/bash

# Check if the arguments are present
if [ $# -ne 2 ]; then
    echo "Error: Arguments missing"
    exit 1
fi

# Get the Arguments
filesdir="$1"
searchstr="$2"

# Step 2, see if filesdir represents a directory on the filesystem
if [ ! -d "$filesdir" ]; then
    echo "Error: The provided directory does not exist"
    exit 1
fi

# Last step of the shell script, use find and grep
matching_lines=0
matching_files=0
while IFS= read -r -d $'\0' file; do
    if grep -q "$searchstr" "$file"; then
        matching_files=$((matching_files + 1))
        matching_lines_in_file=$(grep -c "$searchstr" "$file")
        matching_lines=$((matching_lines + matching_lines_in_file))
    fi
done < <(find "$filesdir" -type f -print0)

# Print the results
echo "The number of files are $matching_files and the number of matching lines are $matching_lines"
