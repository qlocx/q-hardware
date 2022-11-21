#!/bin/bash
nrfjprog --recover

rm -f ./build/config.json
make

node ./scripts/sync_config_to_database.js