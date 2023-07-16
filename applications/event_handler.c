/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-10     jesus.rodriguez       the first version
 */
#include "event_handler.h"
#include <rtthread.h>
#include <rtdef.h>
#include <rtdevice.h>
#include <rtdbg.h>
#include <string.h>

#define EVT_QUEUE_LEN 4
static rt_mq_t evt_queue = RT_NULL;

void event_handler_init(void)
{
    evt_queue = rt_mq_create("evt_queue",
                            sizeof(event_t),
                            EVT_QUEUE_LEN,
                            RT_IPC_FLAG_FIFO);
    RT_ASSERT(evt_queue);
}

void event_handler_push_event(event_type_t type, void *data, int size)
{
    event_t evt;
    evt.type = type;
    memcpy(&(evt.data), data, size);
    rt_mq_send(evt_queue, &evt, sizeof(evt));
}

rt_bool_t event_handler_read_event(event_t *event)
{
    return rt_mq_recv(evt_queue, event, sizeof(event_t), RT_TICK_MAX) > 0;
}
