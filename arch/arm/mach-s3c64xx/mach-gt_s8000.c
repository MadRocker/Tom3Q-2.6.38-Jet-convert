/* linux/arch/arm/mach-s3c64xx/mach-gt_S8000.c
 *
 * Copyright 2011 Tomasz Figa <tomasz.figa at gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/leds.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/max8906.h>
#include <linux/android_pmem.h>
#include <linux/reboot.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/switch.h>
#include <linux/fsa9480.h>
#include <linux/clk.h>
#include <linux/mmc/host.h>
#include <linux/gpio_keys.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>
#include <linux/mtd/partitions.h>
#include <linux/power/samsung_battery.h>
#include <linux/power/gpio-charger.h>
#include <linux/input/s3c_ts.h>

#include <video/ams310fs07.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/regs-fb.h>
#include <mach/map.h>

#include <asm/irq.h>
#include <asm/mach-types.h>
#include <asm/cpu-single.h>
#include <asm/setup.h>

#include <plat/regs-serial.h>
#include <plat/iic.h>
#include <plat/fb.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/adc.h>
#include <plat/ts.h>
#include <plat/sdhci.h>
#include <plat/keypad.h>
#include <plat/pm.h>


#include <mach/regs-modem.h>
#include <mach/regs-gpio.h>
#include <mach/regs-sys.h>
#include <mach/regs-srom.h>
#include <mach/gpio-cfg.h>
#include <mach/s3c6410.h>
#include <mach/gt_S8000.h>
#include <mach/regs-gpio-memport.h>
#include <mach/regs-gpio.h>
#include <mach/regs-clock.h>

/*
 * UART
 */

#define UCON S3C2410_UCON_DEFAULT
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE

static struct s3c2410_uartcfg jet_uartcfgs[] __initdata = {
	[0] = {	/* Phone */
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[1] = {	/* Bluetooth */
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[2] = {	/* Serial */
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
};

/*
 * USB gadget
 */

#include <linux/usb/android_composite.h>
#define S3C_VENDOR_ID			0x18d1
#define S3C_UMS_PRODUCT_ID		0x4E21
#define S3C_UMS_ADB_PRODUCT_ID		0x4E22
#define S3C_RNDIS_PRODUCT_ID		0x4E23
#define S3C_RNDIS_ADB_PRODUCT_ID	0x4E24
#define MAX_USB_SERIAL_NUM	17

static char *usb_functions_ums[] = {
	"usb_mass_storage",
};

static char *usb_functions_rndis[] = {
	"rndis",
};

static char *usb_functions_rndis_adb[] = {
	"rndis",
	"adb",
};
static char *usb_functions_ums_adb[] = {
	"usb_mass_storage",
	"adb",
};

static char *usb_functions_all[] = {
	"rndis",
	"usb_mass_storage",
	"adb",
};

static struct android_usb_product usb_products[] = {
	{
		.product_id	= S3C_UMS_PRODUCT_ID,
		.num_functions	= ARRAY_SIZE(usb_functions_ums),
		.functions	= usb_functions_ums,
	},
	{
		.product_id	= S3C_UMS_ADB_PRODUCT_ID,
		.num_functions	= ARRAY_SIZE(usb_functions_ums_adb),
		.functions	= usb_functions_ums_adb,
	},
	{
		.product_id	= S3C_RNDIS_PRODUCT_ID,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis),
		.functions	= usb_functions_rndis,
	},
	{
		.product_id	= S3C_RNDIS_ADB_PRODUCT_ID,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis_adb),
		.functions	= usb_functions_rndis_adb,
	},
};

static char device_serial[MAX_USB_SERIAL_NUM] = "0123456789ABCDEF";
/* standard android USB platform data */

/* Information should be changed as real product for commercial release */
static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id		= S3C_VENDOR_ID,
	.product_id		= S3C_UMS_PRODUCT_ID,
	.manufacturer_name	= "Samsung",
	.product_name		= "Galaxy GT-S8000",
	.serial_number		= device_serial,
	.num_products		= ARRAY_SIZE(usb_products),
	.products		= usb_products,
	.num_functions		= ARRAY_SIZE(usb_functions_all),
	.functions		= usb_functions_all,
};

struct platform_device jet_android_usb = {
	.name	= "android_usb",
	.id	= -1,
	.dev	= {
		.platform_data	= &android_usb_pdata,
	},
};

static struct usb_mass_storage_platform_data ums_pdata = {
	.vendor			= "Android",
	.product		= "UMS Composite",
	.release		= 1,
	.nluns			= 1,
};

struct platform_device jet_usb_mass_storage = {
	.name	= "usb_mass_storage",
	.id	= -1,
	.dev	= {
		.platform_data = &ums_pdata,
	},
};

static struct usb_ether_platform_data rndis_pdata = {
	/* ethaddr is filled by board_serialno_setup */
	.vendorID	= 0x18d1,
	.vendorDescr	= "Samsung",
};

struct platform_device jet_usb_rndis = {
	.name	= "rndis",
	.id	= -1,
	.dev	= {
		.platform_data = &rndis_pdata,
	},
};

/*
 * I2C devices
 */

/* I2C 0 (hardware) -	FSA9480 (USB transceiver),
 *			BMA023 (accelerometer),
 * 			AK8973B (magnetometer) */
static struct s3c2410_platform_i2c jet_misc_i2c __initdata = {
	.flags		= 0,
	.slave_addr	= 0x10,
	.frequency	= 100*1000,
	.sda_delay	= 100,
	.bus_num	= 0,
};

static struct fsa9480_platform_data jet_fsa9480_pdata = {
};

static struct i2c_board_info jet_misc_i2c_devs[] __initdata = {
	{
		.type		= "fsa9480",
		.addr		= 0x25,
		.irq		= IRQ_EINT(9),
		.platform_data	= &jet_fsa9480_pdata,
	},
	{
		.type		= "bma023",
		.addr		= 0x38,
		.irq		= IRQ_EINT(3),
	},
	{
		.type		= "ak8973",
		.addr		= 0x1c,
		.irq		= IRQ_EINT(2),
	},
};

/* I2C 1 (hardware) -	UNKNOWN (camera sensor) */
static struct s3c2410_platform_i2c jet_cam_i2c __initdata = {
	.flags		= 0,
	.slave_addr	= 0x10,
	.frequency	= 100*1000,
	.sda_delay	= 100,
	.bus_num	= 1,
};

static struct i2c_board_info jet_cam_i2c_devs[] __initdata = {
	/* TODO */
};

/* I2C 2 (GPIO) -	MAX8906EWO-T (voltage regulator) */
static struct i2c_gpio_platform_data jet_pmic_i2c_pdata = {
	.sda_pin		= GPIO_PWR_I2C_SDA,
	.scl_pin		= GPIO_PWR_I2C_SCL,
	.udelay			= 2, /* 250KHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 1,
};

static struct platform_device jet_pmic_i2c = {
	.name			= "i2c-gpio",
	.id			= 2,
	.dev.platform_data	= &jet_pmic_i2c_pdata,
};

static struct regulator_init_data jet_ldo2_data = {
	.constraints	= {
		.name			= "VAP_ALIVE_1.2V",
		.min_uV			= 1200000,
		.max_uV			= 1200000,
		.apply_uV		= 0,
		.always_on		= 1,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
		.state_mem		= {
			.enabled = 1,
		},
	},
};

static struct regulator_consumer_supply ldo3_consumer[] = {
	REGULATOR_SUPPLY("otg-io", "s3c-hsotg")
};

static struct regulator_init_data jet_ldo3_data = {
	.constraints	= {
		.name			= "VAP_OTGI_1.2V",
		.min_uV			= 1200000,
		.max_uV			= 1200000,
		.apply_uV		= 0,
		.valid_ops_mask 	= REGULATOR_CHANGE_STATUS,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo3_consumer),
	.consumer_supplies	= ldo3_consumer,
};

static struct regulator_init_data jet_ldo4_data = {
	.constraints	= {
		.name			= "VLED_3.3V",
		.min_uV			= 3300000,
		.max_uV			= 3300000,
		.apply_uV		= 0,
		.always_on		= 1,
		.valid_ops_mask 	= REGULATOR_CHANGE_STATUS,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
		.state_mem		= {
			.disabled = 1,
		},
	},
};

static struct regulator_consumer_supply ldo5_consumer[] = {
	REGULATOR_SUPPLY("vmmc", "s3c-sdhci.0")
};

static struct regulator_init_data jet_ldo5_data = {
	.constraints	= {
		.name			= "VTF_3.0V",
		.min_uV			= 3000000,
		.max_uV			= 3000000,
		.apply_uV		= 0,
		.valid_ops_mask 	= REGULATOR_CHANGE_STATUS,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo5_consumer),
	.consumer_supplies	= ldo5_consumer,
};

static struct regulator_consumer_supply ldo6_consumer[] = {
	{	.supply	= "lcd_vdd3", },
};

static struct regulator_init_data jet_ldo6_data = {
	.constraints	= {
		.name			= "VLCD_1.8V",
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.apply_uV		= 0,
		.valid_ops_mask 	= REGULATOR_CHANGE_STATUS,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo6_consumer),
	.consumer_supplies	= ldo6_consumer,
};

static struct regulator_consumer_supply ldo7_consumer[] = {
	{	.supply	= "lcd_vci", },
};

static struct regulator_init_data jet_ldo7_data = {
	.constraints	= {
		.name			= "VLCD_3.0V",
		.min_uV			= 3000000,
		.max_uV			= 3000000,
		.apply_uV		= 0,
		.valid_ops_mask 	= REGULATOR_CHANGE_STATUS,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo7_consumer),
	.consumer_supplies	= ldo7_consumer,
};

static struct regulator_consumer_supply ldo8_consumer[] = {
	REGULATOR_SUPPLY("otg-core", "s3c-hsotg")
};

static struct regulator_init_data jet_ldo8_data = {
	.constraints	= {
		.name			= "VAP_OTG_3.3V",
		.min_uV			= 3300000,
		.max_uV			= 3300000,
		.apply_uV		= 0,
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo8_consumer),
	.consumer_supplies	= ldo8_consumer,
};

static struct regulator_init_data jet_ldo9_data = {
	.constraints	= {
		.name			= "VAP_SYS_3.0V",
		.min_uV			= 3000000,
		.max_uV			= 3000000,
		.apply_uV		= 0,
		.always_on		= 1,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
	},
};

static struct regulator_consumer_supply buck1_consumer[] = {
	{	.supply	= "vddarm", },
};

static struct regulator_init_data jet_buck1_data = {
	.constraints	= {
		.name			= "VAP_ARM",
		.min_uV			= 750000,
		.max_uV			= 1500000,
		.apply_uV		= 0,
		.always_on		= 1,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE |
					  REGULATOR_CHANGE_STATUS,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
		.state_mem		= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(buck1_consumer),
	.consumer_supplies	= buck1_consumer,
};

static struct regulator_consumer_supply buck2_consumer[] = {
	{	.supply	= "vddint", },
};

static struct regulator_init_data jet_buck2_data = {
	.constraints	= {
		.name			= "VAP_CORE_1.3V",
		.min_uV			= 750000,
		.max_uV			= 1500000,
		.apply_uV		= 0,
		.always_on		= 1,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE |
					  REGULATOR_CHANGE_STATUS,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
		.state_mem		= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(buck2_consumer),
	.consumer_supplies	= buck2_consumer,
};

static struct regulator_init_data jet_buck3_data = {
	.constraints	= {
		.name			= "VAP_MEM_1.8V",
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.apply_uV		= 0,
		.always_on		= 1,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
	},
};

static struct max8906_regulator_data jet_regulators[] = {
	{ MAX8906_LDO2,  &jet_ldo2_data },
	{ MAX8906_LDO3,  &jet_ldo3_data },
	{ MAX8906_LDO4,  &jet_ldo4_data },
	{ MAX8906_LDO5,  &jet_ldo5_data },
	{ MAX8906_LDO6,  &jet_ldo6_data },
	{ MAX8906_LDO7,  &jet_ldo7_data },
	{ MAX8906_LDO8,  &jet_ldo8_data },
	{ MAX8906_LDO9,  &jet_ldo9_data },
	{ MAX8906_BUCK1, &jet_buck1_data },
	{ MAX8906_BUCK2, &jet_buck2_data },
	{ MAX8906_BUCK3, &jet_buck3_data },
};

static struct max8906_platform_data jet_max8906_pdata = {
	.regulators	= jet_regulators,
	.num_regulators	= ARRAY_SIZE(jet_regulators),
};

static struct i2c_board_info jet_pmic_i2c_devs[] __initdata = {
	{
		.type		= "max8906",
		.addr		= 0x66,
		.platform_data	= &jet_max8906_pdata,
	},
};

/* I2C 3 (GPIO) -	MAX9880AERP-T (audio amplifier),
 *			AK4671EG-L (audio codec) */
static struct i2c_gpio_platform_data jet_audio_i2c_pdata = {
	.sda_pin		= GPIO_FM_I2C_SDA,
	.scl_pin		= GPIO_FM_I2C_SCL,
	.udelay			= 2, /* 250KHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 1,
};

static struct platform_device jet_audio_i2c = {
	.name			= "i2c-gpio",
	.id			= 3,
	.dev.platform_data	= &jet_audio_i2c_pdata,
};

static struct i2c_board_info jet_audio_i2c_devs[] __initdata = {
	{
		.type		= "ak4671",
		.addr		= 0x12,
	},
	{
		.type		= "max9880",
		.addr		= 0x4d,
	},
};

/* I2C 4 (GPIO) -	AT42s3c-CU (touchscreen controller) */
static struct i2c_gpio_platform_data jet_touch_i2c_pdata = {
	.sda_pin		= GPIO_TOUCH_I2C_SDA,
	.scl_pin		= GPIO_TOUCH_I2C_SCL,
	.udelay			= 6, /* 83,3KHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device jet_touch_i2c = {
	.name			= "i2c-gpio",
	.id			= 4,
	.dev.platform_data	= &jet_touch_i2c_pdata,
};

static struct s3c_platform_data jet_s3c_pdata = {
	.rst_gpio	= GPIO_TOUCH_RST,
	.rst_inverted	= 0,
	.en_gpio	= GPIO_TOUCH_EN,
	.en_inverted	= 0,
};

static struct i2c_board_info jet_touch_i2c_devs[] __initdata = {
	{
		.type		= "s3c_ts",
		.addr		= 0x30,
		.irq		= IRQ_EINT(20),
		.platform_data	= &jet_s3c_pdata,
	},
};

/*
 * Android PMEM
 */

#ifdef CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data pmem_pdata = {
	.name		= "pmem",
	.no_allocator	= 1,
	.cached		= 1,
	.buffered	= 1,
	.start		= RESERVED_PMEM_START,
	.size		= RESERVED_PMEM,
};

static struct android_pmem_platform_data pmem_gpu1_pdata = {
	.name		= "pmem_gpu1",
	.no_allocator	= 0,
	.cached		= 1,
	.buffered	= 1,
	.start		= GPU1_RESERVED_PMEM_START,
	.size		= RESERVED_PMEM_GPU1,
};

static struct android_pmem_platform_data pmem_render_pdata = {
	.name		= "pmem_render",
	.no_allocator	= 1,
	.cached		= 0,
	.start		= RENDER_RESERVED_PMEM_START,
	.size		= RESERVED_PMEM_RENDER,
};

static struct android_pmem_platform_data pmem_stream_pdata = {
	.name		= "pmem_stream",
	.no_allocator	= 1,
	.cached		= 0,
	.start		= STREAM_RESERVED_PMEM_START,
	.size		= RESERVED_PMEM_STREAM,
};

static struct android_pmem_platform_data pmem_preview_pdata = {
	.name		= "pmem_preview",
	.no_allocator	= 1,
	.cached		= 0,
        .start		= PREVIEW_RESERVED_PMEM_START,
        .size		= RESERVED_PMEM_PREVIEW,
};

static struct android_pmem_platform_data pmem_picture_pdata = {
	.name		= "pmem_picture",
	.no_allocator	= 1,
	.cached		= 0,
        .start		= PICTURE_RESERVED_PMEM_START,
        .size		= RESERVED_PMEM_PICTURE,
};

static struct android_pmem_platform_data pmem_jpeg_pdata = {
	.name		= "pmem_jpeg",
	.no_allocator	= 1,
	.cached		= 0,
        .start		= JPEG_RESERVED_PMEM_START,
        .size		= RESERVED_PMEM_JPEG,
};

static struct android_pmem_platform_data pmem_skia_pdata = {
	.name		= "pmem_skia",
	.no_allocator	= 1,
	.cached		= 0,
        .start		= SKIA_RESERVED_PMEM_START,
        .size		= RESERVED_PMEM_SKIA,
};

static struct platform_device pmem_device = {
	.name		= "android_pmem",
	.id		= 0,
	.dev		= { .platform_data = &pmem_pdata },
};
 
static struct platform_device pmem_gpu1_device = {
	.name		= "android_pmem",
	.id		= 1,
	.dev		= { .platform_data = &pmem_gpu1_pdata },
};

static struct platform_device pmem_render_device = {
	.name		= "android_pmem",
	.id		= 2,
	.dev		= { .platform_data = &pmem_render_pdata },
};

static struct platform_device pmem_stream_device = {
	.name		= "android_pmem",
	.id		= 3,
	.dev		= { .platform_data = &pmem_stream_pdata },
};

static struct platform_device pmem_preview_device = {
	.name		= "android_pmem",
	.id		= 5,
	.dev		= { .platform_data = &pmem_preview_pdata },
};

static struct platform_device pmem_picture_device = {
	.name		= "android_pmem",
	.id		= 6,
	.dev		= { .platform_data = &pmem_picture_pdata },
};

static struct platform_device pmem_jpeg_device = {
	.name		= "android_pmem",
	.id		= 7,
	.dev		= { .platform_data = &pmem_jpeg_pdata },
};

static struct platform_device pmem_skia_device = {
	.name		= "android_pmem",
	.id		= 8,
	.dev		= { .platform_data = &pmem_skia_pdata },
};

static struct platform_device *pmem_devices[] = {
	&pmem_device,
	&pmem_gpu1_device,
	&pmem_render_device,
	&pmem_stream_device,
	&pmem_preview_device,
	&pmem_picture_device,
	&pmem_jpeg_device,
	&pmem_skia_device
};

static void __init jet_add_mem_devices(void)
{
	unsigned i;
	for (i = 0; i < ARRAY_SIZE(pmem_devices); ++i)
		if (pmem_devices[i]->dev.platform_data) {
			struct android_pmem_platform_data *pmem =
					pmem_devices[i]->dev.platform_data;

			if (pmem->size)
				platform_device_register(pmem_devices[i]);
		}
}
#endif

/*
 * LCD screen
 */
static struct ams310fs07_platform_data jet_ams310fs07_pdata = {
	.reset_gpio	= GPIO_LCD_RST_N,
	.cs_gpio	= GPIO_LCD_CS_N,
	.sck_gpio	= GPIO_LCD_SCLK,
	.sda_gpio	= GPIO_LCD_SDI,
	.vci_regulator	= "lcd_vci",
	.vdd3_regulator	= "lcd_vdd3",
};

static struct platform_device jet_ams310fs07 = {
	.name		= "ams310fs07-lcd",
	.id		= -1,
	.dev		= {
		.platform_data		= &jet_ams310fs07_pdata,
	},
};

/*
 * SDHCI platform data
 */

static irqreturn_t jet_tf_cd_thread(int irq, void *dev_id)
{
	void (*notify_func)(struct platform_device *, int state) = dev_id;
	int status = gpio_get_value(GPIO_TF_DETECT);

	notify_func(&s3c_device_hsmmc0, !status);

	return IRQ_HANDLED;
}

static int jet_tf_cd_init(void (*notify_func)(struct platform_device *,
								int state))
{
	int ret, status;

	ret = gpio_request(GPIO_TF_DETECT, "TF detect");
	if (ret)
		return ret;

	ret = gpio_direction_input(GPIO_TF_DETECT);
	if (ret)
		return ret;

	ret = request_threaded_irq(gpio_to_irq(GPIO_TF_DETECT), NULL,
		jet_tf_cd_thread, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
		"TF card detect", notify_func);
	if (ret)
		return ret;

	status = gpio_get_value(GPIO_TF_DETECT);
	notify_func(&s3c_device_hsmmc0, !status);

	return 0;
}

static int jet_tf_cd_cleanup(void (*notify_func)(struct platform_device *,
								int state))
{
	free_irq(gpio_to_irq(GPIO_TF_DETECT), notify_func);
	gpio_free(GPIO_TF_DETECT);

	return 0;
}

struct s3c_sdhci_platdata jet_hsmmc0_pdata = {
	.cd_type	= S3C_SDHCI_CD_EXTERNAL,
	.ext_cd_init	= jet_tf_cd_init,
	.ext_cd_cleanup	= jet_tf_cd_cleanup,
};

struct s3c_sdhci_platdata jet_hsmmc2_pdata = {
	.cd_type	= S3C_SDHCI_CD_PERMANENT,
};

/*
 * Framebuffer
 */

static struct s3c_fb_pd_win jet_fb_win0 = {
	/* this is to ensure we use win0 */
	.win_mode	= {
		.left_margin	= 10,
		.right_margin	= 10,
		.upper_margin	= 3,
		.lower_margin	= 8,
		.hsync_len	= 10,
		.vsync_len	= 2,
		.xres		= 480,
		.yres		= 800,
	},
	.max_bpp	= 16,
	.default_bpp	= 16,
	.virtual_y	= 800 * 2,
	.virtual_x	= 480,
};

static struct s3c_fb_pd_win jet_fb_win1 = {
	/* this is to ensure we use win0 */
	.win_mode	= {
		.left_margin	= 10,
		.right_margin	= 10,
		.upper_margin	= 3,
		.lower_margin	= 8,
		.hsync_len	= 10,
		.vsync_len	= 2,
		.xres		= 480,
		.yres		= 800,
	},
	.max_bpp	= 16,
	.default_bpp	= 16,
	.virtual_y	= 800 * 2,
	.virtual_x	= 480,
};

static void jet_fb_gpio_setup_18bpp(void)
{
	/* Blue */
	s3c_gpio_cfgrange_nopull(GPIO_LCD_B_0, 6, S3C_GPIO_SFN(2));
	/* Green */
	s3c_gpio_cfgrange_nopull(GPIO_LCD_G_0, 6, S3C_GPIO_SFN(2));
	/* Red + control signals */
	s3c_gpio_cfgrange_nopull(GPIO_LCD_R_0, 10, S3C_GPIO_SFN(2));
}

static struct s3c_fb_platdata jet_lcd_pdata __initdata = {
	.setup_gpio	= jet_fb_gpio_setup_18bpp,
	.win[0]		= &jet_fb_win0,
	.win[1]		= &jet_fb_win1,
	.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
	.vidcon1	= VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC
			| VIDCON1_INV_VCLK,
};

/*
 * RAM console (for debugging)
 */

static struct resource jet_ram_console_resources[] = {
	[0] = {
		.start	= 0x5c000000,
		.end	= 0x5c0fffff,
		.flags	= IORESOURCE_MEM,
	},
};

struct platform_device jet_ram_console = {
	.name		= "ram_console",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(jet_ram_console_resources),
	.resource	= jet_ram_console_resources,
};

/*
 * Keyboard
 */

/* S3C6410 matrix keypad controller */

static uint32_t jet_keymap[] __initdata = {
	/* KEY(row, col, keycode) */
	KEY(0, 0, 201), KEY(0, 1, 209) /* Reserved  */ /* Reserved  */,
	KEY(1, 0, 202), KEY(1, 1, 210), KEY(1, 2, 218), KEY(1, 3, 226),
	KEY(2, 0, 203), KEY(2, 1, 211), KEY(2, 2, 219), KEY(2, 3, 227),
	KEY(3, 0, 204), KEY(3, 1, 212) /* Reserved  */, KEY(3, 3, 228),
};

static struct matrix_keymap_data jet_keymap_data __initdata = {
	.keymap		= jet_keymap,
	.keymap_size	= ARRAY_SIZE(jet_keymap),
};

static struct samsung_keypad_platdata jet_keypad_pdata __initdata = {
	.keymap_data	= &jet_keymap_data,
	.rows		= 4,
	.cols		= 4,
};

/* GPIO keys */

static struct gpio_keys_button jet_gpio_keys_data[] = {
	{
		.gpio			= GPIO_POWER_N,
		.code			= 249,
		.desc			= "Power",
		.active_low		= 1,
		.debounce_interval	= 5,
		.type                   = EV_KEY,
		.wakeup			= 1,
	},
	{
		.gpio			= GPIO_HOLD_KEY_N,
		.code			= 251,
		.desc			= "Hold",
		.active_low		= 1,
		.debounce_interval	= 5,
		.type                   = EV_KEY,
		.wakeup			= 1,
	},
};

static struct gpio_keys_platform_data jet_gpio_keys_pdata  = {
	.buttons	= jet_gpio_keys_data,
	.nbuttons	= ARRAY_SIZE(jet_gpio_keys_data),
};

static struct platform_device jet_gpio_keys = {
	.name		= "gpio-keys",
	.id		= 0,
	.num_resources	= 0,
	.dev		= {
		.platform_data	= &jet_gpio_keys_pdata,
	}
};

/*
 * OneNAND
 */

static struct mtd_partition jet_onenand_parts[] = {
	[0] = {
		.name		= "pbl",
		.size		= SZ_128K,
		.offset		= 0x00000000,
		.mask_flags	= MTD_WRITEABLE,
	},
	[1] = {
		.name		= "sbl",
		.size		= SZ_1M + SZ_256K,
		.offset		= 0x00020000,
		.mask_flags	= MTD_WRITEABLE,
	},
	[2] = {
		.name		= "logo",
		.size		= SZ_128K,
		.offset		= 0x00160000,
		.mask_flags	= MTD_WRITEABLE,
	},
	[3] = {
		.name		= "param",
		.size		= SZ_256K,
		.offset		= 0x00180000,
		.mask_flags	= MTD_WRITEABLE,
	},
	[4] = {
		.name		= "kernel",
		.size		= SZ_4M + SZ_1M,
		.offset		= 0x001c0000,
	},
	/* Following partitions are incompatible with legacy images */
	[5] = {
		.name		= "system",
		.size		= SZ_128M + SZ_16M + SZ_8M,
		.offset		= 0x006c0000,
	},
	[6] = {
		.name		= "data",
		.size		= SZ_256M + SZ_16M + SZ_4M + SZ_2M,
		.offset		= 0x09ec0000,
	},
	/* End of incompatible partitions */
	[7] = {
		.name		= "xbin",
		.size		= SZ_16M + SZ_8M,
		.offset		= 0x1b540000,
	},
	[8] = {
		.name		= "cache",
		.size		= SZ_8M,
		.offset		= 0x1cd40000,
	},
	/* This is a new efs partition to keep the original one untouched */
	[9] = {
		.name		= "efs",
		.size		= SZ_8M,
		.offset		= 0x1d540000,
	},
	[10] = {
		.name		= "modem",
		.size		= SZ_16M,
		.offset		= 0x1dd40000,
	},
	/* Original efs partition */
	[11] = {
		.name		= "efs_legacy",
		.size		= SZ_8M,
		.offset		= 0x1ed40000,
		.mask_flags	= MTD_WRITEABLE,
	},
	/* XSR metadata */
	[12] = {
		.name		= "reservoir",
		.size		= SZ_8M + SZ_2M + SZ_512K + SZ_256K,
		.offset		= 0x1f540000,
		.mask_flags	= MTD_WRITEABLE,
	},
};

static struct onenand_platform_data jet_onenand_pdata = {
	.parts		= jet_onenand_parts,
	.nr_parts	= ARRAY_SIZE(jet_onenand_parts),
};

/*
 * Hardware monitoring (ADC)
 */

static char *jet_charger_supplicants[] = {
	"samsung-battery",
};

static struct gpio_charger_platform_data jet_charger_pdata = {
	.type			= POWER_SUPPLY_TYPE_MAINS,
	.gpio			= GPIO_TA_CONNECTED_N,
	.gpio_active_low	= 1,
	.gpio_chg		= GPIO_TA_CHG_N,
	.gpio_chg_active_low	= 1,
	.gpio_en		= GPIO_TA_EN,
	.gpio_en_active_low	= 1,
	.supplied_to		= jet_charger_supplicants,
	.num_supplicants	= ARRAY_SIZE(jet_charger_supplicants),
};

static struct platform_device jet_charger = {
	.name		= "gpio-charger",
	.id		= -1,
	.dev		= {
		.platform_data	= &jet_charger_pdata,
	}
};

// #define BATT_CAL		2447	// 3.60V
// #define BATT_MAXIMUM		406	// 4.176V
// #define BATT_FULL		353	// 4.10V
// #define BATT_ALMOST_FULL	188	// 3.8641V
// #define BATT_HIGH		112	// 3.7554V
// #define BATT_MED		66	// 3.6907V
// #define BATT_LOW		43	// 3.6566V
// #define BATT_CRITICAL	8	// 3.6037V
// #define BATT_MINIMUM		(-28)	// 3.554V
// #define BATT_OFF		(-128)	// 3.4029V

static struct samsung_battery_threshold jet_battery_lut[] = {
	/* ADC,	Percents	// Volts */
	{ 2319,	0 },		// 3,4029
	{ 2419,	3 },		// 3.5540
	{ 2455,	5 },		// 3.6037
	{ 2490,	15 },		// 3.6566
	{ 2513,	30 },		// 3.6907
	{ 2559,	50 },		// 3.7554
	{ 2635,	70 },		// 3.8641
	{ 2800,	100 },		// 4.1000
};

static struct samsung_battery_pdata jet_battery_pdata = {
	.lut		= jet_battery_lut,
	.lut_cnt	= ARRAY_SIZE(jet_battery_lut),
	.volt_channel	= 0,
	.temp_channel	= 1,
	.charger	= "gpio-charger",
	.use_for_apm	= 1,
};

static struct platform_device jet_battery = {
	.name		= "samsung-battery",
	.id		= -1,
	.dev		= {
		.platform_data	= &jet_battery_pdata,
	}
};

/*
 * Platform devices
 */

static struct platform_device *jet_devices[] __initdata = {
	&s3c_device_hsmmc0,
	&s3c_device_hsmmc2,
	&s3c_device_rtc,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_fb,
	&s3c_device_usb_hsotg,
	&s3c_device_onenand,
	&samsung_device_keypad,
	&jet_pmic_i2c,
	&jet_audio_i2c,
	&jet_touch_i2c,
	&jet_ams310fs07,
	&jet_android_usb,
	&jet_usb_mass_storage,
	&jet_usb_rndis,
	&jet_ram_console,
	&jet_gpio_keys,
#ifdef CONFIG_S3C64XX_DMA_ENGINE
	&s3c64xx_device_dma0,
	&s3c64xx_device_dma1,
#endif
	&s3c_device_adc,
	&jet_charger,
	&jet_battery,
	&samsung_asoc_dma,
	&s3c64xx_device_iis0,
	&s3c_device_timer[1],
};

/*
 * Platform devices for Samsung modules
 */

#ifdef CONFIG_GT_S8000_SAMSUNG_MODULES
static struct resource s3c_g3d_resource[] = {
	[0] = {
		.start = S3C64XX_PA_G3D,
		.end   = S3C64XX_PA_G3D + S3C64XX_SZ_G3D - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_S3C6410_G3D,
		.end   = IRQ_S3C6410_G3D,
		.flags = IORESOURCE_IRQ,
	}
};

static struct resource s3c_mfc_resource[] = {
	[0] = {
		.start = S3C64XX_PA_MFC,
		.end   = S3C64XX_PA_MFC + S3C64XX_SZ_MFC - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_MFC,
		.end   = IRQ_MFC,
		.flags = IORESOURCE_IRQ,
	}
};

static struct resource s3c_jpeg_resource[] = {
	[0] = {
		.start = S3C64XX_PA_JPEG,
		.end   = S3C64XX_PA_JPEG + S3C64XX_SZ_JPEG - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_JPEG,
		.end   = IRQ_JPEG,
		.flags = IORESOURCE_IRQ,
	}
};

static struct resource s3c_camif_resource[] = {
	[0] = {
		.start = S3C64XX_PA_FIMC,
		.end   = S3C64XX_PA_FIMC + S3C64XX_SZ_FIMC - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_CAMIF_C,
		.end   = IRQ_CAMIF_C,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start = IRQ_CAMIF_P,
		.end   = IRQ_CAMIF_P,
		.flags = IORESOURCE_IRQ,
	}
};

static struct resource s3c_g2d_resource[] = {
	[0] = {
		.start = S3C64XX_PA_G2D,
		.end   = S3C64XX_PA_G2D + S3C64XX_SZ_G2D - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_2D,
		.end   = IRQ_2D,
		.flags = IORESOURCE_IRQ,
	}
};

static struct resource s3c_rotator_resource[] = {
	[0] = {
		.start = S3C64XX_PA_ROTATOR,
		.end   = S3C64XX_PA_ROTATOR + S3C64XX_SZ_ROTATOR - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_ROTATOR,
		.end   = IRQ_ROTATOR,
		.flags = IORESOURCE_IRQ,
	}
};

static struct resource s3c_pp_resource[] = {
	[0] = {
		.start = S3C64XX_PA_PP,
		.end   = S3C64XX_PA_PP + S3C64XX_SZ_PP - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_POST0,
		.end   = IRQ_POST0,
		.flags = IORESOURCE_IRQ,
	}
};
#endif

static struct platform_device *jet_mod_devices[] __initdata = {
#ifdef CONFIG_GT_S8000_SAMSUNG_MODULES
	&(struct platform_device){
		.name		= "s3c-g3d",
		.id		= -1,
		.num_resources	= ARRAY_SIZE(s3c_g3d_resource),
		.resource	= s3c_g3d_resource,
	}, &(struct platform_device){
		.name		= "s3c-mfc",
		.id		= -1,
		.num_resources	= ARRAY_SIZE(s3c_mfc_resource),
		.resource	= s3c_mfc_resource,
	}, &(struct platform_device){
		.name		= "s3c-jpeg",
		.id		= -1,
		.num_resources	= ARRAY_SIZE(s3c_jpeg_resource),
		.resource	= s3c_jpeg_resource,
	}, &(struct platform_device){
		.name		= "s3c-fimc",
		.id		= -1,
		.num_resources	= ARRAY_SIZE(s3c_camif_resource),
		.resource	= s3c_camif_resource,
	}, &(struct platform_device){
		.name		= "s3c-g2d",
		.id		= -1,
		.num_resources	= ARRAY_SIZE(s3c_g2d_resource),
		.resource	= s3c_g2d_resource,
	}, &(struct platform_device){
		.name		= "s3c-rotator",
		.id		= -1,
		.num_resources	= ARRAY_SIZE(s3c_rotator_resource),
		.resource	= s3c_rotator_resource,
	}, &(struct platform_device){
		.name		= "s3c-pp",
		.id		= -1,
		.num_resources	= ARRAY_SIZE(s3c_pp_resource),
		.resource	= s3c_pp_resource,
	}
#endif
};

/*
 * Extended I/O map
 */

static struct map_desc jet_iodesc[] __initdata = {
};

/*
 * GPIO setup
 */

static struct s3c_gpio_config jet_gpio_table[] = {
	/*
	 * ALIVE PART
	 */

	/* GPA */
	S3C_GPIO_ALIVE(FLM_RXD, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(FLM_TXD, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(MSENSE_RST, OUTPUT, HIGH, NONE, OUT1, NONE),
	S3C_GPIO_ALIVE(BT_RXD, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(BT_TXD, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(BT_CTS, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(BT_RTS, INPUT, NONE, DOWN, INPUT, DOWN),

	/* GPB */
	S3C_GPIO_ALIVE_EXT(PDA_RXD, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(PDA_TXD, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(I2C1_SCL, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(I2C1_SDA, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(TOUCH_EN, OUTPUT, LOW, NONE, OUT1, NONE),
	S3C_GPIO_ALIVE(I2C0_SCL, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(I2C0_SDA, INPUT, NONE, NONE, INPUT, NONE),

	/* GPC */
	S3C_GPIO_ALIVE_EXT(PM_SET1, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE_EXT(PM_SET2, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE_EXT(PM_SET3, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(WLAN_CMD, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(WLAN_CLK, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(WLAN_WAKE, OUTPUT, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(BT_WAKE, OUTPUT, LOW, NONE, OUT0, NONE),

	/* GPD */
	S3C_GPIO_ALIVE(I2S_CLK, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(BT_WLAN_REG_ON, OUTPUT, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(I2S_LRCLK, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(I2S_DI, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(I2S_DO, INPUT, NONE, DOWN, INPUT, DOWN),

	/* GPE */
	S3C_GPIO_ALIVE(BT_RST_N, OUTPUT, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(BOOT, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(WLAN_RST_N, OUTPUT, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(PWR_I2C_SCL, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(PWR_I2C_SDA, INPUT, NONE, NONE, INPUT, NONE),

	/* GPF */
	S3C_GPIO_ALIVE(CAM_MCLK, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(CAM_HSYNC, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(CAM_PCLK, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(MCAM_RST_N, OUTPUT, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(CAM_VSYNC, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(CAM_D_0, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(CAM_D_1, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(CAM_D_2, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(CAM_D_3, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(CAM_D_4, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(CAM_D_5, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(CAM_D_6, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(CAM_D_7, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(VIBTONE_PWM, NONE, DOWN, INPUT, DOWN),

	/* GPG */
	S3C_GPIO_ALIVE(TF_CLK, INPUT, NONE, DOWN, INPUT, DOWN),
	S3C_GPIO_ALIVE(TF_CMD, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(TF_D_0, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(TF_D_1, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(TF_D_2, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(TF_D_3, INPUT, NONE, NONE, INPUT, NONE),

	/* GPH */
	S3C_GPIO_ALIVE(TOUCH_I2C_SCL, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(TOUCH_I2C_SDA, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(FM_I2C_SCL, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(FM_I2C_SDA, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(VIB_EN, OUTPUT, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(WLAN_D_0, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(WLAN_D_1, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(WLAN_D_2, INPUT, NONE, NONE, INPUT, NONE),
	S3C_GPIO_ALIVE(WLAN_D_3, INPUT, NONE, NONE, INPUT, NONE),

	/* GPI */
	S3C_GPIO_ALIVE_EXT(LCD_B_0, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_B_1, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_B_2, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_B_3, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_B_4, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_B_5, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_G_0, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_G_1, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_G_2, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_G_3, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_G_4, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_G_5, NONE, NONE, INPUT, DOWN),

	/* GPJ */
	S3C_GPIO_ALIVE_EXT(LCD_R_0, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_R_1, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_R_2, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_R_3, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_R_4, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_R_5, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_HSYNC, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_VSYNC, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_DE, NONE, NONE, INPUT, DOWN),
	S3C_GPIO_ALIVE_EXT(LCD_CLK, NONE, NONE, INPUT, DOWN),

	/*
	 * OFF PART
	 */

	/* GPK */
	S3C_GPIO_OFF_EXT(TA_EN, HIGH, NONE),
	S3C_GPIO_OFF_EXT(AUDIO_EN, LOW, NONE),
	S3C_GPIO_OFF_EXT(PHONE_ON, LOW, NONE),
	S3C_GPIO_OFF_EXT(MICBIAS_EN, LOW, NONE),
	S3C_GPIO_OFF_EXT(UART_SEL, HIGH, NONE),
	S3C_GPIO_OFF_EXT(TOUCH_RST, HIGH, NONE),
	S3C_GPIO_OFF_EXT(CAM_EN, LOW, NONE),
	S3C_GPIO_OFF_EXT(PHONE_RST_N, HIGH, NONE),
	S3C_GPIO_OFF_EXT(KEYSCAN_0, NONE, NONE),
	S3C_GPIO_OFF_EXT(KEYSCAN_1, NONE, NONE),
	S3C_GPIO_OFF_EXT(KEYSCAN_2, NONE, NONE),
	S3C_GPIO_OFF_EXT(KEYSCAN_3, NONE, NONE),
	S3C_GPIO_OFF(RES_GPK12, OUTPUT, LOW, NONE),
	S3C_GPIO_OFF(RES_GPK13, OUTPUT, LOW, NONE),
	S3C_GPIO_OFF(RES_GPK14, OUTPUT, LOW, NONE),
	S3C_GPIO_OFF_EXT(VREG_MSMP_26V, NONE, NONE),

	/* GPL */
	S3C_GPIO_OFF_EXT(KEYSENSE_0, LOW, NONE),
	S3C_GPIO_OFF_EXT(KEYSENSE_1, LOW, NONE),
	S3C_GPIO_OFF_EXT(KEYSENSE_2, LOW, NONE),
	S3C_GPIO_OFF_EXT(KEYSENSE_3, LOW, NONE),
	S3C_GPIO_OFF_EXT(USIM_BOOT, LOW, NONE),
	S3C_GPIO_OFF_EXT(CAM_3M_STBY_N, LOW, NONE),
	S3C_GPIO_OFF_EXT(HOLD_KEY_N, NONE, NONE),
	S3C_GPIO_OFF(RES_GPL10, OUTPUT, LOW, NONE),
	S3C_GPIO_OFF_EXT(TA_CONNECTED_N, NONE, NONE),
	S3C_GPIO_OFF_EXT(TOUCH_INT_N, NONE, NONE),
	S3C_GPIO_OFF_EXT(CP_BOOT_SEL, LOW, NONE),
	S3C_GPIO_OFF_EXT(BT_HOST_WAKE, NONE, DOWN),

	/* GPM */
	S3C_GPIO_OFF(RES_GPM0, OUTPUT, LOW, NONE),
	S3C_GPIO_OFF(RES_GPM1, OUTPUT, LOW, NONE),
	S3C_GPIO_OFF_EXT(TA_CHG_N, NONE, NONE),
	S3C_GPIO_OFF_EXT(PDA_ACTIVE, LOW, NONE),
	S3C_GPIO_OFF(RES_GPM4, OUTPUT, LOW, NONE),
	S3C_GPIO_OFF(RES_GPM5, OUTPUT, LOW, NONE),

	/* GPN */
	S3C_GPIO_OFF_EXT(ONEDRAM_INT_N, NONE, NONE),
	S3C_GPIO_OFF_EXT(WLAN_HOST_WAKE, NONE, DOWN),
	S3C_GPIO_OFF_EXT(MSENSE_INT, NONE, NONE),
	S3C_GPIO_OFF_EXT(ACC_INT, NONE, NONE),
	S3C_GPIO_OFF_EXT(SIM_DETECT_N, NONE, NONE),
	S3C_GPIO_OFF_EXT(POWER_N, NONE, NONE),
	S3C_GPIO_OFF_EXT(TF_DETECT, NONE, NONE),
	S3C_GPIO_OFF_EXT(PHONE_ACTIVE, NONE, NONE),
	S3C_GPIO_OFF_EXT(PMIC_INT_N, NONE, NONE),
	S3C_GPIO_OFF_EXT(JACK_INT_N, NONE, NONE),
	S3C_GPIO_OFF_EXT(DET_35, NONE, NONE),
	S3C_GPIO_OFF_EXT(EAR_SEND_END, NONE, NONE),
	S3C_GPIO_OFF_EXT(RESOUT_N, NONE, NONE),
	S3C_GPIO_OFF_EXT(BOOT_EINT13, NONE, NONE),
	S3C_GPIO_OFF_EXT(BOOT_EINT14, NONE, NONE),
	S3C_GPIO_OFF_EXT(BOOT_EINT15, NONE, NONE),

	/*
	 * MEMORY PART
	 */

	/* GPO */
	S3C_GPIO_ALIVE(RES_GPO4, OUTPUT, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(RES_GPO5, OUTPUT, LOW, NONE, OUT0, NONE),

	/* GPP */
	S3C_GPIO_ALIVE(RES_GPP8, OUTPUT, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(RES_GPP10, OUTPUT, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(RES_GPP14, OUTPUT, LOW, NONE, OUT0, NONE),

	/* GPQ */
	S3C_GPIO_ALIVE(RES_GPQ2, OUTPUT, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(RES_GPQ3, OUTPUT, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(RES_GPQ4, OUTPUT, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(RES_GPQ5, OUTPUT, LOW, NONE, OUT0, NONE),
	S3C_GPIO_ALIVE(RES_GPQ6, OUTPUT, LOW, NONE, OUT0, NONE),
};

/*
 * Machine setup
 */

static void __init jet_fixup(struct machine_desc *desc,
		struct tag *tags, char **cmdline, struct meminfo *mi)
{
	mi->nr_banks = 2;

	mi->bank[0].start = PHYS_OFFSET;
	mi->bank[0].size = SZ_128M;

	mi->bank[1].start = PHYS_OFFSET + SZ_128M;
	mi->bank[1].size = PHYS_UNRESERVED_SIZE - SZ_128M;
}

static void __init jet_map_io(void)
{
	s3c64xx_init_io(jet_iodesc, ARRAY_SIZE(jet_iodesc));
	s3c24xx_init_clocks(12000000);
	s3c24xx_init_uarts(jet_uartcfgs, ARRAY_SIZE(jet_uartcfgs));
}

static void __init jet_machine_init(void)
{
	s3c_pm_init();

	s3c_init_gpio(jet_gpio_table, ARRAY_SIZE(jet_gpio_table));

	/* Register I2C devices */
	s3c_i2c0_set_platdata(&jet_misc_i2c);
	i2c_register_board_info(jet_misc_i2c.bus_num, jet_misc_i2c_devs,
					ARRAY_SIZE(jet_misc_i2c_devs));
	s3c_i2c1_set_platdata(&jet_cam_i2c);
	i2c_register_board_info(jet_cam_i2c.bus_num, jet_cam_i2c_devs,
					ARRAY_SIZE(jet_cam_i2c_devs));
	i2c_register_board_info(jet_pmic_i2c.id, jet_pmic_i2c_devs,
					ARRAY_SIZE(jet_pmic_i2c_devs));
	i2c_register_board_info(jet_audio_i2c.id, jet_audio_i2c_devs,
					ARRAY_SIZE(jet_audio_i2c_devs));
	i2c_register_board_info(jet_touch_i2c.id, jet_touch_i2c_devs,
					ARRAY_SIZE(jet_touch_i2c_devs));

	s3c_fb_set_platdata(&jet_lcd_pdata);

	s3c_sdhci0_set_platdata(&jet_hsmmc0_pdata);
	s3c_sdhci2_set_platdata(&jet_hsmmc2_pdata);

	samsung_keypad_set_platdata(&jet_keypad_pdata);

	s3c_set_platdata(&jet_onenand_pdata, sizeof(jet_onenand_pdata),
							&s3c_device_onenand);

	/* Register platform devices */
	platform_add_devices(jet_devices, ARRAY_SIZE(jet_devices));
	platform_add_devices(jet_mod_devices, ARRAY_SIZE(jet_mod_devices));

#ifdef CONFIG_ANDROID_PMEM
	/* Register PMEM devices */
	jet_add_mem_devices();
#endif

	regulator_has_full_constraints();
}

/*
 * Machine definition
 */

MACHINE_START(GT_S8000, "jet")
	/* Maintainer: Currently none */
	.boot_params	= S3C64XX_PA_SDRAM + 0x100,
	.init_irq	= s3c6410_init_irq,
	.fixup		= jet_fixup,
	.map_io		= jet_map_io,
	.init_machine	= jet_machine_init,
	.timer		= &s3c64xx_timer,
MACHINE_END
