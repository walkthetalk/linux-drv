

udf_dir := $(PWD)
#use argument
#pdm_drv_sym := $(TIN_PRD_DIR)/external/Module.symvers
#output_dir := $(udf_dir)/haha
#drv_src_dir :=

inc_dir += $(TIN_PLF_DIR)/include/driver
inc_dir += $(udf_dir)/include
inc_dir += $(subst :, ,$(drv_kinc_dir_set))
src_dir := $(udf_dir)/src

driver_udf_version :=$(shell cd $(udf_dir);hg identify -n |cut -d '+' -f 1)
driver_src_version :=$(shell cd $(output_dir);hg identify -n |cut -d '+' -f 1)

ifeq ($(DEBUG), y)
ko_dir := $(output_dir)/debug/bin
else
ko_dir := $(output_dir)/release/bin
endif

ifeq ($(DRV_USR_MODE), y)
prj_name := $(prj_for_card)_drv_u
EXTRA_CFLAGS += -DDRV_USR_MODE
else
prj_name := $(prj_for_card)_drv_k
src_dir_set += $(src_dir)/th
#src_dir_set += $(drv_src_dir)
endif

src_dir_set += $(src_dir)/ukc_kernel
src_dir_set += $(src_dir)/cp
src_dir_set += $(src_dir)/bh


src_dirs_tmp := $(shell find $(src_dir_set) -type d)
src_dirs := $(filter-out $(src_dir)/%.svn%,$(src_dirs_tmp))
src_dirs += $(src_dir)


# change the ext dir to a dir based on *udf_dir*
udf_dir_space := $(subst /, ,$(udf_dir))
udf_dir_back_space := $(foreach tmp_sdir,$(udf_dir_space),/..)
nullstring :=
space := $(nullstring) # a space at the end
udf_dir_back := $(subst $(space),,$(udf_dir_back_space))
root_dir_use_udf_dir := $(addprefix $(udf_dir),$(udf_dir_back))

#$(warning root_dir_use_udf_dir is $(root_dir_use_udf_dir))
##########################aux function##########################################
define get_rel_dir_from_set
  $(foreach tmp_dir,$(subst :, ,$1),$(root_dir_use_udf_dir)$(tmp_dir))
endef
define get_rel_dirs_tmp_from_set
  $(shell find $(call get_rel_dir_from_set, $1) -type d)
endef
define get_rel_dirs_from_set
  $(filter-out %/.svn/%,$(call get_rel_dirs_tmp_from_set, $1))
endef

#$(warning product_dirs is $(call get_rel_dirs_from_set, $(drv_cmm_dir_set)))
#########################add ext src files############################
ifeq ($(DRV_USR_MODE), y)
else
# the dependency between .o and .c is defined in kernel's Makefile, and
# I don't know how to modify it without bad influence.
inc_dir += $(subst :, ,$(drv_inc_dir_set))
src_dirs += $(call get_rel_dirs_from_set, $(drv_src_dir_set))
endif

#$(warning src_dirs is $(src_dirs))
#########################add ext ksrc files###########################
#########################user-defined bh content
# the dependency between .o and .c is defined in kernel's Makefile, and
# I don't know how to modify it without bad influence.
inc_dir += $(subst :, ,$(drv_kinc_dir_set))
src_dirs += $(call get_rel_dirs_from_set, $(drv_ksrc_dir_set))

##########################product_defined cmm files#############################
#$(warning src_dirs is $(src_dirs))
#$(warning prd_dir is $(drv_prd_dir_set))
#$(warning prd_dirs is $(call get_rel_dirs_from_set, $(drv_prd_dir_set)))

src_dirs += $(call get_rel_dirs_from_set, $(drv_prd_dir_set))

#$(warning src_dirs is $(src_dirs))


############################gen source files##########################
source_files := $(foreach tmp_dir, $(src_dirs), $(wildcard $(tmp_dir)/*.c))

#$(warning source_files is $(source_files))
obj_files := $(patsubst $(udf_dir)/%.c,%.o,$(source_files))
#$(warning obj_files is $(obj_files))


obj-m += $(prj_name).o
$(prj_name)-objs := $(obj_files)
#$(warning $(prj_name)-objs is $($(prj_name)-objs))

EXTRA_CFLAGS += $(foreach tmp_dir, $(inc_dir), -I$(tmp_dir)) -DEXPORT_SYMTAB -DMODULE -DDRV_KERNEL_SIDE

KBUILD_EXTRA_SYMBOLS := $(pdm_drv_sym)


ifeq ($(TIN_ARCH), powerpc)
#$(warning compiling powerpc ko)
ifeq ($(TIN_KERNELDIR),)
$(error you must assign *TIN_KERNELDIR*)
else
ASS_ARCH=ARCH=$(TIN_ARCH)
KERNELDIR = $(TIN_KERNELDIR)
endif
else
#$(warning comipiling $(shell uname -m) ko)
#$(warning you can change arch through variable *TIN_ARCH*)
ASS_ARCH=
KERNELDIR = /lib/modules/$(shell uname -r)/build

EXTRA_CFLAGS += -D__TIN_SIM_HW__
endif

EXTRA_CFLAGS += -Werror
EXTRA_CFLAGS += -D__DRV_UDF_VSN='"$(driver_udf_version)"'
EXTRA_CFLAGS += -D__DRV_SRC_VSN='"$(driver_src_version)"'
EXTRA_CFLAGS += $(TIN_CPPFLAGS_CMM)


all:
	@echo "****************************start make $(prj_name).ko*****************************"
	@make --no-print-directory $(ASS_ARCH) -C $(KERNELDIR) M=$(PWD) modules
	@cp $(prj_name).ko $(ko_dir)/
	@echo "****************************make $(prj_name).ko done******************************"
prepare:
	make $(ASS_ARCH) -C $(KERNELDIR) M=$(PWD) prepare
# I don't know why kernel's Makefile can't remove the .o in drv_src_dir_set, so
# what I can do is only add rm sentence after kernel-makefile's clean.
clean:
	@echo "****************************start clean $(prj_name).ko****************************"
	@echo "LKM clean"
	@make --no-print-directory $(ASS_ARCH) -C $(KERNELDIR) M=$(PWD) clean
	@echo "OWN clean"
	-@rm -f $(obj_files)
	-@rm -f $(ko_dir)/$(prj_name).ko
	@echo "***************************clean $(prj_name).ko done******************************"




