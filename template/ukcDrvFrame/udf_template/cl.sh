
#is_usr_mod=$1
#is_dbg_version=$2

#echo 1st is $1
#echo 2nd is $2
#echo 3rd is $3

#工程名字
prj_for_card="cpm"
#
drv_prd_dir_set="$TIN_PRD_DIR/src/drv_cmm"
#驱动主体源码目录
drv_src_dir_set="$PWD/drvHa"
#驱动内核部分源码目录
drv_ksrc_dir_set="$PWD/drvKernelSrc"
#测试程序目录
drv_test_inc_set="$TIN_PLF_DIR/include/utility"
drv_test_dir_set="$PWD/drvTest:$TIN_PLF_DIR/src/utility"
#驱动主体头文件目录
drv_inc_dir_set="$TIN_PRD_DIR/include/driver:$PWD/drvInc"
#驱动内核部分头文件目录it should contain one file named *drv_self_cfg.h*
drv_kinc_dir_set="$TIN_PRD_DIR/include/driver:$PWD/drvKerneInc"
#输出目录
output_dir="$PWD/drv_output"

#external symbol file
pdm_drv_sym="$TIN_PRD_DIR/src/pdm/driver/Module.symvers"

pwd_dir=$PWD
udf_dir="$TIN_UDF_DIR"

#echo $drv_src_dir_set

make_opt="drv_prd_dir_set=$drv_prd_dir_set drv_ksrc_dir_set=$drv_ksrc_dir_set drv_kinc_dir_set=$drv_kinc_dir_set drv_test_dir_set=$drv_test_dir_set drv_test_inc_set=$drv_test_inc_set output_dir=$output_dir drv_src_dir_set=$drv_src_dir_set drv_inc_dir_set=$drv_inc_dir_set prj_for_card=$prj_for_card pdm_drv_sym=$pdm_drv_sym $1 $2 $3 $4 $5 $6 $7 $8 $9"

cd $udf_dir
make -f Makefile.lib $make_opt
if [ "$?" != "0" ]; then
    exit 1
fi
make $make_opt
if [ "$?" != "0" ]; then
    exit 1
fi
make -f Makefile.test $make_opt
if [ "$?" != "0" ]; then
    exit 1
fi
cd $pwd_dir

exit 0


