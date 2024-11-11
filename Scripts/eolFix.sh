#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <input_directory> <scrap_directory> <output_directory>"
    exit 1
fi

input_directory=$1
scrap_directory=$2
output_directory=$3

# Make sure the input directory exists
if [ ! -d "$input_directory" ]; then
    echo "Input directory with data files does not exist."
    exit 1
fi

# Create the scrap and output directories if they do not exist
if [ ! -d "$scrap_directory" ]; then
    mkdir -p "$scrap_directory"
fi

if [ ! -d "$output_directory" ]; then
    mkdir -p "$output_directory"
fi

# Process each file in the input directory and convert to Unix line endings
for file in "$input_directory"/*; do
    if [ -f "$file" ]; then
        filename=$(basename "$file")
        scrap_file="$scrap_directory/$filename"
        output_file="$output_directory/$filename"

        # Read line by line until the end of the file avoiding stripping of leading/trailing whitespaces
        while IFS= read -r line || [ -n "$line" ]; do
            # Remove trailing CR if present
            line="${line%$'\r'}"
            # Output each line to the scrap file with LF ending
            echo "$line" >> "$scrap_file"
        done < "$file"

        # Move the fixed file to the output directory
        mv "$scrap_file" "$output_file"
    fi
done

# Delete the scrap directory
rm -r "$scrap_directory"
