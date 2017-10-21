/* ------------------------------------------------------------------------------------------------------------------------------
    vs10xx main functions.
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
#include "vs10xx_device.h"
#include "vs10xx_queue.h"
#include "vs10xx_iocomm.h"

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static int debug = 0;
int *vs10xx_debug = &debug;
module_param(debug, int, 0644);

MODULE_DESCRIPTION("Linux Device Driver for vs10xx");
MODULE_AUTHOR("Rajiv Biswas");
MODULE_LICENSE("GPL v2");

static dev_t vs10xx_cdev_region;
static struct cdev *vs10xx_cdev;

/* ----------------------------------------------------------------------------------------------------------------------------- */
/* CHARDEV INTERFACE                                                                                                             */
/* ----------------------------------------------------------------------------------------------------------------------------- */

static int vs10xx_open(struct inode *inode, struct file *file) {

	int id = MINOR(inode->i_rdev);
	int status = 0;

	vs10xx_dbg("id:%d", id);

	status = vs10xx_device_open(id);

	if (status < 0) {
		vs10xx_inf("id:%d not valid or already open", id);
		status = -EACCES;
	} else {
		file->private_data = (void*)id;
	}

	return status;
}


static int vs10xx_release(struct inode *inode, struct file *file) {

	int id = (int)file->private_data;
	int status = 0;

	vs10xx_dbg("id:%d", id);

	status = vs10xx_device_release(id);

	if (status < 0) {
		status = -EIO;
	}

	return status;
}

static ssize_t vs10xx_read(struct file *file, char *buffer, size_t len, loff_t *offset) {
	
	int id = (int)file->private_data;
	unsigned char txF = 0x00;
	int err_cnt = 0;

	txF = vs10xx_device_getTxFlag(id);
	err_cnt = copy_to_user(buffer, &txF, 0x01);

	return err_cnt;
}

static ssize_t vs10xx_write(struct file *file, const char __user *usrbuf, size_t lbuf, loff_t *ppos) {

	int id = (int)file->private_data;
	int status = 0;

	int nbytes = 0;
	int acttodo = 0;
	int copied = 0;

	struct vs10xx_queue_buf_t *buffer = NULL;

	do {

		buffer = vs10xx_device_getbuf(id);

		if ((buffer == NULL)) {

			printk("id:%d queue full.\n", id);
			msleep(1);

		} else {
			acttodo += 1;
			if (acttodo <= 2048)
			{
			     nbytes = copy_from_user( buffer->data, usrbuf+copied, sizeof(buffer->data) );
			     buffer->len = sizeof(buffer->data);
			     status = vs10xx_device_write(id);
			     copied += sizeof(buffer->data);
			}
			else
			     break;
		}

	} while (copied < lbuf && buffer != NULL);

	// Now Transmit the copied to Device, vs10xx
	vs10xx_device_transmit(id);

	return status;
}

	struct vs10xx_scireg scireg;
	struct vs10xx_clockf clockf;
	struct vs10xx_volume volume;
	struct vs10xx_tone tone;
	struct vs10xx_info info;

static long vs10xx_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {

	int ioctype =_IOC_TYPE(cmd), iocnr = _IOC_NR(cmd), /*iocdir = _IOC_DIR(cmd),*/ iocsize = _IOC_SIZE(cmd);
	int id = (int)file->private_data;
	int val = 0;
	char __user * usrbuf = (void __user *)arg;

	if (ioctype != VS10XX_CTL_TYPE) {
		vs10xx_dbg("id:%d unsupported ioctl type:%c nr:%d", id, ioctype, iocnr);
		return -EINVAL;
	}

	switch (iocnr) {
		case _IOC_NR(VS10XX_CTL_RESET):
			printk("Inside ioctl CTL_RESET case:\n");
			vs10xx_device_reset(id);
			break;
		case _IOC_NR(VS10XX_CTL_GETSCIREG):
			printk("Inside ioctl CTL_GETSCIREG case:\n");
			val = copy_from_user(&scireg, usrbuf, iocsize);
			printk("copy_from_user() returns: %d\n", val);
			vs10xx_device_getscireg(id, &scireg);
			val = copy_to_user(usrbuf, &scireg, iocsize);
			printk("copy_to_user() returns: %d\n", val);
			break;
		case _IOC_NR(VS10XX_CTL_SETSCIREG):
			printk("Inside ioctl CTL_SETSCIREG case:\n");
			val = copy_from_user(&scireg, usrbuf, iocsize);
			printk("copy_from_user() returns: %d\n", val);
			vs10xx_device_setscireg(id, &scireg);
			break;
		case _IOC_NR(VS10XX_CTL_GETCLOCKF):
			printk("Inside ioctl CTL_GETCLOCKF case:\n");
			vs10xx_device_getclockf(id, &clockf);
			val = copy_to_user(usrbuf, &clockf, iocsize);
			printk("copy_to_user() returns: %d\n", val);
			break;
		case _IOC_NR(VS10XX_CTL_SETCLOCKF):
			printk("Inside ioctl CTL_SETCLOCKF case:\n");
			val = copy_from_user(&clockf, usrbuf, iocsize);
			printk("copy_from_user() returns: %d\n", val);
			vs10xx_device_setclockf(id, &clockf);
			break;
		case _IOC_NR(VS10XX_CTL_GETVOLUME):
			printk("Inside ioctl CTL_GETVOLUME case:\n");
			vs10xx_device_getvolume(id, &volume);
			val = copy_to_user(usrbuf, &volume, iocsize);
			printk("copy_to_user() returns: %d\n", val);
			break;
		case _IOC_NR(VS10XX_CTL_SETVOLUME):
			printk("Inside ioctl CTL_SETVOLUME case:\n");
			val = copy_from_user(&volume, usrbuf, iocsize);
			printk("copy_from_user() returns: %d\n", val);
			vs10xx_device_setvolume(id, &volume);
			break;
		case _IOC_NR(VS10XX_CTL_GETTONE):
			printk("Inside ioctl CTL_GETTONE case:\n");
			vs10xx_device_gettone(id, &tone);
			val = copy_to_user(usrbuf, &tone, iocsize);
			printk("copy_to_user() returns: %d\n", val);
			break;
		case _IOC_NR(VS10XX_CTL_SETTONE):
			printk("Inside ioctl CTL_SETTONE case:\n");
			val = copy_from_user(&tone, usrbuf, iocsize);
			printk("copy_from_user() returns: %d\n", val);
			vs10xx_device_settone(id, &tone);
			break;
		case _IOC_NR(VS10XX_CTL_GETINFO):
			printk("Inside ioctl CTL_GETINFO case:\n");
			vs10xx_device_getinfo(id, &info);
			val = copy_to_user(usrbuf, &info, iocsize);
			printk("copy_to_user() returns: %d\n", val);
			break;
		default:
			vs10xx_dbg("id:%d unsupported ioctl type:%c nr:%d", id, ioctype, iocnr);
			return -EINVAL;
	}

	return 0;
}

static const struct file_operations vs10xx_fops = {
	.owner = THIS_MODULE,
	.open = vs10xx_open,
	.release = vs10xx_release,
	.write = vs10xx_write,
	.read = vs10xx_read,
	.unlocked_ioctl = vs10xx_ioctl,
};

/* ----------------------------------------------------------------------------------------------------------------------------- */
/* VS10XX SYSFS INTERFACE                                                                                                        */
/* ----------------------------------------------------------------------------------------------------------------------------- */

static struct class_attribute vs10xx_class_attrs[] = {
	__ATTR_NULL,
};

static struct class vs10xx_class = {
	.name =		VS10XX_NAME,
	.owner =	THIS_MODULE,
	.class_attrs =	vs10xx_class_attrs,
};

static ssize_t vs10xx_sys_reset_w(struct device *dev, struct device_attribute *attr, const char *buf, size_t size) {

	const int id = (int)dev_get_drvdata(dev);
	vs10xx_device_reset(id);
	return size;
}

static ssize_t vs10xx_sys_test_w(struct device *dev, struct device_attribute *attr, const char *buf, size_t size) {

	const int id = (int)dev_get_drvdata(dev);

	if (buf && (*buf=='M' || *buf=='m')) {
		vs10xx_device_memtest(id);
	}

	if (buf && (*buf=='S' || *buf=='s')) {
		vs10xx_device_sinetest(id);
	}

	return size;
}

static ssize_t vs10xx_sys_status_r(struct device *dev, struct device_attribute *attr, char *buf) {

	const int id = (int)dev_get_drvdata(dev);
	return vs10xx_device_status(id, buf);
}

static const DEVICE_ATTR(reset, 0220, NULL, vs10xx_sys_reset_w);
static const DEVICE_ATTR(test, 0660, NULL, vs10xx_sys_test_w);
static const DEVICE_ATTR(status, 0440, vs10xx_sys_status_r, NULL);

static const struct attribute *vs10xx_attrs[] = {
	&dev_attr_reset.attr,
	&dev_attr_test.attr,
	&dev_attr_status.attr,
	NULL,
};

static const struct attribute_group vs10xx_attr_group = {
	.attrs = (struct attribute **) vs10xx_attrs,
};

/* ----------------------------------------------------------------------------------------------------------------------------- */
/* MODULE INIT/EXIT                                                                                                              */
/* ----------------------------------------------------------------------------------------------------------------------------- */

static void vs10xx_cleanup (void) {

	int i;

	/* cleanup vs10xx devices */
	for (i=0; i<VS10XX_MAX_DEVICES; i++) {

		/* exit device */
		vs10xx_device_exit(i);

		/* destroy device */
		device_destroy(&vs10xx_class, MKDEV(MAJOR(vs10xx_cdev_region), i));

		/* exit queue */
		vs10xx_queue_exit(i);

		/* exit io */
		vs10xx_io_exit(i);
	}

	vs10xx_io_unregister();

	/* cleanup char device driver */
	if (vs10xx_cdev) {
		cdev_del(vs10xx_cdev);
	}

	/* unregister vs10xx class */
	class_unregister(&vs10xx_class);

	/* unregister vs10xx region */
	unregister_chrdev_region(vs10xx_cdev_region, VS10XX_MAX_DEVICES);

}

static int __init vs10xx_init (void) {

	int i, status = 0;
	struct device *dev;

	vs10xx_dbg("start");

	if (status == 0) {
		/* register vs10xx region */
		status = alloc_chrdev_region(&vs10xx_cdev_region, 0, VS10XX_MAX_DEVICES, VS10XX_NAME);
		if (status < 0) {
			vs10xx_err("alloc_chrdev_region");
		}
	}

	if (status == 0) {
		/* register vs10xx class */
		status = class_register(&vs10xx_class);
		if (status < 0) {
			vs10xx_err("class_register");
		}
	}

	if (status == 0) {
		/* create vs10xx character device driver */
		vs10xx_cdev = cdev_alloc();
		if (vs10xx_cdev == NULL) {
			vs10xx_err("cdev_alloc");
			status = -1;
		} else {
			cdev_init(vs10xx_cdev, &vs10xx_fops);
			status = cdev_add(vs10xx_cdev, vs10xx_cdev_region, VS10XX_MAX_DEVICES);
			if (status < 0) {
				vs10xx_err("cdev_add");
			}
		}
	}

	if (status == 0) {
		/* register io */
		status = vs10xx_io_register();
	}

	printk("status value = %d, after function vs10xx_io_register().\n", status);
	/* initialize vs10xx devices */
	for (i = 0; (status == 0) && (i < VS10XX_MAX_DEVICES); i++) {

		/* init io path */
		if (vs10xx_io_init(i) == 0) {

			/* init queue */
			status = vs10xx_queue_init(i);

			if (status == 0) {
				/* create vs10xx char device */
				dev = device_create(&vs10xx_class, NULL, MKDEV(MAJOR(vs10xx_cdev_region), i), (void*)i, "%s-%d", VS10XX_NAME, i);
				if (dev == NULL) {
					vs10xx_err("device_create id:%d name:%s-%d maj:%d min:%d", i, VS10XX_NAME, i, MAJOR(vs10xx_cdev_region), i);
					status = -1;
				} else {
					dev_set_drvdata(dev, (void*)i);
					status = sysfs_create_group(&dev->kobj, &vs10xx_attr_group);
				}
			}
			
			if (status == 0) {
				status = vs10xx_device_init(i, dev);
			}

			if (status == 0) {
				//vs10xx_inf("id:%d name:%s-%d maj:%d min:%d", i, VS10XX_NAME, i, MAJOR(vs10xx_cdev_region), i);
				printk("id:%d name:%s-%d maj:%d min:%d", i, VS10XX_NAME, i, MAJOR(vs10xx_cdev_region), i);
			}
		}
	}

	if (status != 0) {

		vs10xx_cleanup();
	}

	vs10xx_dbg("done");

	return status;
}

static void __exit vs10xx_exit (void) {

	vs10xx_dbg("start");

	vs10xx_cleanup();

	vs10xx_dbg("done");
}

module_init(vs10xx_init);
module_exit(vs10xx_exit);

