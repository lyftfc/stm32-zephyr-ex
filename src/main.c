// SPDX-License-Identifier: Apache-2.0

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
// #include <zephyr/drivers/led.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usbd_hid.h>

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
	     "Console device is not ACM CDC UART device");
static bool usb_con_ready = false;

static void error_loop (void)
{
	while (1) {
		gpio_pin_toggle_dt(&led);
		k_msleep(100);
	}
}

static void usb_dc_sts_cb (enum usb_dc_status_code cb_status, const uint8_t *param)
{
	ARG_UNUSED(param);
	if (cb_status == USB_DC_CONNECTED || cb_status == USB_DC_CONFIGURED)
		usb_con_ready = true;
	else if (cb_status != USB_DC_RESUME && cb_status != USB_DC_INTERFACE && cb_status != USB_DC_SOF)
		usb_con_ready = false;
	// gpio_pin_set_dt(&led_y, usb_con_ready ? 1 : 0);
}

static void led_timer_cb (const struct device *dev, uint8_t chan, uint32_t ticks, void *p_usr)
{
	printk("Timer interrupt channel %d\n", chan);
	gpio_pin_toggle_dt(&led);
	// re-arm the alarm
	struct counter_alarm_cfg *acfg = (struct counter_alarm_cfg *)p_usr;
	counter_set_channel_alarm(dev, chan, acfg);
}

int main(void)
{
	int ret;
	uint32_t dtr;

	const struct device *const usb_cdc_con = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uart_line_ctrl_get(usb_cdc_con, UART_LINE_CTRL_DTR, &dtr);

	if (!gpio_is_ready_dt(&led)) { return 0; }
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) { return 0; }

	const struct device *const led_timer = DEVICE_DT_GET(DT_NODELABEL(counter2));
	if (!device_is_ready(led_timer)) error_loop();
	struct counter_top_cfg cntop_cfg = {
		.ticks = 10000, .callback = NULL, .user_data = NULL, .flags = 0
	};
	counter_set_top_value(led_timer, &cntop_cfg);
	counter_start(led_timer);

	struct counter_alarm_cfg alarm_cfg[4];
	for (int i = 0; i < 4; ++i) {
        alarm_cfg[i] = (struct counter_alarm_cfg) {
            .flags = COUNTER_ALARM_CFG_ABSOLUTE,
            .ticks = (i + 1) * 500,  // 500, 1000, 1500, 2000
            .callback = led_timer_cb,
            .user_data = &alarm_cfg[i]
        };
        counter_set_channel_alarm(led_timer, i, &alarm_cfg[i]);
    }

	ret = usb_enable(usb_dc_sts_cb);
	if (ret < 0) error_loop();

	while (1) 
    {
		if (usb_con_ready)
			printk("Hello\n");

		k_sleep(K_MSEC(500U));
	}
	return 0;
}
