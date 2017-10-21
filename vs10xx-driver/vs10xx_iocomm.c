/* ------------------------------------------------------------------------------------------------------------------------------
    vs10xx io functions.
    Copyright (C) 2017 Rajiv Biswas <rajiv.biswas55@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
   ----------------------------------------------------------------------------------------------------------------------------- */

#include "vs10xx_common.h"
#include "vs10xx_iocomm.h"
#include "vs10xx_rpi.h"

#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>

#include <linux/module.h>   
#include <linux/string.h>    
#include <linux/fs.h>      
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

#include <linux/list.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/time.h>

#include <linux/of_address.h>
#include <linux/of.h>

#include <linux/moduleparam.h>

/* interrupt mode */
static int irqmode = 1;
module_param(irqmode, int, 0644);

/* dreq status */
//static int skipdreq = 0;
static int skipdreq = 1;
module_param(skipdreq, int, 0644);

/* hwreset */
static int hwreset = 0;
module_param(hwreset, int, 0644);

struct vs10xx_chip {
	int 		    dreq_val;
	int 	            gpio_reset;
	int 		    gpio_dreq;
	int 		    irq_dreq;

	struct spi_message  msg;
	struct spi_transfer transfer[2];
	struct spi_device  *spi_ctrl;
	struct spi_device  *spi_data;

	unsigned char tx_buf[32] ____cacheline_aligned;
	unsigned char rx_buf[2];
	wait_queue_head_t   wq;
};

enum	{
	vs1001=0,
	vs1011=1,
	vs1002=2,
	vs1003=3,
	vs1053=4,
	vs1033=5,
	vs1103=7
};

static struct vs10xx_chip vs10xx_chips[VS10XX_MAX_DEVICES];

/* ----------------------------------------------------------------------------------------------------------------------------- */
/* VS10XX SPI MISC                                                                                                               */
/* ----------------------------------------------------------------------------------------------------------------------------- */

static irqreturn_t vs10xx_io_irq(int irq, void *arg) {
/*
 *  Descr:  interrupt handler, record DREQ value and wakeup waiting task
 *  Return: irq handled
 */

	int id = (int)arg;

	vs10xx_chips[id].dreq_val = (GPIO_READ(vs10xx_chips[id].gpio_dreq) == 0) ? 0 : 1;//gpio_get_value(vs10xx_chips[id].gpio_dreq);
	//printk("Interrupt handler, DREQ val: 0x%x.\n", vs10xx_chips[id].dreq_val);
	if (vs10xx_chips[id].dreq_val) {
		wake_up(&vs10xx_chips[id].wq);
	}

	return IRQ_HANDLED;
}

int vs10xx_io_isready(int id) {
/*
 *  Descr:  Test if DREQ is high
 *  Return: 1 --> DREQ is high (ready)
 *          0 --> DREQ is low (busy)
 */
	//int dreq_Rval= GPIO_READ(vs10xx_chips[id].gpio_dreq);
	bool dreq_Rval= (GPIO_READ(vs10xx_chips[id].gpio_dreq) == 0) ? 0 : 1;
	//int dreq_val = ( irqmode ? vs10xx_chips[id].dreq_val : dreq_Rval );
	//int dreq_val = (irqmode ? vs10xx_chips[id].dreq_val : gpio_get_value(vs10xx_chips[id].gpio_dreq));

	//vs10xx_nsy("id:%d dreq_val:%d", id, dreq_val);

	//return (skipdreq ? 1 : dreq_val);
	vs10xx_nsy("id:%d dreq_val:%d", id, dreq_Rval);
	return (skipdreq ? 1 : dreq_Rval);
}

int vs10xx_io_wtready(int id, unsigned timeout) {
/*
 *  Descr:  Wait timeout [msec] for DREQ to become high
 *  Return: 1 --> DREQ is high (ready)
 *          0 --> timeout (busy)
 */

	if (irqmode) {

		if (!vs10xx_chips[id].dreq_val) {

			wait_event_timeout(vs10xx_chips[id].wq, vs10xx_chips[id].dreq_val, msecs_to_jiffies(timeout));
		}

	} else {

		unsigned long end = jiffies + msecs_to_jiffies(timeout);

		while ( !vs10xx_io_isready(id) && (jiffies < end) ) {

			msleep(1);
		}
	}

	//return vs10xx_io_isready(id);
	return vs10xx_chips[id].dreq_val;
}

void vs10xx_io_reset(int id) {
/*
 *  Descr:  Reset by pulling down gpio
 *  Return: -
 */

	if (hwreset) {
		GPIO_CLR = 1 << (vs10xx_chips[id].gpio_reset);

		udelay(50);
		GPIO_SET = 1 << (vs10xx_chips[id].gpio_reset);

		udelay(1800);
	}
}

/* ----------------------------------------------------------------------------------------------------------------------------- */
/* VS10XX SPI TRANSFER                                                                                                           */
/* ----------------------------------------------------------------------------------------------------------------------------- */

int vs10xx_io_ctrl_xf(int id, const char *txbuf, unsigned txlen, char *rxbuf, unsigned rxlen) {

	int status = 0;
	
	vs10xx_chips[id].transfer[0].tx_buf = vs10xx_chips[id].tx_buf;
	vs10xx_chips[id].transfer[0].len    = sizeof(vs10xx_chips[id].tx_buf);
	vs10xx_chips[id].transfer[1].rx_buf = vs10xx_chips[id].rx_buf;
	vs10xx_chips[id].transfer[1].len    = sizeof(vs10xx_chips[id].rx_buf);

	spi_message_init_with_transfers(&vs10xx_chips[id].msg, vs10xx_chips[id].transfer, ARRAY_SIZE(vs10xx_chips[id].transfer));

	if (txbuf && (txbuf[0] == 0x02)) {
	vs10xx_chips[id].tx_buf[0/*3*/] = txbuf[0];
	vs10xx_chips[id].tx_buf[1/*2*/] = txbuf[1];
	vs10xx_chips[id].tx_buf[2/*1*/] = txbuf[2];
	vs10xx_chips[id].tx_buf[3/*0*/] = txbuf[3];
	vs10xx_chips[id].transfer[0].len= 0x04;
	spi_message_add_tail(&vs10xx_chips[id].transfer[0], &vs10xx_chips[id].msg);
	status = spi_sync(vs10xx_chips[id].spi_ctrl, &vs10xx_chips[id].msg);
	if (status < 0)
		printk("id:%d spi_sync failed in Tx\n", id);
	}

	if (txbuf && (txbuf[0] == 0x03)) {
	vs10xx_chips[id].tx_buf[0] = txbuf[0];
	vs10xx_chips[id].tx_buf[1] = txbuf[1];
	vs10xx_chips[id].transfer[0].len = 0x02;
	if (status < 0)
		printk("id:%d spi_sync failed in Tx\n", id);
	}

	if (rxbuf) {
	vs10xx_chips[id].rx_buf[0] = rxbuf[0];
	vs10xx_chips[id].rx_buf[1] = rxbuf[1];
	status = spi_sync(vs10xx_chips[id].spi_ctrl, &vs10xx_chips[id].msg);
	if (status < 0)
		printk("id:%d spi_sync failed in Rx\n", id);
	rxbuf[0] = vs10xx_chips[id].rx_buf[0];
	rxbuf[1] = vs10xx_chips[id].rx_buf[1];
	}

	return status;
}

int vs10xx_io_data_rx(int id, char *rxbuf, unsigned rxlen) {

	struct spi_device *device = vs10xx_chips[id].spi_data;
	struct spi_message	spi_mesg;
	struct spi_transfer	spi_xfer_rx;
	int status = 0;

	spi_message_init(&spi_mesg);

	memset(&spi_xfer_rx, 0, sizeof spi_xfer_rx);
	spi_xfer_rx.rx_buf = rxbuf;
	spi_xfer_rx.len = rxlen;
	spi_message_add_tail(&spi_xfer_rx, &spi_mesg);

	status = spi_sync(device, &spi_mesg);
	if (status < 0) {
		vs10xx_err("id:%d spi_sync failed\n", id);
	}

	return status;
}

int vs10xx_io_data_tx(int id, const char *txbuf, unsigned txlen) {

	int status =0, ind =0;

	vs10xx_chips[id].transfer[0].tx_buf = vs10xx_chips[id].tx_buf;
	vs10xx_chips[id].transfer[0].len    = sizeof(vs10xx_chips[id].tx_buf);
	vs10xx_chips[id].transfer[1].rx_buf = vs10xx_chips[id].rx_buf;
	vs10xx_chips[id].transfer[1].len    = sizeof(vs10xx_chips[id].rx_buf);

	spi_message_init_with_transfers(&vs10xx_chips[id].msg, vs10xx_chips[id].transfer, ARRAY_SIZE(vs10xx_chips[id].transfer));

	if (txbuf && (txlen <= 32)){
           for (ind =0; ind <txlen; ind++)
	     vs10xx_chips[id].tx_buf[ind] = txbuf[ind];
	   vs10xx_chips[id].transfer[0].len    = txlen;
	   spi_message_add_tail(&vs10xx_chips[id].transfer[0], &vs10xx_chips[id].msg);
	   status = spi_sync(vs10xx_chips[id].spi_data, &vs10xx_chips[id].msg);
	}

	return status;
}

/* ----------------------------------------------------------------------------------------------------------------------------- */
/* VS10XX SPI PROBES                                                                                                             */
/* ----------------------------------------------------------------------------------------------------------------------------- */

static int vs10xx_spi_ctrl_probe(struct spi_device *spi) {

	int status = 0, device_id =0xff, gpio_reset =0x00, gpio_dreq =0x00;
	void* ptr1;

        printk("Inside vs10xx_spi_ctrl_probe\n");
	ptr1 = of_get_property(spi->dev.of_node, "device_id", NULL);
	of_property_read_u32(spi->dev.of_node, "device_id", &device_id);
	of_property_read_u32(spi->dev.of_node, "gpio_reset", &gpio_reset);
	of_property_read_u32(spi->dev.of_node, "gpio_dreq", &gpio_dreq);
	if (ptr1 == NULL) {
		vs10xx_err("no property, device_id provided for vs10xx device");
		status = -1;
	}
	printk("Read vs10xx dev_id    : %d\n", device_id);
	printk("Read vs10xx reset gpio: %d\n", gpio_reset);
	printk("Read vs10xx dreq gpio : %d\n", gpio_dreq);
	vs10xx_chips[device_id].spi_ctrl   = (struct spi_device *)spi;
	vs10xx_chips[device_id].gpio_reset = gpio_reset;
	vs10xx_chips[device_id].gpio_dreq  = gpio_dreq;
	printk("Read vs10xx_chips spi_ctrl : 0x%x\n", (unsigned int)vs10xx_chips[device_id].spi_ctrl);

	return status;
}

#if defined(CONFIG_OF)
static const struct of_device_id vs10xx_ctrl_id[] = {
	{
		.compatible = "vs1001-ctrl",
	}, {
		.compatible = "vs1011-ctrl",
	}, {
		.compatible = "vs1002-ctrl",
	}, {
		.compatible = "vs1003-ctrl",
	}, {
		.compatible = "vs1053-ctrl",
	}, {
		.compatible = "vs1033-ctrl",
	}, {
		.compatible = "vs1103-ctrl",
	}, {
	}
};

MODULE_DEVICE_TABLE(of, vs10xx_ctrl_id);
#endif

static const struct spi_device_id vs10xx_ctrlId[] = {
	{ "vs1001-ctrl", vs1001 },
	{ "vs1011-ctrl", vs1011 },
	{ "vs1002-ctrl", vs1002 },
	{ "vs1003-ctrl", vs1003 },
	{ "vs1053-ctrl", vs1053 },
	{ "vs1033-ctrl", vs1033 },
	{ "vs1103-ctrl", vs1103 },
	{ }
};

MODULE_DEVICE_TABLE(spi, vs10xx_ctrlId);

static struct spi_driver vs10xx_spi_ctrl = {
	.driver = {
		.name = "vs10xx-ctrl",
		.of_match_table = of_match_ptr(vs10xx_ctrl_id),
	},
	.probe		= vs10xx_spi_ctrl_probe,
//	.remove		= __devexit_p(vs10xx_data_remove),
};

//module_spi_driver(vs10xx_spi_ctrl);

static int vs10xx_spi_data_probe(struct spi_device *spi) {

	int status = 0, device_id =0xff;
	printk("Inside vs10xx_spi_data_probe\n");

	of_property_read_u32(spi->dev.of_node, "device_id", &device_id);
	vs10xx_chips[device_id].spi_data = (struct spi_device *)spi;
	printk("Read vs10xx dev_id    : %d\n", device_id);
	printk("Read vs10xx_chips spi_data : 0x%x\n", (unsigned int)vs10xx_chips[device_id].spi_data);
	return status;
}

#if defined(CONFIG_OF)
static const struct of_device_id vs10xx_data_id[] = {
	{
		.compatible = "vs1001-data",
	}, {
		.compatible = "vs1011-data",
	}, {
		.compatible = "vs1002-data",
	}, {
		.compatible = "vs1003-data",
	}, {
		.compatible = "vs1053-data",
	}, {
		.compatible = "vs1033-data",
	}, {
		.compatible = "vs1103-data",
	}, {
	}
};

MODULE_DEVICE_TABLE(of, vs10xx_data_id);
#endif

static const struct spi_device_id vs10xx_id[] = {
	{ "vs1001-data", vs1001 },
	{ "vs1011-data", vs1011 },
	{ "vs1002-data", vs1002 },
	{ "vs1003-data", vs1003 },
	{ "vs1053-data", vs1053 },
	{ "vs1033-data", vs1033 },
	{ "vs1103-data", vs1103 },
	{ }
};

MODULE_DEVICE_TABLE(spi, vs10xx_id);

static struct spi_driver vs10xx_spi_data = {
	.driver = {
		.name = "vs10xx-data",
		.of_match_table = of_match_ptr(vs10xx_data_id),
	},
	.probe		= vs10xx_spi_data_probe,
//	.remove		= __devexit_p(vs10xx_data_remove),
};

MODULE_AUTHOR("Rajiv Biswas <rajiv.biswas@harman.com>");
MODULE_DESCRIPTION("Harman, VS10xx Linux Driver");
MODULE_LICENSE("GPL v2");

/* ----------------------------------------------------------------------------------------------------------------------------- */
/* VS10XX IO REG/UNREG                                                                                                           */
/* ----------------------------------------------------------------------------------------------------------------------------- */

int vs10xx_io_register(void) {

	int s1 = 0, s2 = 0;

	/* register vs10xx ctrl spi driver */
	printk("Inside vs10xx_io_register\n");
	s1 = spi_register_driver(&vs10xx_spi_ctrl);
	if (s1 < 0) {
		vs10xx_err("spi_register_driver: ctrl");
	}

	/* register vs10xx data spi driver */
	s2 = spi_register_driver(&vs10xx_spi_data);
	if (s2 < 0) {
		vs10xx_err("spi_register_driver: data");
	}

	return (s1==0 && s2==0 ? 0  : -1);
}


void vs10xx_io_unregister(void) {

	/* unregister spi devices */
	spi_unregister_driver(&vs10xx_spi_ctrl);
	spi_unregister_driver(&vs10xx_spi_data);
}

/* ----------------------------------------------------------------------------------------------------------------------------- */
/* VS10XX IO INIT/EXIT                                                                                                           */
/* ----------------------------------------------------------------------------------------------------------------------------- */

int vs10xx_io_init(int id) {

	struct vs10xx_chip *chip;
	int status = -1;

	printk("Inside vs10xx_io_init function\n");
	if (id < 0 || id >= VS10XX_MAX_DEVICES) {

		vs10xx_err("id:%d out of range", id);

	} else {

		chip = &vs10xx_chips[id];

		if (chip->spi_ctrl == NULL || chip->spi_data == NULL) {

			vs10xx_dbg("id:%d no board config", id);

		} else {

			/* initialize wait queue */
			init_waitqueue_head(&chip->wq);

			gpio.map  = ioremap(GPIO_BASE, 4096);
			gpio.addr = (volatile unsigned int*)gpio.map;

				/* set gpio_reset as output, keep vs10xx in reset */
				INP_GPIO(chip->gpio_reset);
				OUT_GPIO(chip->gpio_reset);
				printk("Inside,vs10xx_io_init function hwreset value: %d\n", hwreset);
				if (hwreset)
					GPIO_CLR = 1 << (chip->gpio_reset);
				else
					GPIO_SET = 1 << (chip->gpio_reset);

				INP_GPIO(chip->gpio_dreq);
				chip->dreq_val = GPIO_READ(chip->gpio_dreq);

			if (irqmode) {

					/* request dreq irq */
					chip->irq_dreq = gpio_to_irq(chip->gpio_dreq);
					if (chip->irq_dreq < 0) {
						vs10xx_err("gpio_to_irq gpio_dreq:%d", chip->gpio_dreq);
						status = -1;
					}
					printk("gpio_to_irq() returns: %d\n", chip->irq_dreq);
			
					/* request irq for gpio_dreq */
					status = request_irq(chip->irq_dreq, vs10xx_io_irq, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, VS10XX_NAME, (void*)id);
					if (status < 0) {
						vs10xx_err("request_irq irq_dreq:%d\n", chip->irq_dreq);
						chip->irq_dreq = -1;
					}				
			}
		}
	}

	if (status == 0) {
		printk("id:%d gpio_reset:%d gpio_dreq:%d irq_dreq:%d\n", id, chip->gpio_reset, chip->gpio_dreq, chip->irq_dreq);
	}

	return status;
}

void vs10xx_io_exit(int id) {

	struct vs10xx_chip *chip = &vs10xx_chips[id];

	/* release dreq irq */
	if (irqmode && chip->irq_dreq > 0) {
		synchronize_irq(chip->irq_dreq);
		free_irq(chip->irq_dreq, (void*)id);
	}

	/* free gpio dreq */
	if (chip->gpio_dreq > 0) {
	    gpio_free(chip->gpio_dreq);
	}

	/* free gpio reset */
	if (chip->gpio_reset > 0) {

		if (hwreset) {
			gpio_set_value(chip->gpio_reset, 0);
		}

		gpio_free(chip->gpio_reset);
	}
}

