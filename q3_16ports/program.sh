#!/bin/bash

rm -f build/config.json
time make m

node print_label.js $1
