/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mmio.h>

#ifdef SPD_trusty
#define GPIO1_BASE_ADDR         0X30200000
#define GPIO2_BASE_ADDR         0x30210000
#define GPIO3_BASE_ADDR         0x30220000
#define GPIO4_BASE_ADDR         0x30230000
#define GPIO5_BASE_ADDR         0x30240000

/* layout of baseboard id */
#define IMX8MQ_GPIO3_IO24 88  //board_id[2]:2
#define IMX8MQ_GPIO3_IO22 86  //board_id[2]:1
#define IMX8MQ_GPIO3_IO19 83  //board_id[2]:0

struct gpio_regs {
	unsigned int  gpio_dr;	/* data */
	unsigned int  gpio_dir;	/* direction */
	unsigned int  gpio_psr;	/* pad satus */
};

/* GPIO port description */
static unsigned long imx8m_gpio_ports[] = {
	[0] = GPIO1_BASE_ADDR,
	[1] = GPIO2_BASE_ADDR,
	[2] = GPIO3_BASE_ADDR,
	[3] = GPIO4_BASE_ADDR,
	[4] = GPIO5_BASE_ADDR,
};

static int gpio_direction_input_legacy(unsigned int gpio)
{
	unsigned int port;
	struct gpio_regs *regs;
	unsigned int  l;

	port = gpio/32;
	gpio &= 0x1f;
	regs = (struct gpio_regs *)imx8m_gpio_ports[port];
	l = mmio_read_32((unsigned long)&regs->gpio_dir);
	/* set direction as input. */
	l &= ~(1 << gpio);
	mmio_write_32((unsigned long)&regs->gpio_dir, l);

	return 0;
}

static unsigned int gpio_get_value_legacy(unsigned gpio)
{
	unsigned int port;
	struct gpio_regs *regs;
	unsigned int  val;

	port = gpio/32;
	gpio &= 0x1f;
	regs = (struct gpio_regs *)imx8m_gpio_ports[port];
	val = (mmio_read_32((unsigned long)&regs->gpio_dr) >> gpio) & 0x01;

	return val;
}

int get_imx8m_baseboard_id(void)
{
	unsigned int value = 0;
	int  i = 0, baseboard_id = 0;
	int pin[3];

	/* initialize the pin array */
	pin[0] = IMX8MQ_GPIO3_IO19;
	pin[1] = IMX8MQ_GPIO3_IO22;
	pin[2] = IMX8MQ_GPIO3_IO24;

	/* Set gpio direction as input and get the input value */
	baseboard_id = 0;
	for (i = 0; i < 3; i++) {
		gpio_direction_input_legacy(pin[i]);
		value = gpio_get_value_legacy(pin[i]);
		baseboard_id |= ((value & 0x01) << i);
	}

	return baseboard_id;
}
#endif /* SPD_trusty */
