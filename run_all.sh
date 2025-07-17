#!/bin/bash

for infile in input/*.txt; do
  base=$(basename "$infile" .txt)

  outfile="output/${base}_out.txt"
  imgfile="image/${base}.png"

  echo "---------------------Processing $infile---------------------"

  start=$(date +%s.%N)
  ./bin/steiner "$infile" "$outfile"
  end=$(date +%s.%N)
  elapsed=$(echo "$end - $start" | bc)
  printf "Time taken: %.3f seconds\n" "$elapsed"

  ./checker/checker "$infile" "$outfile" 2>&1 | grep "^\[Checker\]"

  python3 ./plot/steiner_plot.py -i "$infile" -o "$outfile" -p "$imgfile" > /dev/null 2>&1
  echo "$imgfile"
done
