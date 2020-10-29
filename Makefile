#CONFIG_VS_FRONTEND = m

#ifeq ($(ARCH), arm)
#obj-$(CONFIG_VS_FRONTEND) += vs_frontend_32.o
#vs_frontend_32-objs = $(OBJS)
#else
#obj-$(CONFIG_VS_FRONTEND) += vs_frontend_64.o
#vs_frontend_64-objs = $(OBJS)
#endif

obj-m += vs_frontend_fe.o
vs_frontend_fe-objs = $(OBJS)

OBJS += aml_dvb.o aml_dmx.o vs_fe.o vs_fe_tuner.o vs_fe_common.o vs_fe_external.o

###################################
#AVL68XX ENTRY                    #
###################################
$(warning #####AVL68XX#####)
avl68xx_src = $(M)/stream_input/vs_frontend/avl68xx
avl68xx_demod = $(avl68xx_src)/sdk_src
avl68xx_tuner = $(avl68xx_src)/tuner

$(warning avl68xx_src: $(avl68xx_src))
ccflags-y += -I$(avl68xx_src) -I$(avl68xx_demod) -I$(avl68xx_tuner) -I$(avl68xx_tuner)/MxL608 -I$(avl68xx_tuner)/R848_V25
ccflags-y += -DAVL68XX_MAIN_XTAL_VALUE=Xtal_30M -DAVL68XX_SECOND_XTAL_VALUE=Xtal_30M
ccflags-y += -DMX608_XTAL_VALUE=MXL608_XTAL_16MHz
ccflags-y += -DMX608_XTAL_SINGLE_SUPPLY_3_3_V_ENABLE=MXL608_ENABLE
ccflags-y += -DMX608_XTAL_CLOCK_OUTPUT_ENABLE=MXL608_DISABLE
ccflags-y += -DMX608_LOOP_THROUGH_ENABLE=MXL608_DISABLE
ccflags-y += -DMX608_IF_OUT_INVERSION_ENABLE=MXL608_DISABLE

avl68xx_src = avl68xx
avl68xx_demod = $(avl68xx_src)/sdk_src
avl68xx_tuner = $(avl68xx_src)/tuner

$(warning avl68xx_src: $(avl68xx_src))
OBJS += $(avl68xx_src)/avl68xx_app_porting.o $(avl68xx_src)/AVL68XX_Porting.o $(avl68xx_src)/AVL68XX_IBlindScan.o $(avl68xx_src)/AVL68XX_IBlindscanAPI.o
OBJS += $(avl68xx_demod)/AVL_Demod_CommonAPI.o \
                   $(avl68xx_demod)/AVL_Demod_CommonInternal.o \
                   $(avl68xx_demod)/AVL_Demod_Config.o \
                   $(avl68xx_demod)/AVL_Demod_DVBC.o \
                   $(avl68xx_demod)/AVL_Demod_DVBSx.o \
                   $(avl68xx_demod)/AVL_Demod_DVBTx.o \
                   $(avl68xx_demod)/AVL_Demod_ISDBT.o \
                   $(avl68xx_demod)/user_defined_function.o

OBJS += $(avl68xx_tuner)/MxL608/MxL608.o \
                   $(avl68xx_tuner)/MxL608/MxL608_OEM_Drv.o \
                   $(avl68xx_tuner)/MxL608/MxL608_TunerApi.o \
                   $(avl68xx_tuner)/MxL608/MxL608_TunerCfg.o \
                   $(avl68xx_tuner)/MxL608/MxL608_TunerSpurTable.o
OBJS += $(avl68xx_tuner)/R848_V25/R848.o \
                   $(avl68xx_tuner)/R848_V25/R848_API.o \
                   $(avl68xx_tuner)/R848_V25/R848_I2C.o
###################################
#AVL68XX EXIT                     #
###################################

###################################
#RS6060 ENTRY                     #
###################################
$(warning #####RS6060#####)
rs6060_src = $(M)/stream_input/vs_frontend/rs6060
rs6060_sdk_src = $(rs6060_src)/sdk_src

$(warning rs6060_src: $(rs6060_src))
ccflags-y += -I$(rs6060_src) -I$(rs6060_sdk_src)

rs6060_src = rs6060
rs6060_sdk_src = $(rs6060_src)/sdk_src

$(warning rs6060_src: $(rs6060_src))
OBJS += $(rs6060_src)/rs6060_app_porting.o
OBJS += $(rs6060_sdk_src)/mt_fe_DiSEqC_example.o \
                   $(rs6060_sdk_src)/mt_fe_dmd_rs6060.o \
                   $(rs6060_sdk_src)/mt_fe_example_rs6060.o \
                   $(rs6060_sdk_src)/mt_fe_i2c.o \
                   $(rs6060_sdk_src)/mt_fe_tn_montage_rs6060.o
###################################
#RS6060 EXIT                      #
###################################

#ccflags-y += -Wno-error=declaration-after-statement

## Sefault setting, invariant
########################################
## Spectrum invert: 0-->NORMAL(false), 1-->INVERT(true)
## TS out: 0-->SERIAL(false), 1-->PARALLEL(true)
## TS serial: 0-->DATA0(false), 1-->DATA7(true)
MAIN_SPECTRUM_INVERT := 1
SECOND_SPECTRUM_INVERT := 1
MAIN_TS_OUT_PARALLEL := 0
SECOND_TS_OUT_PARALLEL := 0
MAIN_TS_SERIAL_DATA7 := 1
SECOND_TS_SERIAL_DATA7 := 1
########################################

## User export, ts related, change here, for debug
########################################
ifneq ($(VS_SPECTRUM_INVERT), )
$(warning "export VS_SPECTRUM_INVERT: $(VS_SPECTRUM_INVERT)")
MAIN_SPECTRUM_INVERT := $(strip $(VS_SPECTRUM_INVERT))
SECOND_SPECTRUM_INVERT := $(strip $(VS_SPECTRUM_INVERT))
endif

ifneq ($(VS_TS_OUT_PARALLEL), )
$(warning "export VS_TS_OUT_PARALLEL: $(VS_TS_OUT_PARALLEL)")
MAIN_TS_OUT_PARALLEL := $(strip $(VS_TS_OUT_PARALLEL))
SECOND_TS_OUT_PARALLEL := $(strip $(VS_TS_OUT_PARALLEL))
endif

ifneq ($(VS_TS_SERIAL_DATA7), )
$(warning "export VS_TS_SERIAL_DATA7: $(VS_TS_SERIAL_DATA7)")
MAIN_TS_SERIAL_DATA7 := $(strip $(VS_TS_SERIAL_DATA7))
SECOND_TS_SERIAL_DATA7 := $(strip $(VS_TS_SERIAL_DATA7))
endif
########################################

MAIN_SPECTRUM_INVERT := $(strip $(MAIN_SPECTRUM_INVERT))
SECOND_SPECTRUM_INVERT := $(strip $(SECOND_SPECTRUM_INVERT))
MAIN_TS_OUT_PARALLEL := $(strip $(MAIN_TS_OUT_PARALLEL))
SECOND_TS_OUT_PARALLEL := $(strip $(SECOND_TS_OUT_PARALLEL))
MAIN_TS_SERIAL_DATA7 := $(strip $(MAIN_TS_SERIAL_DATA7))
SECOND_TS_SERIAL_DATA7 := $(strip $(SECOND_TS_SERIAL_DATA7))

$(info "########################################")
$(info "TARGET_PRODUCT: $(TARGET_PRODUCT), PRODUCT_MODEL: $(PRODUCT_MODEL)")
$(info "Main, spectrum_invert: $(MAIN_SPECTRUM_INVERT), ts_out_parallel: $(MAIN_TS_OUT_PARALLEL), serial_data7: $(MAIN_TS_SERIAL_DATA7)")
$(info "Second, spectrum_invert: $(SECOND_SPECTRUM_INVERT), ts_out_parallel: $(SECOND_TS_OUT_PARALLEL), serial_data7: $(SECOND_TS_SERIAL_DATA7)")
$(info "########################################")

ccflags-y += -DMAIN_SPECTRUM_INVERT=$(MAIN_SPECTRUM_INVERT) -DSECOND_SPECTRUM_INVERT=$(SECOND_SPECTRUM_INVERT) \
        -DMAIN_TS_OUT_PARALLEL=$(MAIN_TS_OUT_PARALLEL) -DSECOND_TS_OUT_PARALLEL=$(SECOND_TS_OUT_PARALLEL) \
        -DMAIN_TS_SERIAL_DATA7=$(MAIN_TS_SERIAL_DATA7) -DSECOND_TS_SERIAL_DATA7=$(SECOND_TS_SERIAL_DATA7)

ccflags-y += -Wno-error=unused-variable
ccflags-y += -Wno-error=implicit-function-declaration
ccflags-y += -Wno-error=unused-value
ccflags-y += -Wno-error=format
ccflags-y += -Wno-error=missing-field-initializers
ccflags-y += -Wno-error=unused-function
ccflags-y += -Wno-error=incompatible-pointer-types
ccflags-y += -Wno-error=missing-braces
ccflags-y += -Wno-error=frame-larger-than=
ccflags-y += -Wno-error=strict-prototypes

ccflags-y += -I$(srctree)/drivers/media/dvb-core
ccflags-y += -I.

ccflags-y += -I$(srctree)/drivers/amlogic/dvb