/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-22     flybreak     Make the first version of pcf8574's package
 */

#include <rtthread.h>
#include "pcf8574.h"

#define TEST_IO    0 //(0-7)
#define I2C_BUS    "i2c1"
//#define I2C_ADDRESS 0x27
#define I2C_ADDRESS 0x3F
static pcf8574_device_t dev = RT_NULL;

int pcf8574_sample(void)
{
    rt_uint8_t value;
    if (dev == RT_NULL)
    {
        dev = pcf8574_init(I2C_BUS, I2C_ADDRESS);
    }

    if (dev == RT_NULL)
        return -1;

    pcf8574_pin_write(dev, TEST_IO, 0);

    value = pcf8574_pin_read(dev, TEST_IO);
    rt_kprintf("The value of pcf8574.P%d is %d\n", TEST_IO, value);

    rt_thread_mdelay(1000);

    pcf8574_pin_write(dev, TEST_IO, 1);

    value = pcf8574_pin_read(dev, TEST_IO);
    rt_kprintf("The value of pcf8574.P%d is %d\n", TEST_IO, value);

    pcf8574_deinit(dev);

    return 0;
}

int pcf8574_write_pin(int argc, char *argv[])
{
    if (argc != 3)
    {
        rt_kprintf("usage: %s pin value\n",__FUNCTION__);
        return 0;
    }

    if (dev == RT_NULL)
    {
        dev = pcf8574_init(I2C_BUS, I2C_ADDRESS);
    }

    if (dev == RT_NULL)
    {
        return -1;
    }

    rt_uint8_t pin = atoi(argv[1]);
    rt_uint8_t value = atoi(argv[2]);

    rt_kprintf("writing %d to pin %d\n",pin,value);
    pcf8574_pin_write(dev, pin, value);
    return 0;
}

int pcf8574_read_pin(int argc, char *argv[])
{
    if (argc != 2)
    {
        rt_kprintf("usage: %s pin \n",__FUNCTION__);
        return 0;
    }

    if (dev == RT_NULL)
    {
        dev = pcf8574_init(I2C_BUS, I2C_ADDRESS);
    }

    if (dev == RT_NULL)
        return -1;
    uint8_t pin = atoi(argv[2]);
    uint8_t value = pcf8574_pin_read(dev, pin);

    rt_kprintf("read %d from pin %d\n",pin,value);
    return 0;
}

static char *conver_to_binary(rt_uint8_t value)
{
    static char buffer[9];
    char *p = buffer;

    for(int i=0; i<(sizeof(value)*8); i++)
    {
        rt_uint8_t mask = 1 << (7-i);
        *p++ = (value & mask) ? '1' : '0';
    }
    *p = '\0';
    return buffer;
}


int pcf8574_read_port(int argc, char *argv[])
{
    if (argc != 1)
    {
        rt_kprintf("usage: %s\n",__FUNCTION__);
        return 0;
    }

    if (dev == RT_NULL)
    {
        dev = pcf8574_init(I2C_BUS, I2C_ADDRESS);
    }

    if (dev == RT_NULL)
        return -1;
    uint8_t value = pcf8574_port_read(dev);

    rt_kprintf("read  from pin %s\n",conver_to_binary(value));
    return 0;
}


//int i2c_scan(int argc, char *argv[])
//{
//    struct rt_i2c_bus_device *dev = RT_NULL;
//    const char *bus = argv[1];
//
//    if (argc != 2)
//    {
//        rt_kprintf("usage: %s bus\n",__FUNCTION__);
//        return -1;
//    }
//
//    dev = (struct rt_i2c_bus_device *)rt_device_find(bus);
//    if (dev == RT_NULL)
//    {
//        rt_kprintf("can't open bus [%s]\n", argv[1]);
//        return -1;
//    }
//
//    struct rt_i2c_msg msgs;
//    msgs.addr = I2C_ADDRESS;
//    msgs.flags = RT_I2C_WR;
//    msgs.buf = RT_NULL;
//    msgs.len = 0;
//
//    rt_kprintf("scanning bus %s\n", argv[1]);
//    rt_kprintf("   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
//    /* Call the I2C device interface to transfer data */
//    for(int i=0; i <= 127; i++)
//    {
//        if ( (i%16) == 0)
//        {
//            rt_kprintf("\n%02X",(rt_uint8_t)i);
//        }
//
//        msgs.addr = i;
//
//        if (rt_i2c_transfer(dev, &msgs, 1) == 1)
//        {
//            rt_kprintf(" %02X", (rt_uint8_t)i);
//        }
//        else
//        {
//            rt_kprintf(" --");
//        }
//    }
//    rt_kputs("\n");
//    return 0;
//}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(pcf8574_read_pin, read_pin, pcf8574 read value of a pin);
MSH_CMD_EXPORT_ALIAS(pcf8574_write_pin, write_pin, pcf8574 write value to a pin);
MSH_CMD_EXPORT_ALIAS(pcf8574_read_port, read_port, pcf8574 read port value);
//MSH_CMD_EXPORT_ALIAS(i2c_scan, i2c_scan, scan devices);
#endif
