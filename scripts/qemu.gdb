set pagination off
set disassembly-flavor intel
file kernel
target remote localhost:1234
break kmain
continue
