#!/bin/bash

make

echo "=====[ gshare:12 ]====="
./predictor --gshare:12 ../traces/fp_1 
echo ""
./predictor --gshare:12 ../traces/fp_2
echo ""
./predictor --gshare:12 ../traces/int_1
echo ""
./predictor --gshare:12 ../traces/int_2
echo ""
./predictor --gshare:12 ../traces/mm_1
echo ""
./predictor --gshare:12 ../traces/mm_2
echo ""
echo ""

echo "=====[ tournament:12:10:10 ]====="
./predictor --tournament:12:10:10 ../traces/fp_1
echo ""
./predictor --tournament:12:10:10 ../traces/fp_2
echo ""
./predictor --tournament:12:10:10 ../traces/int_1
echo ""
./predictor --tournament:12:10:10 ../traces/int_2
echo ""
./predictor --tournament:12:10:10 ../traces/mm_1
echo ""
./predictor --tournament:12:10:10 ../traces/mm_2
echo ""

echo "=====[ custom ]====="
./predictor --custom ../traces/fp_1
echo ""
./predictor --custom ../traces/fp_2
echo ""
./predictor --custom ../traces/int_1
echo ""
./predictor --custom ../traces/int_2
echo ""
./predictor --custom ../traces/mm_1
echo ""
./predictor --custom ../traces/mm_2
echo ""