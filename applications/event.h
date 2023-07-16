/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-10     jesus.rodriguez       the first version
 */
#ifndef APPLICATIONS_EVENT_H_
#define APPLICATIONS_EVENT_H_

#include <rtdef.h>

#define MAX_DEVICE_NAME_LEN     16

typedef enum
{
    event_type_undefined_e,
    event_type_config_e,
    event_type_credit_e,
    event_type_enable_e,
    event_type_mqtt_e,
    event_type_card_e,
}event_type_t;

typedef struct
{
    char name[MAX_DEVICE_NAME_LEN]; /* name to show */
    rt_uint32_t price;              /* price of credit */
    rt_uint16_t pulses_per_credit;  /* pulses to send per credit */
}config_evt_t;

typedef struct
{
    rt_uint16_t count;  /* total of credits to add */
    rt_uint32_t balance; /* balance of the account (-1 for service) */
}credit_evt_t;

typedef struct
{
    rt_bool_t enable;   /* RT_TRUE -> enable device, RT_FALSE disable device */
}enable_evt_t;

typedef enum
{
    mqtt_event_undefined_e,
    mqtt_event_connecting_e,
    mqtt_event_conn_failed_e,
    mqtt_event_conn_success_e,
}mqtt_event_t;

typedef struct
{
   char uid[32 + 1];    /* null terminated hex string max 32 chars / 16 bytes */
}card_event_t;

typedef struct
{
    event_type_t type;
    union
    {
        config_evt_t config;
        credit_evt_t credit;
        enable_evt_t enable;
        mqtt_event_t mqtt;
        card_event_t card;
    }data;
}event_t;

#endif /* APPLICATIONS_EVENT_H_ */
