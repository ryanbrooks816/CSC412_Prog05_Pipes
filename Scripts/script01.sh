# Get command line arguments
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <data_folder> <output_file>"
    exit 1
fi

data_folder=$1
output_file=$2

# Check if the data folder exists
if [ ! -d "$data_folder" ]; then
    echo "Data folder does not exist"
    exit 2
fi

# Search through the data folder for all files and use regex to match the first 
# character in the file and keep track of the highest value
highest_process_idx=0
for file in "$data_folder"/*; do
    first_num=$(awk '{print $1; exit}' "$file")
    if [[ "$first_num" =~ ^[0-9]+$ ]] && [ "$first_num" -gt "$highest_process_idx" ]; then
        highest_process_idx=$first_num
    fi
done

echo "Launching server process with $((highest_process_idx + 1)) processes"

# Launch the server process
./Executables/Version\ 1/version1 $highest_process_idx $data_folder $output_file
