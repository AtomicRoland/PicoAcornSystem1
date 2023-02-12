#!/bin/bash

beebasm -i $1.asm -v > $1.lst
objcopy -O ihex -I binary --change-address 0x200 $1.bin $1.hex
egrep ':.{6}0[0-1]' $1.hex > $1.obj
echo "char clockdemo { " > $1.h && hexdump -v -e '/1 "0x%02X,"' $1.bin >> $1.h && echo "};\n" >> $1.h
