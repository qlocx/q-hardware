/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

/**
 * @file qlocx_api.h
 *
 * The API module handles the requests recieved
 * from remote via BLE.
 */

#ifndef QLOCX_API_H
#define QLOCX_API_H

#include <stdint.h>
#include "qlocx_ble_service.h"

#define QLOCX_API_OPEN_PORT             0x01
#define QLOCX_API_GET_PORT_STATUS       0x02
#define QLOCX_API_SET_SENSE_CONFIG      0x03
#define QLOCX_API_GET_LOGS              0x04
#define QLOCX_API_PURGE_LOGS            0x05
#define QLOCX_API_GET_BATTERY_STATUS    0x06
#define QLOCX_API_GET_DATE              0x07
#define QLOCX_API_GET_SOFTWARE_VERSION  0x08

void qlocx_api_init();

bool qlocx_api_open_port(qlocx_ble_t *context);
bool qlocx_api_get_port_status(qlocx_ble_t *context);
bool qlocx_api_set_sense_config(qlocx_ble_t *context);
bool qlocx_api_get_logs(qlocx_ble_t *context);
bool qlocx_api_get_battery_status(qlocx_ble_t *context);
bool qlocx_api_purge_logs(qlocx_ble_t *context);
bool qlocx_api_get_date(qlocx_ble_t *context);
bool qlocx_api_get_software_version(qlocx_ble_t *context);

#endif
