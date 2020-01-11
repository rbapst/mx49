#!/bin/bash
HWDEV=hw:1,0,0
REQUEST="F07E7F0601F7"
NAME=IdentityRequest
DEVICE=mx49
amidi -p hw:1,0,0 -S $HWREQUEST  -r $DEVICE-${NAME}.syx -t1
hd $DEVICE-${NAME}.syx
