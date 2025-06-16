// SPDX-License-Identifier: Apache-2.0

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led_y = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec btn_b0 = GPIO_DT_SPEC_GET(DT_ALIAS(usrbtn), gpios);

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

void usb_dc_sts_cb (enum usb_dc_status_code cb_status, const uint8_t *param)
{
	ARG_UNUSED(param);
	if (cb_status == USB_DC_CONNECTED || cb_status == USB_DC_CONFIGURED)
		usb_con_ready = true;
	else if (cb_status != USB_DC_RESUME && cb_status != USB_DC_INTERFACE && cb_status != USB_DC_SOF)
		usb_con_ready = false;
	gpio_pin_set_dt(&led_y, usb_con_ready ? 1 : 0);
}

int main(void)
{
	int ret;
	uint32_t dtr;

	const __unused struct device *const usb_cdc_con = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uart_line_ctrl_get(usb_cdc_con, UART_LINE_CTRL_DTR, &dtr);

	if (!gpio_is_ready_dt(&led)) { return 0; }
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) { return 0; }

	if (!gpio_is_ready_dt(&led_y)) { return 0; }
	ret = gpio_pin_configure_dt(&led_y, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) { return 0; }
	gpio_pin_set_dt(&led_y, 0);
	if (ret < 0) { return 0; }

	ret = usb_enable(usb_dc_sts_cb);
	if (ret < 0) error_loop();

	while (1) 
    {
		if (usb_con_ready)
			printk("Hello\n");

		int btn_st = gpio_pin_get_dt(&btn_b0);
		if (!btn_st) {
			ret = gpio_pin_toggle_dt(&led);
			if (ret < 0) { return 0; }
		}

		k_sleep(K_MSEC(500U));
	}
	return 0;
}
