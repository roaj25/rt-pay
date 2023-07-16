/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-10     jesus.rodriguez       the first version
 */
#include <rtdbg.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <rtdef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "app.h"
#include "lcd.h"
#include "event.h"
#include "event_handler.h"
#include "mqtt_link.h"

#define APP_STACK_SIZE              (1024)
#define APP_THREAD_PRIORITY         (16)        /* lower means higher priority */
#define DEFAULT_PRICE               100
#define DEFAULT_PULSES_PER_CREDIT   1
#define DEFAULT_PULSE_HIGH_TIME     100
#define DEFAULT_PULSE_LOW_TIME      100
#define OUTPUT_PULSE_PIN            "RB0"

typedef struct
{
    char name[MAX_DEVICE_NAME_LEN];
    rt_uint32_t price;
    rt_uint32_t pulses_per_credit;
    rt_bool_t enabled;
    rt_uint32_t pulse_high_time;
    rt_uint32_t pulse_low_time;
    rt_base_t pulse_pin;
}system_config_t;

static event_t evt;
static system_config_t sys_cfg;
static rt_thread_t app_task_handle;

static void app_loop(void *params);
static void on_enable_event(enable_evt_t *evt);
static void on_config_event(config_evt_t *evt);
static void on_credit_event(credit_evt_t *evt);
static void on_mqtt_event(mqtt_event_t evt);
static void start_message(void);
static void idle_message(void);
static void credit_message(rt_uint32_t balance);
static void out_of_service_message(char *reason);
static void connecting_message(void);

void app_init(void)
{
    strncpy(sys_cfg.name,"rt-pay",MAX_DEVICE_NAME_LEN);
    sys_cfg.enabled = RT_FALSE;
    sys_cfg.price = DEFAULT_PRICE;
    sys_cfg.pulses_per_credit = DEFAULT_PULSES_PER_CREDIT;
    sys_cfg.pulse_high_time = DEFAULT_PULSE_HIGH_TIME;
    sys_cfg.pulse_low_time = DEFAULT_PULSE_LOW_TIME;
    sys_cfg.pulse_pin = rt_pin_get(OUTPUT_PULSE_PIN);

    LCD_Init();
    /* start app thread */
    app_task_handle = rt_thread_create("app_thread",
                                        app_loop,
                                        RT_NULL,
                                        APP_STACK_SIZE,
                                        APP_THREAD_PRIORITY,
                                        50);
    RT_ASSERT(app_task_handle);
}

void app_start(void)
{
    rt_thread_startup(app_task_handle);
}

static void app_loop(void *params)
{
    start_message();
    while(1)
    {
        if (RT_TRUE == event_handler_read_event(&evt))
        {
            if (evt.type == event_type_enable_e)
            {
                on_enable_event(&(evt.data.enable));
            }
            else if (evt.type == event_type_config_e)
            {
                on_config_event(&(evt.data.config));
            }
            else if (evt.type == event_type_credit_e)
            {
                on_credit_event(&(evt.data.credit));
            }
            else if (evt.type == event_type_mqtt_e)
            {
                on_mqtt_event(evt.data.mqtt);
            }
        }
    }
}

static void on_enable_event(enable_evt_t *evt)
{
    if (evt->enable != sys_cfg.enabled)
    {
        sys_cfg.enabled = evt->enable;
        if (evt->enable)
        {
            idle_message();
        }
        else
        {
            out_of_service_message("disabled by ownr");
        }
    }
}

static void on_config_event(config_evt_t *evt)
{
    sys_cfg.price = evt->price;
    sys_cfg.pulses_per_credit = evt->pulses_per_credit;
    if (evt->name[0] =! '\n')
    {
        strncpy(sys_cfg.name,evt->name,sizeof(sys_cfg.name));
    }
    /* If device is enabled update the idle screen */
    if (sys_cfg.enabled)
    {
      idle_message();
    }
}

static void add_credit(void)
{
    rt_pin_write(sys_cfg.pulse_pin, PIN_HIGH);
    for (int i=0; i < sys_cfg.pulses_per_credit; i++)
    {
        rt_thread_mdelay(sys_cfg.pulse_high_time);
        rt_pin_write(sys_cfg.pulse_pin, PIN_LOW);
        rt_thread_mdelay(sys_cfg.pulse_high_time);
    }
    rt_pin_write(sys_cfg.pulse_pin, PIN_HIGH);
}

static void on_credit_event(credit_evt_t *evt)
{
    /* dont continue if we are not enabled */
    if (!sys_cfg.enabled)
    {
        return;
    }

    /* insert the pulses */
    for (int i=0; i < evt->count; i++)
    {
        add_credit();
    }

    credit_message(evt->balance);
    rt_thread_mdelay(1500);
    idle_message();
}

static void on_mqtt_event(mqtt_event_t evt)
{
    if (evt == mqtt_event_connecting_e)
    {
        connecting_message();
    }
    else if (evt == mqtt_event_conn_failed_e)
    {
        out_of_service_message("connection error");
    }
    else if (evt == mqtt_event_conn_success_e)
    {
        if (sys_cfg.enabled)
        {
            idle_message();
        }
        else
        {
            out_of_service_message("disabled by ownr");
        }
    }
}

static void start_message(void)
{
                  //0123456789012345
    LCD_Puts(0, 0, " Booting system ");
    LCD_Puts(0, 1, "   Please wait  ");
}

static void idle_message(void)
{
    char line[17];
    snprintf(line,sizeof(line),
            "Price $%02d.%02d",
            (sys_cfg.price / 100),
            (sys_cfg.price % 100));
                  //0123456789012345
    LCD_Puts(0, 0,"                 ");
    LCD_Puts(0, 0,line);
    LCD_Puts(0, 1, "swipe or scan QR");
}

static void credit_message(rt_uint32_t balance)
{
    char line[17];
    snprintf(line,sizeof(line),
            "Balance $%02d.%02d",
            (balance / 100),
            (balance % 100));
                 //0123456789012345
    LCD_Puts(0, 0,"  Thank you!!!  ");
    LCD_Puts(0, 1,"                ");
    LCD_Puts(0, 1,line);

}

static void out_of_service_message(char *reason)
{
                 //0123456789012345
    LCD_Puts(0, 0," Out of service ");
    LCD_Puts(0, 1,reason);
}

static void connecting_message(void)
{
                 //0123456789012345
    LCD_Puts(0, 0,"   connecting   ");
    LCD_Puts(0, 1,"   to  server   ");
}

