#!/bin/bash

rm -f build/config.json
time make m

node syncScript.js $1
