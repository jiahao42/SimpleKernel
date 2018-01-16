# LambKernel
Course work for Computer System Engineering II

## Prerequisite  

#### Download GCC for ARM and QEMU

Step 1 - Install [Homebrew](http://brew.sh/)

Step 2 - Install GCC Arm Toolchain and QEMU  
```
brew tap PX4/homebrew-px4
brew update
brew install gcc-arm-none-eabi-49
brew install qemu
```

## How to run and debug

```plaintext
James:~/LambKernel ► make
James:~/LambKernel ► qemu-system-arm -machine realview-pb-a8 -cpu cortex-a8 -nographic -monitor null -serial null -semihosting -kernel main.elf -gdb tcp::51234 -S
```

**Using gdb to attach process in another terminal:**  

```plaintext
James:~/LambKernel ► arm-none-eabi-gdb main.elf  
(gdb) target remote localhost:51234
Remote debugging using localhost:51234
0x000080b0 in _start ()
(gdb) load
Loading section .init, size 0xc lma 0x8000
Loading section .text, size 0x524 lma 0x800c
Loading section .fini, size 0xc lma 0x8530
Loading section .rodata, size 0x8 lma 0x853c
Loading section .ARM.exidx, size 0x8 lma 0x8544
Loading section .eh_frame, size 0x4 lma 0x854c
Loading section .init_array, size 0x8 lma 0x10550
Loading section .fini_array, size 0x4 lma 0x10558
Loading section .jcr, size 0x4 lma 0x1055c
Loading section .data, size 0x434 lma 0x10560
Start address 0x80b0, load size 2452
Transfer rate: 2394 KB/sec, 245 bytes/write.
(gdb) b main 
Breakpoint 1 at 0x81f8: file src/context-demo.c, line 15.
(gdb) c
Continuing.

Breakpoint 1, main () at src/context-demo.c:15
15	  Running = &taskA;
```

## References  
[Install GCC for ARM](https://gist.github.com/joegoggins/7763637)  
[Simulation with QEMU on macOS](http://cgi.cs.indiana.edu/~geobrown/stm32/Main/Simulation)
