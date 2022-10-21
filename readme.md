# Cycling Time Predictor

## Summary
This is a mini-program designed to predict the required time to ride a bicycle through some path of varying slopes. It is intended to solve two common problems among current path planners in various map applications:  
1. Cyclists come in all capabilities, shapes and mindsets, so there is no universal agreement upon how fast would someone travel. 
2. These maps do not adjust the estimated times based on uphill / downhills along the way, which can be a significant issue at hilly places.

## Usage
This is a commandline application. The source code can be compiled using the following: 
```
g++ -std=c++2a -o bikesim pathsim.cpp
```
For usage, type something like
```
./bikesim simple.txt 150 90 0.1 -v
```
for a cyclist with an average of $150w$ output, and cyclist + bicycle together weighs $90kg$. For more info, type 
```
./bikesim -h
```
Note that all units are metric.

A sample path data file `simple.txt` is given in the repository. 