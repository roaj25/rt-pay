/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-15     jesus.rodriguez       the first version
 */
#include <rtthread.h>
#include <rtdef.h>
#include <stdio.h>
#include <stdlib.h>
#include "mqtt_link.h"
#include "event.h"
#include "umqtt.h"
#include "umqtt_internal.h"
#include "jsmn.h"
#include "event_handler.h"

/* defines */
#define DBG_TAG           "mqtt.link"
#define DBG_LVL           DBG_LOG
#include <rtdbg.h>

//#define MQTT_BROKER_URI "tcp://test.mosquitto.org:1883"
#define MQTT_BROKER_URI "tcp://mqtt.eclipseprojects.io:1883"
#define MQTT_PUBLISH_TIMEOUT    100
#define MQTT_TOPIC_ROOT         "rt-pay"
#define DEVICE_UUID             "00000000"
#define MQTT_CONFIG_TOPIC       "config"
#define MQTT_ENABLE_TOPIC       "enable"
#define MQTT_ADD_COINS_TOPIC    "add-credit"
#define MQTT_CARD_SWIPE_TOPIC   "card-swipe"
#define MQTT_MAX_JSON_TOKENS    16

/* static variables */
static umqtt_client_t client = RT_NULL;

/* static function prototypes */
static umqtt_client_t mqtt_start_client(const char *server);
static void mqtt_config_callback(struct umqtt_client *client, void *msg_data);
static void mqtt_add_coins_callback(struct umqtt_client *client, void *msg_data);
static void mqtt_enable_callback(struct umqtt_client *client, void *msg_data);
static int mqtt_subscribe_topics(umqtt_client_t client);

/* public functions */
void mqtt_link_init(void)
{
    /* Placeholder just in case we need a separate init function */
}

rt_bool_t mqtt_link_connect(void)
{
    rt_bool_t result = RT_FALSE;
    /* send a event to inform we are starting to connect */
    mqtt_event_t evt = mqtt_event_connecting_e;
    event_handler_push_event(event_type_mqtt_e, &evt, sizeof(evt));
    client = mqtt_start_client(MQTT_BROKER_URI);

    if (client != RT_NULL)
    {
        evt = mqtt_event_conn_success_e;
        mqtt_subscribe_topics(client);
        result = RT_TRUE;
    }
    else
    {
        LOG_E("failed to start mqtt service");
        evt = mqtt_event_conn_failed_e;
        result = RT_FALSE;
    }
    /* inform the connection result */
    event_handler_push_event(event_type_mqtt_e, &evt, sizeof(evt));
    return result;
}

rt_bool_t mqtt_link_send(mqtt_link_msg_type_t type, void *data, int size)
{
    static char topic[128];

    memset(topic,0x00,sizeof(topic));
    if (type == mqtt_link_mt_card_swipe_e)
    {
        snprintf(topic,sizeof(topic),"%s/%s/%s",MQTT_TOPIC_ROOT,DEVICE_UUID,MQTT_CARD_SWIPE_TOPIC);
    }


    if (strlen(topic) > 0 && client != RT_NULL)
    {
        umqtt_publish(client, UMQTT_QOS1, topic, data, size, MQTT_PUBLISH_TIMEOUT);
    }

}

/* static function definitions */
static int mqtt_event_callback(struct umqtt_client *client, enum umqtt_evt event)
{
    RT_ASSERT(client);

    switch(event)
    {
    case UMQTT_EVT_LINK:
        LOG_D(" %s event - link!", __FUNCTION__);
        break;
    case UMQTT_EVT_ONLINE:
        LOG_D(" %s event - online!", __FUNCTION__);
        break;
    case UMQTT_EVT_OFFLINE:
        LOG_D(" %s event - offline!", __FUNCTION__);
        break;
    case UMQTT_EVT_HEARTBEAT:
        LOG_D(" %s event - heartbeat!", __FUNCTION__);
        break;
    default:
        LOG_D(" unhandled event:%d", event);
        break;
    }

    return 0;
}


static umqtt_client_t mqtt_start_client(const char *uri)
{
    umqtt_client_t client = RT_NULL;
    struct umqtt_info umqtt_info = { 0 };

    umqtt_info.uri = uri;
    LOG_I("%s() -> creating client for uri %s",__FUNCTION__,uri);
    client = umqtt_create(&umqtt_info);

    if (client == RT_NULL)
    {
        LOG_E("%s() -> failed to create client", __FUNCTION__);
        return RT_NULL;
    }

    LOG_I("%s() -> registering mqtt evt callback",__FUNCTION__);
    umqtt_control(client, UMQTT_CMD_EVT_CB, mqtt_event_callback);

    LOG_I("%s() -> starting client ",__FUNCTION__);
    if (umqtt_start(client) < 0)
    {
        LOG_E("%s() -> failed to start client",__FUNCTION__);
        umqtt_delete(client);
        return RT_NULL;
    }

    return client;
}

/* TODO: create a wrapper for jsmn */

static void mqtt_config_callback(struct umqtt_client *client, void *msg_data)
{
    config_evt_t config;
    jsmn_parser parser;
    jsmntok_t tokens[MQTT_MAX_JSON_TOKENS];
    int root_keys = 0;
    struct umqtt_pkgs_publish *msg = (struct umqtt_pkgs_publish *)msg_data;

    RT_ASSERT(client);
    RT_ASSERT(msg_data);

    rt_memset(&config,0x00,sizeof(config));
    jsmn_init(&parser);
    root_keys = jsmn_parse(&parser,
                             msg->payload,
                             msg->payload_len,
                             tokens,
                             MQTT_MAX_JSON_TOKENS);
    if (root_keys < 0)
    {
        LOG_E("failed parsing config msg");
        return;
    }

    for (int i=0; i<root_keys; i++)
    {
        /* we only are interested in keys */
        if (!(tokens[i].type == JSMN_STRING && tokens[i].size > 0))
        {
            continue;
        }

        char *key = (char *)(msg->payload + tokens[i].start);
        int key_len = tokens[i].end - tokens[i].start;
        char *value = (char *)(msg->payload + tokens[i + 1].start);
        int value_len = tokens[i+1].end - tokens[i+1].start;
        /* make sure value is null terminated */
        value[value_len] = '\0';


        if (strncmp(key,"name",key_len) == 0)
        {
            strncpy(config.name,value,value_len);
            LOG_D("name -> %s",value);
        }
        else if (strncmp(key,"price",key_len) == 0)
        {
            config.price = atoi(value);
            LOG_D("price -> %s",value);
        }
        else if (strncmp(key,"pulses",key_len) == 0)
        {
            config.pulses_per_credit = atoi(value);
            LOG_D("pulses -> %s",value);
        }
        else
        {
            LOG_I("unexpected key in config msg [%.*s]",key_len,key);
        }
    }

    event_handler_push_event(event_type_config_e, &config, sizeof(config));
}

static void mqtt_add_coins_callback(struct umqtt_client *client, void *msg_data)
{
    credit_evt_t credit;
    jsmn_parser parser;
    jsmntok_t tokens[MQTT_MAX_JSON_TOKENS];
    int root_keys = 0;
    struct umqtt_pkgs_publish *msg = (struct umqtt_pkgs_publish *)msg_data;

    RT_ASSERT(client);
    RT_ASSERT(msg_data);

    rt_memset(&credit,0x00,sizeof(credit));
    jsmn_init(&parser);
    root_keys = jsmn_parse(&parser,
                             msg->payload,
                             msg->payload_len,
                             tokens,
                             MQTT_MAX_JSON_TOKENS);
    if (root_keys < 0)
    {
        LOG_E("failed parsing config msg");
        return;
    }

    for (int i=0; i<root_keys; i++)
    {
        /* we only are interested in keys */
        if (!(tokens[i].type == JSMN_STRING && tokens[i].size > 0))
        {
            continue;
        }

        char *key = (char *)(msg->payload + tokens[i].start);
        int key_len = tokens[i].end - tokens[i].start;
        char *value = (char *)(msg->payload + tokens[i + 1].start);
        int value_len = tokens[i+1].end - tokens[i+1].start;
        /* make sure value is null terminated */
        value[value_len] = '\0';


        if (strncmp(key,"balance",key_len) == 0)
        {
            credit.balance = atoi(value);
            LOG_D("balance -> %s",value);
        }
        else if (strncmp(key,"count",key_len) == 0)
        {
            credit.count = atoi(value);
            LOG_D("count -> %s",value);
        }
        else
        {
            LOG_I("unexpected key in credit msg [%.*s]",key_len,key);
        }
    }

    event_handler_push_event(event_type_credit_e, &credit, sizeof(credit));
}

static void mqtt_enable_callback(struct umqtt_client *client, void *msg_data)
{
    enable_evt_t enable_msg;
    jsmn_parser parser;
    jsmntok_t tokens[MQTT_MAX_JSON_TOKENS];
    struct umqtt_pkgs_publish *msg = (struct umqtt_pkgs_publish *)msg_data;
    int root_keys = 0;

    RT_ASSERT(client);
    RT_ASSERT(msg_data);

    rt_memset(&enable_msg,0x00,sizeof(enable_msg));
    jsmn_init(&parser);
    root_keys = jsmn_parse(&parser,
                             msg->payload,
                             msg->payload_len,
                             tokens,
                             MQTT_MAX_JSON_TOKENS);
    if (root_keys < 0)
    {
        LOG_E("failed parsing config msg");
        return;
    }

    for (int i=0; i<root_keys; i++)
    {
        /* we only are interested in keys */
        if (!(tokens[i].type == JSMN_STRING && tokens[i].size > 0))
        {
            continue;
        }

        char *key = (char *)(msg->payload + tokens[i].start);
        int key_len = tokens[i].end - tokens[i].start;
        char *value = (char *)(msg->payload + tokens[i + 1].start);
        int value_len = tokens[i+1].end - tokens[i+1].start;
        /* make sure value is null terminated */
        value[value_len] = '\0';


        if (strncmp(key,"enable",key_len) == 0)
        {
            enable_msg.enable = atoi(value);
            LOG_D("enable -> %s",value);
        }
        else
        {
            LOG_I("unexpected key in enable msg [%.*s]",key_len,key);
        }
    }
    event_handler_push_event(event_type_enable_e, &enable_msg, sizeof(enable_msg));
}

static int mqtt_subscribe_topics(umqtt_client_t client)
{
    static char topic[128];

    snprintf(topic,sizeof(topic),"%s/%s/%s",MQTT_TOPIC_ROOT,DEVICE_UUID,MQTT_CONFIG_TOPIC);
    LOG_I("%s() -> registering to topic %s",__FUNCTION__,topic);
    umqtt_subscribe(client, topic, UMQTT_QOS1, mqtt_config_callback);

    snprintf(topic,sizeof(topic),"%s/%s/%s",MQTT_TOPIC_ROOT,DEVICE_UUID,MQTT_ENABLE_TOPIC);
    LOG_I("%s() -> registering to topic %s",__FUNCTION__,topic);
    umqtt_subscribe(client, topic, UMQTT_QOS1, mqtt_enable_callback);

    snprintf(topic,sizeof(topic),"%s/%s/%s",MQTT_TOPIC_ROOT,DEVICE_UUID,MQTT_ADD_COINS_TOPIC);
    LOG_I("%s() -> registering to topic %s",__FUNCTION__,topic);
    umqtt_subscribe(client, topic, UMQTT_QOS2, mqtt_add_coins_callback);

    return 0;
}
