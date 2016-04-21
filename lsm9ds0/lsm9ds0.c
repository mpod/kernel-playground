/*
 * lsm9ds0_gyro.c
 *
 * Copyright (C) 2016 Matija Podravec <matija_podravec@fastmail.fm>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * 
 *
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/kfifo_buf.h>

#define LSM9DS0_WHO_AM_I_G_REG          (0x0F)
#define LSM9DS0_CTRL_REG1_G_REG         (0x20)
#define LSM9DS0_CTRL_REG2_G_REG         (0x21)
#define LSM9DS0_CTRL_REG3_G_REG         (0x22)
#define LSM9DS0_CTRL_REG4_G_REG         (0x23)
#define LSM9DS0_CTRL_REG5_G_REG         (0x24)
#define LSM9DS0_REFERENCE_G_REG         (0x25)
#define LSM9DS0_STATUS_REG_G_REG        (0x27)
#define LSM9DS0_OUT_X_L_G_REG           (0x28)
#define LSM9DS0_OUT_X_H_G_REG           (0x29)
#define LSM9DS0_OUT_Y_L_G_REG           (0x2A)
#define LSM9DS0_OUT_Y_H_G_REG           (0x2B)
#define LSM9DS0_OUT_Z_L_G_REG           (0x2C)
#define LSM9DS0_OUT_Z_H_G_REG           (0x2D)
#define LSM9DS0_FIFO_CTRL_REG_G_REG     (0x2E)
#define LSM9DS0_FIFO_SRC_REG_G_REG      (0x2F)
#define LSM9DS0_INT1_CFG_G_REG          (0x30)
#define LSM9DS0_INT1_SRC_G_REG          (0x31) 
#define LSM9DS0_INT1_TSH_XH_G_REG       (0x32)
#define LSM9DS0_INT1_TSH_XL_G_REG       (0x33)
#define LSM9DS0_INT1_TSH_YH_G_REG       (0x34)
#define LSM9DS0_INT1_TSH_YL_G_REG       (0x35)
#define LSM9DS0_INT1_TSH_ZH_G_REG       (0x36)
#define LSM9DS0_INT1_TSH_ZL_G_REG       (0x37)
#define LSM9DS0_INT1_DURATION_G_REG     (0x38)

#define LSM9DS0_GYRO_ODR_AVL_95HZ_VAL   0x00
#define LSM9DS0_GYRO_ODR_AVL_190HZ_VAL  0x01
#define LSM9DS0_GYRO_ODR_AVL_380HZ_VAL  0x02
#define LSM9DS0_GYRO_ODR_AVL_760HZ_VAL  0x03

#define LSM9DS0_WHO_AM_I_G              0xD4

struct lsm9ds0_gyro_data {
  struct i2c_client *client;
  struct mutex lock;
};

enum { SCAN_INDEX_X, SCAN_INDEX_Y, SCAN_INDEX_Z };

struct sensor_odr_avl {
  unsigned int hz;
  u8 value;
};

static const struct sensor_odr_avl lsm9ds0_gyro_odr_avl[4] = {
  {95, LSM9DS0_GYRO_ODR_AVL_95HZ_VAL},
  {190, LSM9DS0_GYRO_ODR_AVL_190HZ_VAL},
  {380, LSM9DS0_GYRO_ODR_AVL_380HZ_VAL},
  {760, LSM9DS0_GYRO_ODR_AVL_760HZ_VAL},
};

static ssize_t lsm9ds0_gyro_show_samp_freq_avail(struct device *dev,
        struct device_attribute *attr, char *buf)
{
  size_t len = 0;
  int n = ARRAY_SIZE(lsm9ds0_gyro_odr_avl);

  while (n-- > 0)
    len += scnprintf(buf + len, PAGE_SIZE - len,
      "%d ", lsm9ds0_gyro_odr_avl[n].hz);

  /* replace trailing space by newline */
  buf[len - 1] = '\n';

  return len;
}

static IIO_DEV_ATTR_SAMP_FREQ_AVAIL(lsm9ds0_gyro_show_samp_freq_avail);

static struct attribute *lsm9ds0_gyro_attributes[] = {
  &iio_dev_attr_sampling_frequency_available.dev_attr.attr,
  //&iio_dev_attr_in_accel_scale_available.dev_attr.attr,
  NULL
};

static const struct attribute_group lsm9ds0_gyro_group = {
  .attrs = lsm9ds0_gyro_attributes,
};


static const struct iio_buffer_setup_ops lsm9ds0_gyro_buffer_setup_ops = {
  .postenable = &iio_triggered_buffer_postenable,
  .predisable = &iio_triggered_buffer_predisable,
};

static irqreturn_t lsm9ds0_gyro_trigger_h(int irq, void *p)
{
  struct iio_poll_func *pf = p;
  struct iio_dev *indio_dev = pf->indio_dev;
  u32 *buf_data;

  buf_data = kmalloc(indio_dev->scan_bytes, GFP_KERNEL);
  if (!buf_data)
    goto done;

  if (!bitmap_empty(indio_dev->active_scan_mask, indio_dev->masklength)) {
    struct lsm9ds0_gyro_data *data = iio_priv(indio_dev);

    mutex_lock(&data->lock);

    mutex_unlock(&data->lock);
  }

  iio_push_to_buffers_with_timestamp(indio_dev, buf_data, iio_get_time_ns());
  kfree(buf_data);

done:
  iio_trigger_notify_done(indio_dev->trig);

  return IRQ_HANDLED;
}

static const struct iio_chan_spec lsm9ds0_gyro_channels[] = {
  {
    .type = IIO_ANGL_VEL,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE), 
    .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ), 
    .modified = 1,
    .channel2 = IIO_MOD_X,
    .scan_index = SCAN_INDEX_X,
    .scan_type = {
      .sign = 's',
      .realbits = 16,
      .storagebits = 16,
      .shift = 0,
      .endianness = IIO_LE,
    },
  }, {
    .type = IIO_ANGL_VEL,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE), 
    .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ), 
    .modified = 1,
    .channel2 = IIO_MOD_Y,
    .scan_index = SCAN_INDEX_Y,
    .scan_type = {
      .sign = 's',
      .realbits = 16,
      .storagebits = 16,
      .shift = 0,
      .endianness = IIO_LE,
    },
  }, {
    .type = IIO_ANGL_VEL,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE), 
    .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ), 
    .modified = 1,
    .channel2 = IIO_MOD_Z,
    .scan_index = SCAN_INDEX_Z,
    .scan_type = {
      .sign = 's',
      .realbits = 16,
      .storagebits = 16,
      .shift = 0,
      .endianness = IIO_LE,
    },
  },
  IIO_CHAN_SOFT_TIMESTAMP(3),
};

static int lsm9ds0_gyro_read_raw(struct iio_dev *iio_dev,
      struct iio_chan_spec const *channel, 
      int *val, int *val2, long mask)
{
  return IIO_VAL_INT;
}


static const struct iio_info lsm9ds0_gyro_info = {
	.attrs = &lsm9ds0_gyro_group,
  .read_raw = lsm9ds0_gyro_read_raw,
  .driver_module = THIS_MODULE,
};

static int lsm9ds0_gyro_init(struct i2c_client *client)
{
  return 0;
}

static int lsm9ds0_gyro_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
{
  struct iio_dev *indio_dev;
  struct lsm9ds0_gyro_data *data;
  struct iio_buffer *buffer;
  int ret;

  
  dev_info(&client->dev, "Entering LSM9DS0 probe...\n");
  if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_WORD_DATA)) {
    ret = -ENODEV;
    goto error_ret;
  }

  ret = i2c_smbus_read_byte_data(client, LSM9DS0_WHO_AM_I_G_REG);
  if (ret < 0) {
    ret = -EINVAL;
    goto error_ret;
  }
  dev_info(&client->dev, "Chip id found: 0x%x\n", ret);
  if (ret != LSM9DS0_WHO_AM_I_G) {
    dev_err(&client->dev, "No LSM9DS0 sensor\n");
    ret = -ENODEV;
    goto error_ret;
  }

  // LSM9DS0 init
  lsm9ds0_gyro_init(client);

  indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
  if (!indio_dev) {
    ret = -ENOMEM;
    goto error_ret;
  }

  data = iio_priv(indio_dev);
  mutex_init(&data->lock);
  i2c_set_clientdata(client, indio_dev);
  data->client = client;

  indio_dev->dev.parent = &client->dev;
  indio_dev->name = dev_name(&client->dev);
  indio_dev->channels = lsm9ds0_gyro_channels;
  indio_dev->num_channels = ARRAY_SIZE(lsm9ds0_gyro_channels);
  indio_dev->info = &lsm9ds0_gyro_info;
  indio_dev->modes = INDIO_DIRECT_MODE | INDIO_BUFFER_TRIGGERED;

  /* Allocate buffer */
  buffer = iio_kfifo_allocate();
  if (!buffer) {
    ret = -ENOMEM;
    goto error_free_device;
  }
  iio_device_attach_buffer(indio_dev, buffer);
  buffer->scan_timestamp = true;
  indio_dev->setup_ops = &lsm9ds0_gyro_buffer_setup_ops;
  indio_dev->pollfunc = iio_alloc_pollfunc(NULL,
             &lsm9ds0_gyro_trigger_h,
             IRQF_ONESHOT,
             indio_dev,
             "lsm9ds0_gyro_consumer%d",
             indio_dev->id);
  if (!indio_dev->pollfunc) {
    ret = -ENOMEM;
    goto error_free_buffer;
  }

  ret = iio_device_register(indio_dev);
  if (ret < 0)
    goto error_unconfigure_buffer;

  dev_info(&client->dev, "LSM9DS0 registered.\n");
  return 0;

error_unconfigure_buffer:
  iio_dealloc_pollfunc(indio_dev->pollfunc);
error_free_buffer:
  iio_kfifo_free(indio_dev->buffer);
error_free_device:
  iio_device_free(indio_dev);
error_ret:
  return ret;
}

static int lsm9ds0_gyro_remove(struct i2c_client *client)
{
  return 0;
}

static const struct i2c_device_id lsm9ds0_gyro_id[] = {
  { "lsm9ds0_gyro", 0 },
  { }
};
MODULE_DEVICE_TABLE(i2c, lsm9ds0_gyro_id);

static struct i2c_driver lsm9ds0_gyro_driver = {
  .driver = {
    .name = "lsm9ds0_gyro",
    .owner = THIS_MODULE,
  },
  .probe = lsm9ds0_gyro_probe,
  .remove = lsm9ds0_gyro_remove,
  .id_table = lsm9ds0_gyro_id,
};
module_i2c_driver(lsm9ds0_gyro_driver);

MODULE_AUTHOR("Matija Podravec <matija_podravec@fastmail.fm>");
MODULE_DESCRIPTION("LSM9DS0 gyroscope sensor");
MODULE_LICENSE("GPL");
