/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

/**
 * @file qlocx_config.h
 *
 * The config module contains utilities to
 * write and read configurations in the non
 * volatile data storage.
 */

#ifndef QLOCX_CONFIG_H
#define QLOCX_CONFIG_H

#include "qlocx_defines.h"
#include <stdint.h>

void qlocx_config_init();

void qlocx_config_get_date(void*);
void qlocx_config_get_private_key(void*);
void qlocx_config_get_public_key(void*);
void qlocx_config_get_sense_config(void*);
void qlocx_config_get_device_name(void*);
void qlocx_config_set_date(void*);
void qlocx_config_set_sense_config(void*);

#endif

