/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-16     jesus.rodriguez       the first version
 */
#ifndef APPLICATIONS_CARD_READER_C_
#define APPLICATIONS_CARD_READER_C_

#include <rtthread.h>
#include <rtdef.h>
#include <rtdevice.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mfrc522.h"
#include "event.h"
#include "event_handler.h"
#include "mqtt_link.h"


#define CR_STACK_SIZE          1024
#define CR_THREAD_PRIORITY     18

static rt_thread_t cr_task_handle = RT_NULL;

static void card_reader_loop(void *params);

void card_reader_init(void)
{
    MFRC522(MFRC522_SS_PIN, MFRC522_RST_PIN);
    PCD_Init();     // Init MFRC522
    rt_thread_mdelay(4);

    cr_task_handle = rt_thread_create("app_thread",
                                        card_reader_loop,
                                        RT_NULL,
                                        CR_STACK_SIZE,
                                        CR_THREAD_PRIORITY,
                                        50);
    RT_ASSERT(cr_task_handle);
}

void card_reader_start(void)
{
    rt_thread_startup(cr_task_handle);
}

card_event_t card_evt;
static void card_reader_loop(void *params)
{
    /* get the pointer to the uid of the selected card */
    Uid *uid = get_uid();
    while (1)
    {
        rt_thread_mdelay(500);

        if (PICC_IsNewCardPresent())
        {
            if (PICC_ReadCardSerial())
            {
                char *dst = card_evt.uid;
                int remaining_bytes = sizeof(card_evt.uid);
                for(int i=0; i<uid->size && remaining_bytes > 0; i++)
                {
                    dst += snprintf(dst,remaining_bytes,"%02x",(rt_uint8_t)uid->uidByte[i]);
                    remaining_bytes -= 2;;
                }
                dst = '\0';
                /* push an event so app can handle this later */
                event_handler_push_event(event_type_card_e, &(card_evt), sizeof(card_evt));
                /* send a message to server */
                mqtt_link_send(mqtt_link_mt_card_swipe_e,card_evt.uid,strlen(card_evt.uid));
            }
        }
    }
}

#endif /* APPLICATIONS_CARD_READER_C_ */
