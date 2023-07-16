/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-15     jesus.rodriguez       the first version
 */
#ifndef APPLICATIONS_MQTT_LINK_H_
#define APPLICATIONS_MQTT_LINK_H_

typedef enum
{
    mqtt_link_mt_undefined_e,
    mqtt_link_mt_card_swipe_e,
}mqtt_link_msg_type_t;

void mqtt_link_init(void);
rt_bool_t mqtt_link_connect(void);
rt_bool_t mqtt_link_send(mqtt_link_msg_type_t type, void *data, int size);

#endif /* APPLICATIONS_MQTT_LINK_H_ */
