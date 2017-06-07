#!/bin/bash


echo "=====[ GSHARE:4 ]====="
./predictor --gshare:4 ../traces/fp_1
./predictor --gshare:4 ../traces/fp_2
./predictor --gshare:4 ../traces/int_1
./predictor --gshare:4 ../traces/int_2
./predictor --gshare:4 ../traces/mm_1
./predictor --gshare:4 ../traces/mm_1
echo

echo "=====[ GSHARE:8 ]====="
./predictor --gshare:8 ../traces/fp_1
./predictor --gshare:8 ../traces/fp_2
./predictor --gshare:8 ../traces/int_1
./predictor --gshare:8 ../traces/int_2
./predictor --gshare:8 ../traces/mm_1
./predictor --gshare:8 ../traces/mm_1
echo

echo "=====[ GSHARE:16 ]====="
./predictor --gshare:16 ../traces/fp_1
./predictor --gshare:16 ../traces/fp_2
./predictor --gshare:16 ../traces/int_1
./predictor --gshare:16 ../traces/int_2
./predictor --gshare:16 ../traces/mm_1
./predictor --gshare:16 ../traces/mm_1
echo

#echo "=====[ GSHARE:32 ]====="
#./predictor --gshare:32 ../traces/fp_1
#./predictor --gshare:32 ../traces/fp_2
#./predictor --gshare:32 ../traces/int_1
#./predictor --gshare:32 ../traces/int_2
#./predictor --gshare:32 ../traces/mm_1
#./predictor --gshare:32 ../traces/mm_1
#echo
