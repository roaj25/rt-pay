/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-08-23     liYony       first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <stdio.h>
#include "ch32v30x.h"
#include "event_handler.h"
#include "mqtt_link.h"
#include "card_reader.h"
#include "app.h"

int main(void)
{
    rt_bool_t mqtt_link_connected = RT_FALSE;
    event_handler_init();
    mqtt_link_init();
    app_init();
    card_reader_init();
    app_start();
    card_reader_start();
    while (1)
    {
        if (!mqtt_link_connected)
        {
            mqtt_link_connected = mqtt_link_connect();
        }
        /* wait 30 seconds before trying to reconnect */
        rt_thread_mdelay(100);
    }
}
