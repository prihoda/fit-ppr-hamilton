#!/bin/bash
cd ~/fit-ppr-hamilton/parallel
git pull
mpiCC -o ../bin/hamilton main.cpp Graph.cpp MaxHamilton.cpp
