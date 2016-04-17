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
#include <linux/i2c.h>
#include <linux/iio/iio.h>

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

#define LSM9DS0_WHO_AM_I_G              0xD4

struct lsm9ds0_gyro_data {
  struct i2c_client *client;
  struct mutex lock;
};

static int lsm9ds0_gyro_init(struct i2c_client *client)
{
  return 0;
}

static int lsm9ds0_gyro_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
{
  /*struct iio_dev *indio_dev;
  struct lsm9ds0_gyro_data *data;
  struct iio_buffer *buffer;*/
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

  return 0;

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
