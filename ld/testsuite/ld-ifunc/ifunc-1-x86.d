#ld: -shared
#objdump: -dw
#target: x86_64-*-* i?86-*-*

#...
[ \t0-9a-f]+:[ \t0-9a-f]+call[ \t0-9a-fq]+<\*ABS\*@plt>
#pass