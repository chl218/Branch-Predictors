#!/bin/bash

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