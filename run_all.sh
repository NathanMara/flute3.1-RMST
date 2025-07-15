#!/bin/bash

for infile in input/*.txt; do
  base=$(basename "$infile" .txt)

  outfile="output/${base}_out.txt"
  imgfile="image/${base}.png"

  echo "Processing $infile"
  echo "---------------------Creating Output $outfile---------------------"

  start=$(date +%s.%N)
  ./bin/steiner "$infile" "$outfile"
  end=$(date +%s.%N)
  elapsed=$(echo "$end - $start" | bc)
  printf "Time taken: %.3f seconds\n" "$elapsed"

  echo "---------------------Running Checker---------------------"
  ./checker/checker "$infile" "$outfile"

  echo "---------------------Creating Img $imgfile---------------------"
  python3 ./plot/steiner_plot.py -i "$infile" -o "$outfile" -p "$imgfile" > /dev/null 2>&1
done
