# Semi-Reducibility Checker for Multi-Boundary Islands

This repository contains a program for checking the semi-reducibility of multi-boundary islands.
It is used to prove Lemma B.3.

## Requirements

- g++, CMake
- boost, spdlog

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Usage
First, precompute Kempe chain files and coloring files.
The following command generates Kempe chain files up to size 20 and coloring files up to size 18.
```bash
./build/a.out -k 20 -c 18
```

After generating these files, run the following command to check the semi-reducibility of a given multi-boundary island file.
```bash
./build/a.out -i <multi-boundary-island-file>
```

To check the semi-reducibility of all multi-boundary island files in the same directory, run the following command.
```bash
bash testall.sh <multi-boundary-island-directory> <log-output-directory>
```
