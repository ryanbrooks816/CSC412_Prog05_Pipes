cd Testing
mkdir input_dir output_dir scrap_dir
echo -e "Line one\r\nLine two\r\nLine three" > input_dir/test_file.txt
../Scripts/eolFix.sh input_dir scrap_dir output_dir
cat -e output_dir/test_file.txt | grep -q '\$' && echo "Test passed" || echo "Test failed"
od -c output_dir/test_file.txt
rm -r input_dir output_dir