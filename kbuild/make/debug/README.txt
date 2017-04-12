This file introduce a rough make debug method. You can print some global Make
variables, also can get most make output during the kernel building.


How to get the make data:

make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- LOCALVERSION= -pq > makedbg.data


The usage just like below:

yuan_zc@troy-Lenovo-Product:~/linaro/work_dir/linux_master_L$ make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- LOCALVERSION= -f Makefile -f vars.mk -n scripts_basic d-build


How to get the commands output of compilation:

make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- LOCALVERSION= -n Image > compiledbg.txt



make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- LOCALVERSION= -f Makefile -f vars.mk -n --debug=v scripts_basic d-build > debugver.data
