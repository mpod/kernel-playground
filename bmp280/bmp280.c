/*
 * bmp280.c
 *
 * Copyright (c) 2016 Matija Podravec <matija_podravec@fastmail.fm>
 *
 * This file is subject to the terms and conditions of version 2 of
 * the GNU General Public License.  See the file COPYING in the main
 * directory of this archive for more details.
 *
 * Driver for the Bosch BMP280 Digital Pressure Sensor 
 *
 */

#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/bitops.h>

#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>


#define BMP280_CHIP_ID_REG                   (0xD0)  /*Chip ID Register */
#define BMP280_RST_REG                       (0xE0)  /*Softreset Register */
#define BMP280_STAT_REG                      (0xF3)  /*Status Register */
#define BMP280_CTRL_MEAS_REG                 (0xF4)  /*Ctrl Measure Register */
#define BMP280_CONFIG_REG                    (0xF5)  /*Configuration Register */
#define BMP280_PRESSURE_MSB_REG              (0xF7)  /*Pressure MSB Register */
#define BMP280_PRESSURE_LSB_REG              (0xF8)  /*Pressure LSB Register */
#define BMP280_PRESSURE_XLSB_REG             (0xF9)  /*Pressure XLSB Register */
#define BMP280_TEMPERATURE_MSB_REG           (0xFA)  /*Temperature MSB Reg */
#define BMP280_TEMPERATURE_LSB_REG           (0xFB)  /*Temperature LSB Reg */
#define BMP280_TEMPERATURE_XLSB_REG          (0xFC)  /*Temperature XLSB Reg */

#define BMP280_TEMPERATURE_COMP_START_REG    (0x88)
#define BMP280_TEMPERATURE_COMP_REG_COUNT    6
#define BMP280_PRESSURE_COMP_START_REG       (0x8E)
#define BMP280_PRESSURE_COMP_REG_COUNT       18

#define BMP280_OSRS_TEMP_MASK                (BIT(7) | BIT(6) | BIT(5))
#define BMP280_OSRS_TEMP_SKIP                0
#define BMP280_OSRS_TEMP_1X                  BIT(5)
#define BMP280_OSRS_TEMP_2X                  BIT(6)
#define BMP280_OSRS_TEMP_4X                  (BIT(6) | BIT(5))
#define BMP280_OSRS_TEMP_8X                  BIT(7)
#define BMP280_OSRS_TEMP_16X                 (BIT(7) | BIT(5))

#define BMP280_OSRS_PRESS_MASK               (BIT(4) | BIT(3) | BIT(2))
#define BMP280_OSRS_PRESS_SKIP               0
#define BMP280_OSRS_PRESS_1X                 BIT(2)
#define BMP280_OSRS_PRESS_2X                 BIT(3)
#define BMP280_OSRS_PRESS_4X                 (BIT(3) | BIT(2))
#define BMP280_OSRS_PRESS_8X                 BIT(4)
#define BMP280_OSRS_PRESS_16X                (BIT(4) | BIT(2))

#define BMP280_MODE_MASK                     (BIT(1) | BIT(0))
#define BMP280_MODE_SLEEP                    0
#define BMP280_MODE_FORCED                   BIT(0)
#define BMP280_MODE_NORMAL                   (BIT(1) | BIT(0))

#define BMP280_FILTER_MASK                   (BIT(4) | BIT(3) | BIT(2))
#define BMP280_FILTER_OFF                    0
#define BMP280_FILTER_2X                     BIT(2)
#define BMP280_FILTER_4X                     BIT(3)
#define BMP280_FILTER_8X                     (BIT(3) | BIT(2))
#define BMP280_FILTER_16X                    BIT(4)

#define BMP280_STANDBY_TIME_0_5              0
#define BMP280_STANDBY_TIME_62_5             BIT(5)
#define BMP280_STANDBY_TIME_125              BIT(6)
#define BMP280_STANDBY_TIME_250              (BIT(5) | BIT(6))
#define BMP280_STANDBY_TIME_500              BIT(7)
#define BMP280_STANDBY_TIME_1000             (BIT(7) | BIT(5)) 
#define BMP280_STANDBY_TIME_2000             (BIT(7) | BIT(6))
#define BMP280_STANDBY_TIME_4000             (BIT(7) | BIT(6) | BIT(5))

#define BMP280_CHIP_ID                       0x58
#define BMP280_SOFT_RESET_VAL                0xB6

struct bmp280_data {
  struct i2c_client *client;
  struct mutex lock;
  u16 config;
  int temperature;
  int preassure;
  s32 t_fine;
};

enum { T1, T2, T3 };
enum { P1, P2, P3, P4, P5, P6, P7, P8, P9 };

static s32 bmp280_compensate_temp(struct bmp280_data *data, s32 adc_temp)
{
  u8 buf[BMP280_TEMPERATURE_COMP_REG_COUNT];
  s32 var1, var2, temp;
  s16 dig[BMP280_TEMPERATURE_COMP_REG_COUNT / 2];
  int ret;
  int i;

  buf[0] = BMP280_TEMPERATURE_COMP_START_REG;
  ret = i2c_master_send(data->client, buf, 1);
  if (ret < 0) {
    return ret;
  }
  ret = i2c_master_recv(data->client, buf, BMP280_TEMPERATURE_COMP_REG_COUNT);
  if (ret < 0) {
    return ret;
  }
  for (i = 0; i < BMP280_TEMPERATURE_COMP_REG_COUNT / 2; i++) {
    dig[i] = (buf[2 * i + 1] << 8) | buf[2 * i];
  }

  var1 = ((((adc_temp >> 3) - ((s32)(u16)dig[T1] << 1))) * ((s32)dig[T2])) >> 11;
  var2 = (((((adc_temp >> 4) - ((s32)dig[T1])) * ((adc_temp >> 4) - 
            ((s32)(u16)dig[T1]))) >> 12) * ((s32)dig[T3])) >> 14; 
  data->t_fine = var1 + var2;
  temp  = (data->t_fine * 5 + 128) >> 8;
  return temp;
}

static u32 bmp280_compensate_press(struct bmp280_data *data, s32 adc_press)
{
  u8 buf[BMP280_PRESSURE_COMP_REG_COUNT];
  s64 var1, var2, press;
  s16 dig[BMP280_PRESSURE_COMP_REG_COUNT / 2];
  int ret;
  int i;

  buf[0] = BMP280_PRESSURE_COMP_START_REG;
  ret = i2c_master_send(data->client, buf, 1);
  if (ret < 0) {
    return ret;
  }
  ret = i2c_master_recv(data->client, buf, BMP280_PRESSURE_COMP_REG_COUNT);
  if (ret < 0) {
    return ret;
  }
  for (i = 0; i < BMP280_PRESSURE_COMP_REG_COUNT / 2; i++) {
    dig[i] = (buf[2 * i + 1] << 8) | buf[2 * i];
  }

  var1 = ((s64)data->t_fine) - 128000;
  var2 = var1 * var1 * (s64)dig[P6];
  var2 = var2 + ((var1*(s64)dig[P5]) << 17);
  var2 = var2 + (((s64)dig[P4]) << 35);
  var1 = ((var1 * var1 * (s64)dig[P3]) >> 8) + ((var1 * (s64)dig[P2]) << 12);
  var1 = (((((s64)1) << 47) + var1)) * ((s64)(u16)dig[P1]) >> 33;
  if (var1 == 0) {
    // avoid exception caused by division by zero
    return 0; 
  }
  press = 1048576 - adc_press;
  press = div64_s64((((press << 31) - var2) * 3125), var1);
  var1 = (((s64)dig[P9]) * (press >> 13) * (press >> 13)) >> 25;
  var2 = (((s64)dig[P8]) * press) >> 19;
  press = ((press + var1 + var2) >> 8) + (((s64)dig[P7]) << 4);
  return (u32)press;
}

static int bmp280_read_measurements(struct i2c_client *client, s32 *temp, s32 *press)
{
  u8 buf[6] = {0, 0, 0, 0, 0, 0};
  int ret;

  buf[0] = BMP280_PRESSURE_MSB_REG;
  ret = i2c_master_send(client, buf, 1);
  if (ret < 0) {
    return ret;
  }
  ret = i2c_master_recv(client, buf, 6);
  if (ret < 0) {
    return ret;
  }
  *press = (buf[0] << 12) | (buf[1] << 4) | ((buf[2] & 0xF0) >> 4);
  *temp = (buf[3] << 12) | (buf[4] << 4) | ((buf[5] & 0xF0) >> 4);
  return ret;
}

static int bmp280_read_raw(struct iio_dev *iio_dev,
      struct iio_chan_spec const *channel, 
      int *val, int *val2, long mask)
{
  struct bmp280_data *data = iio_priv(iio_dev);
  int ret;
  s32 adc_temp = 0;
  s32 adc_press = 0;
  dev_info(iio_dev->dev.parent, "Entering bmp280_read_raw...");
  dev_info(iio_dev->dev.parent, "channel: 0x%x, mask: %ld", channel->type, mask);
  mutex_lock(&data->lock);
  bmp280_read_measurements(data->client, &adc_temp, &adc_press);
  if (mask == IIO_CHAN_INFO_RAW && channel->type == IIO_TEMP) {
    *val = adc_temp;
    ret = IIO_VAL_INT;
  } else if (mask == IIO_CHAN_INFO_RAW && channel->type == IIO_PRESSURE) {
    *val = adc_press;
    ret = IIO_VAL_INT;
  } else if (mask == IIO_CHAN_INFO_PROCESSED && channel->type == IIO_TEMP) {
    *val = bmp280_compensate_temp(data, adc_temp);
    ret = IIO_VAL_INT;
  } else if (mask == IIO_CHAN_INFO_PROCESSED && channel->type == IIO_PRESSURE) {
    bmp280_compensate_temp(data, adc_temp);
    *val = bmp280_compensate_press(data, adc_press);
    *val2 = 256;
    ret = IIO_VAL_FRACTIONAL;
  } else {
    ret = -EINVAL;
  }

  mutex_unlock(&data->lock);
  return ret;
}

static int bmp280_init(struct i2c_client *client) 
{
  int ret;

  ret = i2c_smbus_write_byte_data(client, BMP280_CONFIG_REG,
        BMP280_STANDBY_TIME_500 | BMP280_FILTER_16X);
  if (ret < 0) {
    dev_err(&client->dev,
      "Failed to write config register.\n");
    return ret;
  }

  ret = i2c_smbus_write_byte_data(client, BMP280_CTRL_MEAS_REG, 
        BMP280_OSRS_PRESS_16X | BMP280_OSRS_TEMP_2X | BMP280_MODE_NORMAL);
  if (ret < 0) {
    dev_err(&client->dev,
      "Failed to write ctrl_meas register.\n");
    return ret;
  }

  return ret;
}

static const struct iio_chan_spec bmp280_channels[] = {
  {
    .type = IIO_TEMP,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
      BIT(IIO_CHAN_INFO_PROCESSED),
  }, {
    .type = IIO_PRESSURE,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | 
      BIT(IIO_CHAN_INFO_PROCESSED),
  }
};

static const struct iio_info bmp280_info = {
  .read_raw = bmp280_read_raw,
  .driver_module = THIS_MODULE,
};

static int bmp280_probe(struct i2c_client *client,
       const struct i2c_device_id *id)
{
  struct iio_dev *indio_dev;
  struct bmp280_data *data;
  int ret;

  dev_info(&client->dev, "Entering BMP280 probe...\n");
  if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_WORD_DATA))
    return -ENODEV;

  ret = i2c_smbus_read_byte_data(client, BMP280_CHIP_ID_REG);
  if (ret < 0) {
    return ret;
  }
  dev_info(&client->dev, "Chip id found: 0x%x\n", ret);
  if (ret != BMP280_CHIP_ID) {
    dev_err(&client->dev, "No BMP280 sensor\n");
    return -ENODEV;
  }

  ret = bmp280_init(client);
  if (ret < 0) {
    return -EINVAL;
  }

  indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
  if (!indio_dev)
    return -ENOMEM;

  data = iio_priv(indio_dev);
  mutex_init(&data->lock);
  i2c_set_clientdata(client, indio_dev);
  data->client = client;

  indio_dev->dev.parent = &client->dev;
  indio_dev->name = dev_name(&client->dev);
  indio_dev->channels = bmp280_channels;
  indio_dev->num_channels = ARRAY_SIZE(bmp280_channels);
  indio_dev->info = &bmp280_info;
  indio_dev->modes = INDIO_DIRECT_MODE;

  return iio_device_register(indio_dev);
}

static int bmp280_remove(struct i2c_client *client)
{
  struct iio_dev *indio_dev = i2c_get_clientdata(client);

  iio_device_unregister(indio_dev);
  return i2c_smbus_write_byte_data(client, BMP280_CTRL_MEAS_REG, 0);
}

static const struct i2c_device_id bmp280_id[] = {
  { "bmp280", 0 },
  { }
};
MODULE_DEVICE_TABLE(i2c, bmp280_id);

static struct i2c_driver bmp280_driver = {
  .driver = {
    .name = "bmp280",
    .owner = THIS_MODULE,
  },
  .probe = bmp280_probe,
  .remove = bmp280_remove,
  .id_table  = bmp280_id,
};
module_i2c_driver(bmp280_driver);

MODULE_AUTHOR("Matija Podravec <matija_podravec@fastmail.fm>");
MODULE_DESCRIPTION("BMP280 preassure and temperature sensor");
MODULE_LICENSE("GPL");
