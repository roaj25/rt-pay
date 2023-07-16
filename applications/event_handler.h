/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-10     jesus.rodriguez       the first version
 */
#ifndef APPLICATIONS_EVENT_HANDLER_C_
#define APPLICATIONS_EVENT_HANDLER_C_

#include <rtdef.h>
#include "event.h"

void event_handler_init(void);
void event_handler_push_event(event_type_t type, void *data, int size);
rt_bool_t event_handler_read_event(event_t *event);

#endif /* APPLICATIONS_EVENT_HANDLER_C_ */


