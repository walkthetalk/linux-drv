
is_usr_mod=$1
is_dbg_version=$2

echo 1st is $1
echo 2nd is $2
echo 3rd is $3

make -f Makefile.lib DRV_USR_MODE=$is_usr_mod DEBUG=$is_dbg_version $3
#make DRV_USR_MODE=$is_usr_mod DEBUG=$is_dbg_version $3
#make -f Makefile.test DRV_USR_MODE=$is_usr_mod DEBUG=$is_dbg_version $3

