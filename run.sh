#!/bin/bash

cd ~/fit-ppr-hamilton/out; rm *;touch blank;qrun 5 default ../hamilton.sh;
sleep 2;
watch -n 1 "cat *"
