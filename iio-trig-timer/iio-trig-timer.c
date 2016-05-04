/*
 * iio-trig-timer.c
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
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/irq_work.h>
#include <linux/iio/iio.h>
#include <linux/iio/trigger.h>

struct iio_trig_timer_info {
  struct iio_trigger *trig;
  struct rtc_device *rtc;
  struct irq_work work;
  unsigned int frequency;
  struct rtc_task task;
  bool state;
};

static struct iio_trig_timer_info *iio_trig_timer;

static struct attribute *iio_trig_timer_attrs[] = {
  NULL,
};

static const struct attribute_group iio_trig_timer_group = {
  .attrs = iio_trig_timer_attrs,
};

static const struct attribute_group *iio_trig_timer_groups[] = {
  &iio_trig_timer_group,
  NULL
};

/* Nothing to actually do upon release */
static void iio_trig_timer_release(struct device *dev)
{
}

static void iio_rtc_trigger_poll(void *private_data)
{
  iio_trigger_poll(private_data);
  printk(KERN_INFO "trigger\n");
}

static const struct iio_trigger_ops iio_trig_timer_ops = {
  .owner = THIS_MODULE,
};

static int __init iio_trig_timer_init(void)
{
  struct iio_trigger *trig;
  int ret;
  
  iio_trig_timer = kmalloc(sizeof(*iio_trig_timer), GFP_KERNEL);
  if (iio_trig_timer == NULL) {
    ret = -ENOMEM;
    goto error_free_trig_timer;
  }

  trig = iio_trigger_alloc("timertrig");
  if (!trig) {
    ret = -ENOMEM;
    goto error_free_trig;
  }
  iio_trig_timer->trig = trig;
  trig->dev.bus = &iio_bus_type;
  trig->dev.groups = iio_trig_timer_groups;
  trig->dev.release = &iio_trig_timer_release;
  trig->ops = &iio_trig_timer_ops;
  iio_trigger_set_drvdata(trig, iio_trig_timer);

  /* RTC access */
  iio_trig_timer->rtc = rtc_class_open("rtc0");
  if (!iio_trig_timer->rtc) {
    ret = -EINVAL;
    goto error_free_trig;
  }
  iio_trig_timer->task.func = iio_rtc_trigger_poll;
  iio_trig_timer->task.private_data = trig;
  ret = rtc_irq_register(iio_trig_timer->rtc, &iio_trig_timer->task);
  if (ret < 0)
    goto error_close_rtc;
  ret = rtc_irq_set_freq(iio_trig_timer->rtc, &iio_trig_timer->task, 1);
  if (ret < 0)
    goto error_unregister_rtc_irq;
  ret = rtc_irq_set_state(iio_trig_timer->rtc, &iio_trig_timer->task, 1);
  if (ret < 0)
    goto error_unregister_rtc_irq;

  ret = iio_trigger_register(trig);
  if (ret)
    goto error_unregister_rtc_irq;

  printk(KERN_INFO "ok\n");
  return 0;

error_unregister_rtc_irq:
  rtc_irq_unregister(iio_trig_timer->rtc, &iio_trig_timer->task);
error_close_rtc:
  rtc_class_close(iio_trig_timer->rtc);
  iio_trigger_put(trig);
error_free_trig:
  kfree(iio_trig_timer);
error_free_trig_timer:
  return ret;
}
module_init(iio_trig_timer_init);

static void __exit iio_trig_timer_exit(void)
{
  rtc_irq_unregister(iio_trig_timer->rtc, &iio_trig_timer->task);
  rtc_class_close(iio_trig_timer->rtc);
  iio_trigger_unregister(iio_trig_timer->trig);
  iio_trigger_free(iio_trig_timer->trig);
  kfree(iio_trig_timer);
  printk(KERN_INFO "removed\n");
}
module_exit(iio_trig_timer_exit);

MODULE_AUTHOR("Matija Podravec <matija_podravec@fastmail.fm>");
MODULE_DESCRIPTION("Timer based trigger for the iio subsystem");
MODULE_LICENSE("GPL");
