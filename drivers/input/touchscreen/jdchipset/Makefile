# Linux driver folder
ccflags-y += -I$(srctree)/drivers/input/touchscreen/jdchipset/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/i2c/$(MTK_PLATFORM)/

# Makefile for the Jadard touchscreen drivers.
# obj-$(CONFIG_TOUCHSCREEN_JADARD_IC_MODULE)      += jadard_module.o
# obj-$(CONFIG_TOUCHSCREEN_JADARD_IC_JD9365T)    += jadard_ic_JD9365T.o
# obj-$(CONFIG_TOUCHSCREEN_JADARD_COMMON)         += jadard_common.o jadard_platform.o
# obj-$(CONFIG_TOUCHSCREEN_JADARD_DEBUG)          += jadard_debug.o
# obj-$(CONFIG_TOUCHSCREEN_JADARD_INSPECT)        += jadard_sorting.o

obj-y      	+= jadard_module.o
obj-y    	+= jadard_ic_JD9365T.o
obj-y       += jadard_common.o jadard_platform.o
obj-y       += jadard_debug.o
obj-y       += jadard_sorting.o
