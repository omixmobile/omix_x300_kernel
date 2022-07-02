#define LOG_TAG "LCM"

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif

#include "lcm_drv.h"


#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
/*#include <mach/mt_pm_ldo.h>*/
#ifdef CONFIG_MTK_LEGACY
#include <mach/mt_gpio.h>
#endif
#endif

#ifdef BUILD_LK
#define LCM_LOGI(fmt, args...)  printk(KERN_INFO  " LCM file=%s: %s: line=%d: "fmt"\n", __FILE__,__func__,  __LINE__,##args)
#define LCM_LOGD(fmt, args...)  printk(KERN_DEBUG " LCM file=%s: %s: line=%d: "fmt"\n", __FILE__,__func__,  __LINE__,##args)
#else
#define LCM_LOGI(fmt, args...)  printk(KERN_INFO " LCM :"fmt"\n", ##args)
#define LCM_LOGD(fmt, args...)  printk(KERN_DEBUG " LCM :"fmt"\n", ##args)
#endif

#define I2C_I2C_LCD_BIAS_CHANNEL 0
static struct LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)			(lcm_util.set_reset_pin((v)))
#define MDELAY(n)					(lcm_util.mdelay(n))

/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
	lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) \
	lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
	lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) \
	lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
	lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

static const unsigned char LCD_MODULE_ID = 0x01;
/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */
#define LCM_DSI_CMD_MODE	0
#define FRAME_WIDTH  										720
#define FRAME_HEIGHT 										1560
//prize-penggy modify LCD size-20190328-start
#define LCM_PHYSICAL_WIDTH                  				(69500)
#define LCM_PHYSICAL_HEIGHT                  				(154440)
//prize-penggy modify LCD size-20190328-end

#define REGFLAG_DELAY             							 0xFFFA
#define REGFLAG_UDELAY             							 0xFFFB
#define REGFLAG_PORT_SWAP									 0xFFFC
#define REGFLAG_END_OF_TABLE      							 0xFFFD   // END OF REGISTERS MARKER

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* --------------------------------------------------------------------------- */
/* Local Variables */
/* --------------------------------------------------------------------------- */

struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
{0x00,1,{0x00}},
{0xFF,3,{0x87,0x19,0x01}},
{0x00,1,{0x80}},
{0xFF,2,{0x87,0x19}},
{0x00,1,{0x90}},
{0xFF,1,{0x00}},
{0x00,1,{0xA1}},
{0xB3,4,{0x02,0xD0,0x06,0x18}},
{0x00,1,{0xA5}},
{0xB3,1,{0x40}},
{0x00,1,{0xA6}},
{0xB3,1,{0xF8}},
{0x00,1,{0x85}},
{0xA7,1,{0x03}},
{0x00,1,{0xD0}},
{0xC3,1,{0x46}},
{0x00,1,{0xD3}},
{0xC3,1,{0x30}},
{0x00,1,{0xD4}},
{0xC3,1,{0x46}},
{0x00,1,{0xD7}},
{0xC3,1,{0x30}},
{0x00,1,{0xD8}},
{0xC3,1,{0x46}},
{0x00,1,{0xDB}},
{0xC3,1,{0x30}},
{0x00,1,{0xCA}},
{0xC0,1,{0x80}},
{0x00,1,{0x80}},
{0xC2,16,{0x83,0x01,0x01,0x81,0x82,0x01,0x01,0x81,0x82,0x00,0x01,0x81,0x81,0x00,0x01,0x81}},
{0x00,1,{0x90}},
{0xC2,16,{0x80,0x00,0x01,0x81,0x01,0x00,0x01,0x81,0x02,0x00,0x01,0x81,0x03,0x00,0x01,0x81}},
{0x00,1,{0xA0}},
{0xC2,15,{0x8A,0x0A,0x00,0x01,0x8C,0x89,0x0A,0x00,0x01,0x8C,0x88,0x0A,0x00,0x01,0x8C}},
{0x00,1,{0xB0}},
{0xC2,15,{0x87,0x0A,0x00,0x01,0x8C,0x00,0x00,0x00,0x01,0x81,0x00,0x00,0x00,0x01,0x81}},
{0x00,1,{0xC0}},
{0xC2,15,{0x00,0x00,0x00,0x01,0x81,0x00,0x00,0x00,0x01,0x81,0x00,0x00,0x00,0x01,0x81}},
{0x00,1,{0xD0}},
{0xC2,15,{0x00,0x00,0x00,0x01,0x81,0x00,0x00,0x00,0x01,0x81,0x00,0x00,0x00,0x01,0x81}},
{0x00,1,{0xE0}},
{0xC2,14,{0x33,0x33,0x33,0x33,0x33,0x33,0x00,0x00,0x12,0x00,0x0A,0x50,0x01,0x81}},
{0x00,1,{0xF0}},
{0xC2,14,{0x80,0x00,0x50,0x05,0x11,0x80,0x00,0x50,0x05,0x11,0xFF,0xFF,0xFF,0x01}},
{0x00,1,{0x80}},
{0xCB,16,{0xFD,0xFD,0xFC,0x01,0xFD,0x01,0x01,0xFD,0xFE,0x02,0xFD,0x01,0xFD,0xFD,0x01,0x01}},
{0x00,1,{0x90}},
{0xCB,16,{0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0x00,1,{0xA0}},
{0xCB,4,{0x00,0x00,0x00,0x00}},
{0x00,1,{0xB0}},
{0xCB,4,{0x51,0x55,0xA5,0x55}},
{0x00,1,{0xE0}},
{0xC3,1,{0x35}},
{0x00,1,{0xE4}},
{0xC3,1,{0x35}},
{0x00,1,{0xE8}},
{0xC3,1,{0x35}},
{0x00,1,{0x80}},
{0xCC,16,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x29,0x24,0x04,0x04,0x18,0x17,0x16}},
{0x00,1,{0x90}},
{0xCC,8,{0x07,0x09,0x01,0x25,0x25,0x22,0x24,0x03}},
{0x00,1,{0x80}},
{0xCD,16,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x29,0x24,0x04,0x04,0x18,0x17,0x16}},
{0x00,1,{0x90}},
{0xCD,8,{0x06,0x08,0x01,0x25,0x25,0x22,0x24,0x02}},
{0x00,1,{0xA0}},
{0xCC,16,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x29,0x24,0x04,0x04,0x18,0x17,0x16}},
{0x00,1,{0xB0}},
{0xCC,8,{0x08,0x06,0x01,0x25,0x25,0x24,0x23,0x02}},
{0x00,1,{0xA0}},
{0xCD,16,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x29,0x24,0x04,0x04,0x18,0x17,0x16}},
{0x00,1,{0xB0}},
{0xCD,8,{0x09,0x07,0x01,0x25,0x25,0x24,0x23,0x03}},
{0x00,1,{0x80}},
{0xC0,6,{0x00,0xB0,0x00,0x50,0x00,0x10}},
{0x00,1,{0xA0}},
{0xC0,6,{0x01,0x7D,0x00,0x2D,0x00,0x10}},
{0x00,1,{0xB0}},
{0xC0,5,{0x00,0xB0,0x01,0xAF,0x10}},
{0x00,1,{0xC1}},
{0xC0,8,{0x00,0xFA,0x00,0xC5,0x00,0xA7,0x01,0x27}},
{0x00,1,{0xD7}},
{0xC0,6,{0x00,0xA7,0x00,0x53,0x00,0x10}},
{0x00,1,{0xA5}},
{0xC1,4,{0x00,0x37,0x00,0x02}},
{0x00,1,{0x82}},
{0xCE,13,{0x01,0x09,0x00,0x90,0x00,0x90,0x00,0x60,0x00,0x60,0x0D,0x0E,0x09}},
{0x00,1,{0x90}},
{0xCE,8,{0x00,0x82,0x0D,0x5C,0x00,0x82,0x80,0x09}},
{0x00,1,{0xA0}},
{0xCE,3,{0x00,0x00,0x00}},
{0x00,1,{0xB0}},
{0xCE,3,{0x11,0x00,0x00}},
{0x00,1,{0xD1}},
{0xCE,7,{0x00,0x0A,0x01,0x01,0x00,0x41,0x01}},
{0x00,1,{0xE1}},
{0xCE,11,{0x08,0x02,0x15,0x02,0x15,0x02,0x15,0x00,0x1E,0x00,0x44}},
{0x00,1,{0xF1}},
{0xCE,9,{0x0F,0x07,0x0A,0x01,0x43,0x02,0x10,0x01,0xCE}},
{0x00,1,{0xB0}},
{0xCF,4,{0x00,0x00,0x48,0x4C}},
{0x00,1,{0xB5}},
{0xCF,4,{0x03,0x03,0x18,0x1C}},
{0x00,1,{0xC0}},
{0xCF,4,{0x05,0x05,0xDC,0xE0}},
{0x00,1,{0xC5}},
{0xCF,4,{0x00,0x00,0x08,0x0C}},
{0x00,1,{0x90}},
{0xC0,6,{0x00,0xB0,0x00,0x50,0x00,0x10}},
{0x00,1,{0x82}},
{0xCF,1,{0x06}},
{0x00,1,{0x84}},
{0xCF,1,{0x06}},
{0x00,1,{0x87}},
{0xCF,1,{0x06}},
{0x00,1,{0x89}},
{0xCF,1,{0x06}},
{0x00,1,{0x8A}},
{0xCF,1,{0x07}},
{0x00,1,{0x8B}},
{0xCF,1,{0x00}},
{0x00,1,{0x8C}},
{0xCF,1,{0x06}},
{0x00,1,{0x92}},
{0xCF,1,{0x06}},
{0x00,1,{0x94}},
{0xCF,1,{0x06}},
{0x00,1,{0x97}},
{0xCF,1,{0x06}},
{0x00,1,{0x99}},
{0xCF,1,{0x06}},
{0x00,1,{0x9A}},
{0xCF,1,{0x07}},
{0x00,1,{0x9B}},
{0xCF,1,{0x00}},
{0x00,1,{0x9C}},
{0xCF,1,{0x06}},
{0x00,1,{0xA0}},
{0xCF,1,{0x24}},
{0x00,1,{0xA2}},
{0xCF,1,{0x06}},
{0x00,1,{0xA4}},
{0xCF,1,{0x06}},
{0x00,1,{0xA7}},
{0xCF,1,{0x06}},
{0x00,1,{0xA9}},
{0xCF,1,{0x06}},
{0x00,1,{0x87}},
{0xA4,1,{0x0F}},
{0x00,1,{0x89}},
{0xA4,1,{0x0F}},
{0x00,1,{0x8D}},
{0xA4,1,{0x0F}},
{0x00,1,{0x8F}},
{0xA4,1,{0x0F}},
{0x00,1,{0x85}},
{0xA7,1,{0x00}},
{0x00,1,{0xE8}},
{0xC0,1,{0x40}},
{0x00,1,{0x85}},
{0xC4,1,{0x1E}},
{0x00,1,{0x00}},
{0xE1,40,{0x00,0x03,0x05,0x10,0x36,0x1D,0x25,0x2B,0x35,0x0A,0x3D,0x44,0x4A,0x4F,0xE7,0x54,0x5C,0x64,0x6B,0x60,0x72,0x79,0x80,0x89,0x0D,0x93,0x99,0x9F,0xA7,0x8C,0xB0,0xBA,0xC8,0xD2,0x3C,0xDB,0xEB,0xF7,0xFF,0x6B}},
{0x00,1,{0x00}},
{0xE2,40,{0x00,0x03,0x05,0x10,0x34,0x17,0x1F,0x25,0x2F,0x4A,0x37,0x3E,0x44,0x49,0xE7,0x4E,0x56,0x5E,0x65,0x60,0x6C,0x73,0x7A,0x83,0x0D,0x8D,0x93,0x99,0xA1,0x8C,0xAA,0xB4,0xC2,0xCC,0x3C,0xDA,0xEB,0xF7,0xFF,0xEB}},
{0x00,1,{0x00}},
{0xE3,40,{0x00,0x03,0x05,0x10,0x36,0x1D,0x25,0x2B,0x35,0x0A,0x3D,0x44,0x4A,0x4F,0xE7,0x54,0x5C,0x64,0x6B,0x60,0x72,0x79,0x80,0x89,0x0D,0x93,0x99,0x9F,0xA7,0x8C,0xB0,0xBA,0xC8,0xD2,0x3C,0xDB,0xEB,0xF7,0xFF,0x6B}},
{0x00,1,{0x00}},
{0xE4,40,{0x00,0x03,0x05,0x10,0x34,0x17,0x1F,0x25,0x2F,0x4A,0x37,0x3E,0x44,0x49,0xE7,0x4E,0x56,0x5E,0x65,0x60,0x6C,0x73,0x7A,0x83,0x0D,0x8D,0x93,0x99,0xA1,0x8C,0xAA,0xB4,0xC2,0xCC,0x3C,0xDA,0xEB,0xF7,0xFF,0xEB}},
{0x00,1,{0x00}},
{0xE5,40,{0x00,0x03,0x05,0x10,0x36,0x1D,0x25,0x2B,0x35,0x0A,0x3D,0x44,0x4A,0x4F,0xE7,0x54,0x5C,0x64,0x6B,0x60,0x72,0x79,0x80,0x89,0x0D,0x93,0x99,0x9F,0xA7,0x8C,0xB0,0xBA,0xC8,0xD2,0x3C,0xDB,0xEB,0xF7,0xFF,0x6B}},
{0x00,1,{0x00}},
{0xE6,40,{0x00,0x03,0x05,0x10,0x34,0x17,0x1F,0x25,0x2F,0x4A,0x37,0x3E,0x44,0x49,0xE7,0x4E,0x56,0x5E,0x65,0x60,0x6C,0x73,0x7A,0x83,0x0D,0x8D,0x93,0x99,0xA1,0x8C,0xAA,0xB4,0xC2,0xCC,0x3C,0xDA,0xEB,0xF7,0xFF,0xEB}},
{0x00,1,{0x82}},
{0xC5,2,{0x50,0x50}},
{0x00,1,{0x84}},
{0xC5,3,{0x32,0x32,0x60}},
{0x00,1,{0x87}},
{0xC5,2,{0x60,0x0C}},
{0x00,1,{0x00}},
{0xD8,2,{0x29,0x29}},
{0x00,1,{0xA3}},
{0xC5,1,{0x19}},
{0x00,1,{0xA9}},
{0xC5,1,{0x23}},
{0x00,1,{0x80}},
{0xA4,1,{0x70}},
{0x00,1,{0x80}},
{0xC5,2,{0x86,0x86}},
{0x00,1,{0x80}},
{0xA7,1,{0x03}},
{0x00,1,{0x8C}},
{0xC3,3,{0x03,0x00,0x30}},
{0x00,1,{0xB0}},
{0xF5,1,{0x00}},
{0x00,1,{0x82}},
{0xA4,2,{0x29,0x23}},
{0x00,1,{0x89}},
{0xC0,3,{0x03,0x2A,0x05}},
{0x00,1,{0xE8}},
{0xC0,1,{0x40}},
{0x00,1,{0xB0}},
{0xF3,2,{0x15,0xEA}},
{0x00,1,{0xCB}},
{0xC0,1,{0x19}},
{0x11,0,{}},
{REGFLAG_DELAY,120, {}},
{0x29,0,{}},
{0x35,1,{0x00}},
{REGFLAG_DELAY,200, {}},

};

static struct LCM_setting_table lcm_suspend_setting[] = {
#if 1
	{0x28, 0, {}},
	{REGFLAG_DELAY, 120, {} },
	{0x10, 0, {}},
	{REGFLAG_DELAY, 30, {} },
	//{0x00, 1, {0x00}},
	//{0xFF, 3, {0x87,0x56,0x01}},
	//{0x00, 1, {0x80}},
	//{0xFF, 2, {0x87,0x56}},
	//{0x00, 1, {0x00}},
	//{0xF7, 4, {0x5A,0xA5,0x95,0x27}},

	//{REGFLAG_END_OF_TABLE, 0x00, {}}
#else
	{0x28, 0, {}},
	{REGFLAG_DELAY, 30, {} },
	{0x10, 0, {}},
	{REGFLAG_DELAY, 120, {} },
	{0x00, 1, {0x00}},
	{0xF7, 4, {0x5A,0xA5,0x95,0x27}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}  
#endif
};

static void push_table(struct LCM_setting_table *table, unsigned int count,
		       unsigned char force_update)
{
	unsigned int i;
    LCM_LOGI("nt35695----tps6132-lcm_init   push_table++++++++++++++===============================devin----\n");
	for (i = 0; i < count; i++) {
		unsigned cmd;

		cmd = table[i].cmd;

		switch (cmd) {

		case REGFLAG_DELAY:
			if (table[i].count <= 10)
				MDELAY(table[i].count);
			else
				MDELAY(table[i].count);
			break;

		case REGFLAG_END_OF_TABLE:
			break;

		default:
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}

/* --------------------------------------------------------------------------- */
/* LCM Driver Implementations */
/* --------------------------------------------------------------------------- */

static void lcm_set_util_funcs(const struct LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(struct LCM_UTIL_FUNCS));
}

static void lcm_get_params(struct LCM_PARAMS *params)
{
	memset(params, 0, sizeof(struct LCM_PARAMS));
	
	params->type = LCM_TYPE_DSI;
	
	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	
	// enable tearing-free
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
	#if (LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
	params->dsi.switch_mode = SYNC_PULSE_VDO_MODE;
	#else
	params->dsi.mode   = SYNC_PULSE_VDO_MODE;//SYNC_EVENT_VDO_MODE;//BURST_VDO_MODE;////
	#endif
		
	// DSI
	/* Command mode setting */
	//1 Three lane or Four lane
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format		= LCM_DSI_FORMAT_RGB888;
		
		
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		
	#if (LCM_DSI_CMD_MODE)
	params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
	params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
	#else
	params->dsi.intermediat_buffer_num = 0; //because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
	#endif
	
	// Video mode setting
	params->dsi.packet_size=256;
	
	params->dsi.vertical_sync_active     = 20;
	params->dsi.vertical_backporch       = 21;
	params->dsi.vertical_frontporch      = 120;
	params->dsi.vertical_active_line     = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active   = 6;
	params->dsi.horizontal_backporch     = 40;//36
	params->dsi.horizontal_frontporch    = 170;//78
	params->dsi.horizontal_active_pixel  = FRAME_WIDTH;

    params->dsi.PLL_CLOCK= 290;        //4LANE   232
	params->dsi.ssc_disable = 0;
	params->dsi.ssc_range = 1;
	params->dsi.cont_clock = 0;
	params->dsi.clk_lp_per_line_enable = 0;
	//prize-penggy modify LCD size-20190328-start
	params->physical_width = LCM_PHYSICAL_WIDTH/1000;
	params->physical_height = LCM_PHYSICAL_HEIGHT/1000;
	//prize-penggy modify LCD size-20190328-end
	params->physical_width_um = LCM_PHYSICAL_WIDTH;
	params->physical_height_um = LCM_PHYSICAL_HEIGHT;
	/*prize penggy add-for LCD ESD-20190219-start*/
	#if 1
	params->dsi.ssc_disable = 0;
	params->dsi.lcm_ext_te_monitor = FALSE;
		
	params->dsi.esd_check_enable = 0;
	params->dsi.customization_esd_check_enable = 0;
	params->dsi.lcm_esd_check_table[0].cmd			= 0x0a;
	params->dsi.lcm_esd_check_table[0].count		= 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;
	#endif
}

static unsigned int lcm_compare_id(void)
{
   return 1; 
}
#ifndef BUILD_LK
extern atomic_t ESDCheck_byCPU;
#endif
static unsigned int lcm_ata_check(unsigned char *bufferr)
{
	//prize-Solve ATA testing-pengzhipeng-20181127-start
	unsigned char buffer1[2]={0};
	unsigned char buffer2[2]={0};
	
	unsigned int data_array[6]; 
	 
	data_array[0]= 0x00023902;//LS packet 
	data_array[1]= 0x000050b8; 
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);
	atomic_set(&ESDCheck_byCPU, 1);
	
	read_reg_v2(0xb8, buffer1, 1);
	atomic_set(&ESDCheck_byCPU, 0);
	
	
	data_array[0]= 0x0002390a;//HS packet 
	data_array[1]= 0x000031b8; 
	dsi_set_cmdq(data_array, 2, 1);
	
	data_array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);
	atomic_set(&ESDCheck_byCPU, 1);
	
	read_reg_v2(0xb8, buffer2, 1);
	atomic_set(&ESDCheck_byCPU, 0);
	
	
	LCM_LOGI("%s, Kernel TDDI id buffer1= 0x%04x buffer2= 0x%04x\n", __func__, buffer1[0],buffer2[0]);
	return ((0x50 == buffer1[0])&&(0x31 == buffer2[0]))?1:0; 
	//prize-Solve ATA testing-pengzhipeng-20181127-end
}

static void lcm_init(void)
{
	display_ldo28_enable(1);
    display_ldo18_enable(1);

    MDELAY(5);
	display_bias_enable();
	MDELAY(10);

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);  

	SET_RESET_PIN(1);
	MDELAY(10);//250
	
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting)/sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
    push_table(lcm_suspend_setting, sizeof(lcm_suspend_setting)/sizeof(struct LCM_setting_table), 1);
	     
	display_bias_disable();
	MDELAY(10);//100
	
	//SET_RESET_PIN(0);	//prize-wyq 20190315 keep reset pin high to fix suspend current leakage
	//MDELAY(10);//100
}

static void lcm_resume(void)
{
	lcm_init();
}

static void lcm_init_power(void)
{
	display_bias_enable();
}

static void lcm_suspend_power(void)
{
	display_bias_disable();
}

static void lcm_resume_power(void)
{
	SET_RESET_PIN(0); //prize-wyq 20190315 keep reset pin high to fix suspend current leakage
	display_bias_enable();
}

struct LCM_DRIVER ft8615_fhdp_dsi_vdo_tcl_drv = 
{
    .name			= "ft8615_fhdp_dsi_vdo_tcl",
	#if defined(CONFIG_PRIZE_HARDWARE_INFO) && !defined (BUILD_LK)
	.lcm_info = {
		.chip	= "ft8615",
		.vendor	= "focaltech",
		.id		= "0x80",
		.more	= "720*1560",
	},
	#endif
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.init_power = lcm_init_power,
	.resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	.ata_check 		= lcm_ata_check,
};
