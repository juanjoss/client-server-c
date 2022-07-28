#!/bin/bash

for N in {1..1000}
do
    python3 client.py &
done
wait