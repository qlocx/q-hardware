#!/bin/bash

cd app &&
make &&

cd .. &&

nrfutil pkg generate --hw-version 52 \
--sd-req 0xA8 \
--application-version 4 \
--application app/build/nrf52832_xxaa.hex \
--key-file ./private_key.pem update_dfu_09_q1.zip