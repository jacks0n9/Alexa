/* drivers/leds/leds-omap_pwm.c
 *
 * Driver to blink LEDs using OMAP PWM timers
 *
 * Copyright (C) 2006 Nokia Corporation
 * Author: Timo Teras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/ctype.h>
#include <linux/sched.h>
#include <linux/clk.h>
#include <asm/delay.h>
#include <plat/board.h>
#include <plat/dmtimer.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <asm/atomic.h>

#define MIN_PULSE_BRIGHTNESS(x)	((x*4)/100 ) /* ~4% of brightness */
#define MIN_PULSE_ON_PERIOD_US	1000 /* 1-ms */
#define DEFAULT_ON_PERIOD_US	3780000
#define MAX_TIME(x,y)	((x >= y) ? x : y)
#define RAMP_TO_BRIGHTNESS(x,y)	((x*(y))/100)

struct omap_pwm_thread {
	struct task_struct *task;
	wait_queue_head_t wait_q;
	void *priv;
	struct mutex wait_lock;
};

struct omap_pwm_led {
	struct led_classdev cdev;
	struct omap_pwm_thread pulse_thread;
	struct mutex update_lock;
	struct omap_pwm_led_platform_data *pdata;
	struct omap_dm_timer *timer;
	enum  omap_pwm_timer_type timer_type;
	int powered;
	int resumed;
	unsigned int on_period, off_period;
	atomic_t brightness;
};

static u8 g_ramp_table[] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 7, 7,
	7, 8, 9, 9, 10, 11, 11, 12, 13, 14, 15, 16, 18, 19, 20,
	22, 24, 25, 27, 30, 32, 34, 36, 39, 42, 45, 47, 50, 53,
	56, 59, 63, 66, 69, 72, 75, 78, 81, 84, 87, 89, 92, 95,
	97, 100
};

static inline struct omap_pwm_led *pdev_to_omap_pwm_led(
		struct platform_device *pdev)
{
	return platform_get_drvdata(pdev);
}

static inline struct omap_pwm_led *cdev_to_omap_pwm_led(
		struct led_classdev *led_cdev)
{
	return container_of(led_cdev, struct omap_pwm_led, cdev);
}

static void omap_pwm_led_set_blink(struct omap_pwm_led *led)
{
	if (!led->powered)
		return;

	if (led->on_period != 0 && led->off_period != 0) {
		unsigned long load_reg, cmp_reg;

		load_reg = 32768 * (led->on_period + led->off_period) / 1000;
		cmp_reg = 32768 * led->on_period / 1000;

		omap_dm_timer_stop(led->timer);
		omap_dm_timer_set_load(led->timer, 1, -load_reg);
		omap_dm_timer_set_match(led->timer, 1, -cmp_reg);
		omap_dm_timer_set_pwm(led->timer, 1, 1,
				OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);
		omap_dm_timer_write_counter(led->timer, -2);
		omap_dm_timer_start(led->timer);
	} else {
		omap_dm_timer_set_pwm(led->timer, 1, 1,
				OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);
		omap_dm_timer_stop(led->timer);
	}
}

static void omap_pwm_led_power_on(struct omap_pwm_led *led)
{
	if (led->powered)
		return;

	led->powered = 1;

	/* Enable the timer */
	omap_dm_timer_enable(led->timer);

	switch (led->timer_type) {
	case OMAP_PWM_TIMER_INTENSITY:
	case OMAP_PWM_TIMER_PULSE:
		/* Select sys clock */
		omap_dm_timer_set_source(led->timer, OMAP_TIMER_SRC_SYS_CLK);
		break;
	case OMAP_PWM_TIMER_BLINKER:
		/* Select 32 Khz clock */
		omap_dm_timer_set_source(led->timer, OMAP_TIMER_SRC_32_KHZ);
		omap_pwm_led_set_blink(led);
		break;
	default:
		omap_dm_timer_disable(led->timer);
		led->powered = 0;
		return;
	}

	/* Turn voltage on */
	if (led->pdata->set_power != NULL)
		led->pdata->set_power(led->pdata, 1);

}

static void omap_pwm_led_power_off(struct omap_pwm_led *led)
{
	if (!led->powered)
		return;
	led->powered = 0;
	/* Everything off */
	if (led->timer) {
		omap_dm_timer_stop(led->timer);
		omap_dm_timer_disable(led->timer);
	}

	if (led->pdata->set_power != NULL)
		led->pdata->set_power(led->pdata, 0);
}

static void pwm_set_speed(struct omap_dm_timer *gpt, int frequency,
		int duty_cycle)
{
	u32 val;
	u32 period;
	struct clk *timer_fclk;

	/* and you will have an overflow in 1 sec         */
	/* so,                              */
	/* freq_timer     -> 1s             */
	/* carrier_period -> 1/carrier_freq */
	/* => carrier_period = freq_timer/carrier_freq */

	timer_fclk = omap_dm_timer_get_fclk(gpt);
	period = clk_get_rate(timer_fclk) / frequency;

	val = 0xFFFFFFFF + 1 - period;
	omap_dm_timer_set_load(gpt, 1, val);

	val = 0xFFFFFFFF + 1 - (period * duty_cycle / 256);
	omap_dm_timer_set_match(gpt, 1, val);

	/* assume overflow first: no toogle if first trig is match */
	omap_dm_timer_write_counter(gpt, 0xFFFFFFFE);
}

static void omap_pwm_led_set_pwm_cycle(struct omap_pwm_led *led, int cycle)
{
	int pwm_frequency = 10000;

	if (led->pdata->max)
		cycle = ((cycle * led->pdata->max) / 255);

	if (led->pdata->freq)
		pwm_frequency = led->pdata->freq;

	if (cycle == LED_FULL) {
		if (led->pdata->invert)
			omap_dm_timer_set_pwm(led->timer, 0, 1,
					OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);
		else
			omap_dm_timer_set_pwm(led->timer, 1, 1,
					OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);
		omap_dm_timer_stop(led->timer);
	} else {
		if (led->pdata->invert)
			omap_dm_timer_set_pwm(led->timer, 1, 1,
					OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);
		else
			omap_dm_timer_set_pwm(led->timer, 0, 1,
					OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);

		pwm_set_speed(led->timer, pwm_frequency, 256 - cycle);
		omap_dm_timer_start(led->timer);
	}
}

static void omap_pwm_led_apply(struct omap_pwm_led *led)
{
	/* Note - The brightness does not change for blinkers - TODO */
	if (atomic_read(&led->brightness) != LED_OFF) {
		omap_pwm_led_power_on(led);
		if(led->timer_type == OMAP_PWM_TIMER_INTENSITY ||
				led->timer_type == OMAP_PWM_TIMER_PULSE)
			omap_pwm_led_set_pwm_cycle(led, atomic_read(&led->brightness));
	} else {
		if (led->pdata->invert && led->resumed) {
			omap_pwm_led_power_on(led);
			if (led->timer_type == OMAP_PWM_TIMER_INTENSITY ||
					led->timer_type == OMAP_PWM_TIMER_PULSE)
				omap_pwm_led_set_pwm_cycle(led, atomic_read(&led->brightness));
		} else {
			omap_pwm_led_power_off(led);
		}
	}

	led->resumed = 0;
}

static void omap_pwm_led_set(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	struct omap_pwm_led *led = cdev_to_omap_pwm_led(led_cdev);

	if (OMAP_PWM_TIMER_PULSE == led->timer_type && led->on_period > 0) {
		/* Actual pulsing behavior is triggered by setting an 'on period'
		 * prior to setting a brightness value */

		/* Prevent multiple updates race */
		mutex_lock(&led->update_lock);
		if (0 == atomic_read(&led->brightness) && value != 0) {
			/* If led was previously off, update the brightness value
			 * and trigger the pulse thread
			 */
			atomic_set(&led->brightness, value);
			wake_up_interruptible(&led->pulse_thread.wait_q);
		} else {
			// Thread is running, update the brightness value
			atomic_set(&led->brightness, value);
			if (0 == atomic_read(&led->brightness)) {
				// Wait for the pulse thread to go to sleep
				mutex_lock(&led->pulse_thread.wait_lock);
				// LED turned off. Reset the periods
				led->on_period = 0;
				led->off_period = 0;
				mutex_unlock(&led->pulse_thread.wait_lock);
			}
		}
		mutex_unlock(&led->update_lock);
	} else {
		atomic_set(&led->brightness, value);
		/* cut down latency : we need pwm configured asap */
		omap_pwm_led_apply(led);
	}
}

static int omap_pwm_led_pulse_ramp(struct omap_pwm_led *led, unsigned long initial_brightness,
		unsigned long target_brightness, unsigned long ramp_period_us)
{
	/* Determine ramp direction */
	int index = 0;
	int ramping = 1;
	int positive_ramp = initial_brightness < target_brightness ? 1 : 0;
	long ramp_value;
	/* Estimate sleep time per ramp cycle */
	unsigned long num_steps = ARRAY_SIZE(g_ramp_table);
	unsigned long sleep_time = 0;

	/* Ensure we have a valid sleep time */
	if (num_steps && ramp_period_us >= MIN_PULSE_ON_PERIOD_US)
		sleep_time = ramp_period_us / num_steps;
	else
		return -1;

	/* Set the ramp table initial index */
	if (positive_ramp)
		/* Start ramp from 0 index */
		index = 0;
	else
		index = ARRAY_SIZE(g_ramp_table) - 1;

	/* Within the a very coarse period, ramp to the peak value */
	while (ramping && (atomic_read(&led->brightness) != LED_OFF) && num_steps--) {

		if (positive_ramp) {
			/* Set the ramp value */
			ramp_value = RAMP_TO_BRIGHTNESS(g_ramp_table[index++], target_brightness);
			if (ramp_value >= target_brightness || index >= ARRAY_SIZE(g_ramp_table)) {
				ramp_value = target_brightness;
				/* Exit */
				ramping = 0;
			}
		} else {
			/* Set the ramp value */
			ramp_value = RAMP_TO_BRIGHTNESS(g_ramp_table[index--], initial_brightness);
			if (ramp_value <= 0 || index <= 0) {
				ramp_value = target_brightness;
				/* Exit */
				ramping = 0;
			}
		}

		/* Request for the pwm update */
		omap_pwm_led_set_pwm_cycle(led, ramp_value);
		/* Sleep */
		usleep_range(sleep_time, sleep_time);
	}

	return 0;
}

static int omap_pwm_led_pulse_thread(void *data)
{
	struct omap_pwm_thread *thread = (struct omap_pwm_thread *)data;
	struct omap_pwm_led *led = (struct omap_pwm_led *)thread->priv;
	unsigned long target_brightness = 0;
	int ret = 0;

	mutex_lock(&thread->wait_lock);

	while (!kthread_should_stop()) {

		if (atomic_read(&led->brightness) == LED_OFF || ret) {
			// Disable the led
			omap_pwm_led_power_off(led);
			// Release the wait lock
			mutex_unlock(&thread->wait_lock);
			// Wait for brightness update event
			wait_event_interruptible(thread->wait_q,
					(atomic_read(&led->brightness) != LED_OFF));
			// Hold the wait lock
			mutex_lock(&thread->wait_lock);
			/* Clear the error */
			ret = 0;
		}

		if(kthread_should_stop())
			break;
		else {
			if (0 == led->powered)
				omap_pwm_led_power_on(led);
			/* ramp up for half of the on time */
			ret = omap_pwm_led_pulse_ramp(
					led, MIN_PULSE_BRIGHTNESS(atomic_read(&led->brightness)),
					atomic_read(&led->brightness),
					MAX_TIME(led->on_period / 2, MIN_PULSE_ON_PERIOD_US));
			if (0 != ret) {
				/* Avoid a busy wait - return to wait event (reset needed)*/
				continue;
			}

			/* ramp down the other half of the time */
			/* If there's an off-period, ramp down all the way to 0 intensity */
			target_brightness = (led->off_period != 0) ? 0
					: MIN_PULSE_BRIGHTNESS(atomic_read(&led->brightness));

			ret = omap_pwm_led_pulse_ramp(
					led, atomic_read(&led->brightness),
					target_brightness,
					MAX_TIME(led->on_period / 2, MIN_PULSE_ON_PERIOD_US));
			if (0 != ret) {
				/* Avoid a busy wait - return to wait event (reset needed)*/
				continue;
			}

			/* Sleep for the off time */
			if(led->off_period) {
				usleep_range(led->off_period, led->off_period );
			}
		}
	}

	omap_pwm_led_power_off(led);

	// Release the wait lock
	mutex_unlock(&thread->wait_lock);

	return 0;
}

static ssize_t omap_pwm_led_on_period_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct omap_pwm_led *led = cdev_to_omap_pwm_led(led_cdev);

	return sprintf(buf, "%u\n", led->on_period/1000) + 1; /* print ms */
}

static ssize_t omap_pwm_led_on_period_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct omap_pwm_led *led = cdev_to_omap_pwm_led(led_cdev);
	int ret = -EINVAL;
	unsigned long val;
	char *after;
	size_t count;

	val = simple_strtoul(buf, &after, 10);
	count = after - buf;
	if (*after && isspace(*after))
		count++;

	if (count == size) {
		led->on_period = val * 1000; /* ms -> us */
		if(led->timer_type == OMAP_PWM_TIMER_BLINKER)
			omap_pwm_led_set_blink(led);
		ret = count;
	}

	return ret;
}

static ssize_t omap_pwm_led_off_period_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct omap_pwm_led *led = cdev_to_omap_pwm_led(led_cdev);

	return sprintf(buf, "%u\n", led->off_period/1000) + 1; /* print ms */
}

static ssize_t omap_pwm_led_off_period_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct omap_pwm_led *led = cdev_to_omap_pwm_led(led_cdev);
	int ret = -EINVAL;
	unsigned long val;
	char *after;
	size_t count;

	val = simple_strtoul(buf, &after, 10);
	count = after - buf;
	if (*after && isspace(*after))
		count++;

	if (count == size) {
		led->off_period = val * 1000; /* ms -> us */
		if(led->timer_type == OMAP_PWM_TIMER_BLINKER)
			omap_pwm_led_set_blink(led);
		ret = count;
	}

	return ret;
}

static DEVICE_ATTR(on_period, 0644, omap_pwm_led_on_period_show,
		omap_pwm_led_on_period_store);
static DEVICE_ATTR(off_period, 0644, omap_pwm_led_off_period_show,
		omap_pwm_led_off_period_store);

static int omap_pwm_led_probe(struct platform_device *pdev)
{
	struct omap_pwm_led_platform_data *pdata = pdev->dev.platform_data;
	struct omap_pwm_led *led;
	int ret = 0;

	led = kzalloc(sizeof(struct omap_pwm_led), GFP_KERNEL);
	if (led == NULL) {
		dev_err(&pdev->dev, "No memory for device\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, led);
	led->cdev.brightness_set = omap_pwm_led_set;
	led->cdev.default_trigger = pdata->default_trigger;
	led->cdev.name = pdata->name;
	led->pdata = pdata;
	atomic_set(&led->brightness, LED_OFF);
	led->powered = 0;
	led->timer_type = pdata->timer_type;

	dev_info(&pdev->dev, "OMAP PWM LED (%s) at GP timer %d\n", pdata->name,
			pdata->timer_id);

	/* register our new led device */
	ret = led_classdev_register(&pdev->dev, &led->cdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "led_classdev_register failed\n");
		goto error_classdev;
	}

	/* Request the dm timer */
	led->timer = omap_dm_timer_request_specific(pdata->timer_id);
	if (led->timer == NULL) {
		dev_err(&pdev->dev, "failed to request pwm timer\n");
		ret = -ENODEV;
		goto error_timer;
	}

	switch (led->pdata->timer_type) {
	case OMAP_PWM_TIMER_PULSE:
		/* Initialize the ramp thread */
		led->pulse_thread.priv = led;
		init_waitqueue_head(&led->pulse_thread.wait_q);
		led->pulse_thread.task = kthread_run(omap_pwm_led_pulse_thread,
				&led->pulse_thread, "kpulse/%d", pdata->timer_id);
		/* Fall through */
	case OMAP_PWM_TIMER_BLINKER:
		/* Create On/Off Period attribute files */
		led->on_period = 0;
		led->off_period = 0;
		ret = device_create_file(led->cdev.dev, &dev_attr_on_period);
		if (ret)
			goto error_on_period;

		ret = device_create_file(led->cdev.dev, &dev_attr_off_period);
		if (ret)
			goto error_off_period;
		break;

	case OMAP_PWM_TIMER_INTENSITY:
		if (led->pdata->invert)
			omap_dm_timer_set_pwm(led->timer, 1, 1,
					OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);
		break;

	default:
		dev_err(&pdev->dev, "invalid timer type - failed\n");
		ret = -EINVAL;
		goto error_timer_free;
	}

	omap_dm_timer_disable(led->timer);

	if (led->pdata->brightness) {
		atomic_set(&led->brightness, pdata->brightness);

		if (OMAP_PWM_TIMER_PULSE == led->pdata->timer_type) {
			led->on_period = DEFAULT_ON_PERIOD_US;
			wake_up_interruptible(&led->pulse_thread.wait_q);
		} else
			omap_pwm_led_apply(led);
	}

	mutex_init(&led->update_lock);
	mutex_init(&led->pulse_thread.wait_lock);

	return 0;

	error_off_period: device_remove_file(led->cdev.dev, &dev_attr_on_period);
	error_on_period: dev_err(&pdev->dev, "failed to create device file(s)\n");
	error_timer_free: omap_dm_timer_free(led->timer);
	error_timer: led_classdev_unregister(&led->cdev);
	error_classdev: kfree(led);
	return ret;
}

static int omap_pwm_led_remove(struct platform_device *pdev)
{
	struct omap_pwm_led *led = pdev_to_omap_pwm_led(pdev);
	device_remove_file(led->cdev.dev, &dev_attr_on_period);
	device_remove_file(led->cdev.dev, &dev_attr_off_period);
	led_classdev_unregister(&led->cdev);
	/* Stop the pulse thread */
	if (OMAP_PWM_TIMER_PULSE == led->timer_type)
		kthread_stop(led->pulse_thread.task);
	/* Turn off the led */
	omap_pwm_led_set(&led->cdev, LED_OFF);
	if (led->timer != NULL)
		omap_dm_timer_free(led->timer);

	mutex_destroy(&led->update_lock);
	kfree(led);

	return 0;
}

#ifdef CONFIG_PM
static int omap_pwm_led_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct omap_pwm_led *led = pdev_to_omap_pwm_led(pdev);

	led_classdev_suspend(&led->cdev);
	return 0;
}

static int omap_pwm_led_resume(struct platform_device *pdev)
{
	struct omap_pwm_led *led = pdev_to_omap_pwm_led(pdev);

	led->resumed = 1;
	led_classdev_resume(&led->cdev);
	return 0;
}
#else
#define omap_pwm_led_suspend NULL
#define omap_pwm_led_resume NULL
#endif

static struct platform_driver omap_pwm_led_driver = {
		.probe = omap_pwm_led_probe,
		.remove = omap_pwm_led_remove,
		.suspend = omap_pwm_led_suspend,
		.resume = omap_pwm_led_resume,
		.driver = {
				.name = "omap_pwm_led",
				.owner = THIS_MODULE,
		},
};

static int __init omap_pwm_led_init(void)
{
	return platform_driver_register(&omap_pwm_led_driver);
}

static void __exit omap_pwm_led_exit(void)
{
	platform_driver_unregister(&omap_pwm_led_driver);
}

module_init (omap_pwm_led_init);
module_exit (omap_pwm_led_exit);

MODULE_AUTHOR("Timo Teras");
MODULE_DESCRIPTION("OMAP PWM LED driver");
MODULE_LICENSE("GPL");
