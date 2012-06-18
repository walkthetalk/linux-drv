################################################################################
# pdm_drv
# the driver used by process & device manger
################################################################################
############################################## called from kernel
# check arg
ifneq (${KERNELRELEASE},)

ifeq ($(__PDM_DRV_VSN),)
$(warning unkown __PDM_DRV_VSN)
__PDM_DRV_VSN := unkown
endif

VAR_NEED_DETCT := TIN_PLF_DIR \
                  TIN_PRD_DIR \
                  __PDM_DRV_VSN \
                  KERNEL_SRC \
                  M

define VAR_DETECT_template =
ifeq ($${$1},)
$$(error you must define $1)
else
$$(warning $1 is $${$1})
endif
endef

$(foreach temp,${VAR_NEED_DETCT},${eval $(call VAR_DETECT_template,${temp})})

# variables
prj_name := pdm_drv

inc_dir1 := $(TIN_PLF_DIR)/include/driver
inc_dir2 := $(TIN_PRD_DIR)/include/driver
inc_dir3 := $(M)/include
src_dir := $(M)/src

src_dirs_tmp := $(shell find $(src_dir) -type d)
src_dirs := $(filter-out $(src_dir)/%.svn%,$(src_dirs_tmp))
source_files := $(foreach tmp_dir, $(src_dirs), $(wildcard $(tmp_dir)/*.c))
obj_files := $(patsubst $(M)/%.c,%.o,$(source_files))

EXTRA_CFLAGS += -I$(inc_dir1) -I$(inc_dir2) -I$(inc_dir3) -DEXPORT_SYMTAB -DMODULE -DDRV_KERNEL_SIDE
EXTRA_CFLAGS += -D__PDM_DRV_VSN='"$(__PDM_DRV_VSN)"'
EXTRA_CFLAGS += -Werror
ifeq (${CHIP_SIMULATION},yes)
EXTRA_CFLAGS += -D__TIN_SIM_HW__
else
EXTRA_CFLAGS += -U__TIN_SIM_HW__
endif

obj-m += $(prj_name).o
$(prj_name)-objs := $(obj_files)
############################################## called from CUI
else


all:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) modules_install

clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers

endif

