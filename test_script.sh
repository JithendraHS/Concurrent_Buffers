#!/bin/bash

make clean
make

./container -h

# Arrays for parameter variations
input_files=("1K_entry.txt")
#"1_entry.txt" "100_entry.txt" "1K_entry.txt" "10K_entry.txt"

num_threads=(4)
#1 2 3 4 8 16

stack_types=("sgl" "treiber" "sgl_elim" "treiber_elim" "stack_flat")
#"sgl" "treiber" "sgl_elim" "treiber_elim" "stack_flat"

queue_types=("sgl" "mns")
#"sgl" "mns"

# Iterate over input files
for input_file in "${input_files[@]}"; do
  # Iterate over thread counts
  for num in "${num_threads[@]}"; do
    # Execute for stack types
    for stack in "${stack_types[@]}"; do
      echo "Running: ./container -i $input_file -o out.txt -t $num --stack=$stack"
      perf stat -e cache-misses -e cache-references -e page-faults -e branch-instructions -e branch-misses ./container -i "$input_file" -o out.txt -t "$num" --stack="$stack"
      
      echo "Sorting out.txt"
      wc -l out.txt
      sed -i '/^0/d' out.txt
      wc -l out.txt
      sort -n out.txt -o out.txt
      
      
     echo "Comparing out.txt with $input_file"
     if diff out.txt "$input_file" > /dev/null; then
        echo "Success: Output matches input"
     else
        echo "Failure: Output does not match input"
     fi
     echo "-----------------------------------------"
     sleep 1
    done

    # Execute for queue types
    for queue in "${queue_types[@]}"; do
      echo "Running: ./container -i $input_file -o out.txt -t $num --queue=$queue"
      perf stat -e cache-misses -e cache-references -e page-faults -e branch-instructions -e branch-misses ./container -i "$input_file" -o out.txt -t "$num" --queue="$queue"
      
      echo "Sorting out.txt"
      wc -l out.txt
      sed -i '/^0/d' out.txt
      wc -l out.txt
      sort -n out.txt -o out.txt
     
      echo "Comparing out.txt with $input_file"
      if diff out.txt "$input_file" > /dev/null; then
        echo "Success: Output matches input"
      else
        echo "Failure: Output does not match input"
      fi
      echo "-----------------------------------------"
      sleep 1
    done
  done
done
