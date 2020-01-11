#!/bin/bash
HWDEV=hw:1,0,0
REQUEST="F0 43 20 7F 17 0E 00 00 F7"
NAME=SYSTEM-System-bulk
DEVICE=mx49
amidi -p $HWDEV -S $REQUEST -r $DEVICE-${NAME}.syx -t1
echo $REQUEST
hd $DEVICE-${NAME}.syx
