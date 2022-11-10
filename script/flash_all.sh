#!/bin/sh
TOOL_PATH=toolchain/openocd_x86_64/openocd_x86_64/bin
OPENOCD=${TOOL_PATH}/openocd

dd if=bootloader/bootloader.bin of=temp/fw.bin 
dd if=firmware/firmware.bin of=temp/fw.bin bs=1k seek=8
chmod a+x ${OPENOCD}
$OPENOCD -f ${TOOL_PATH}/wch-riscv.cfg -c "program temp/fw.bin verify reset exit"
#$OPENOCD -f ${TOOL_PATH}/wch-riscv.cfg -c "program bootloader/bootloader.bin verify reset exit"
