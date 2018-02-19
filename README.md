# LambKernel
Course work for Computer System Engineering II  
It's the course requirement to write everything in one file, `kernel.c`. :)

## Prerequisite on macOS

#### Download GCC for ARM and QEMU

Step 1 - Install [Homebrew](http://brew.sh/)

Step 2 - Install GCC Arm Toolchain and QEMU  
```
brew tap PX4/homebrew-px4
brew update
brew install gcc-arm-none-eabi-49
brew install qemu
```

## How to run and debug on macOS

```plaintext
James:~/LambKernel ► make
James:~/LambKernel ► make qemu-gdb                                                      
***
*** Now run 'arm-none-eabi-gdb'.
***
qemu-system-arm -kernel kern/kernel.elf -machine realview-pb-a8 -cpu cortex-a8 -nographic -monitor null -serial null -semihosting  -S -gdb tcp::25501
```

**Using arm-none-eabi-gdb to attach process in another terminal:**  

```plaintext
James:~/LambKernel ► arm-none-eabi-gdb                                          
GNU gdb (GNU Tools for ARM Embedded Processors) 7.8.0.20150604-cvs
Copyright (C) 2014 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "--host=x86_64-apple-darwin10 --target=arm-none-eabi".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
<http://www.gnu.org/software/gdb/documentation/>.
For help, type "help".
Type "apropos word" to search for commands related to "word".
/Users/james/.gdbinit:1: Error in sourced command file:
No symbol table is loaded.  Use the "file" command.
+ target remote localhost:25501
0x000080b0 in ?? ()
(gdb) b main
Breakpoint 1 at 0x81f8: file src/context-demo.c, line 15.
(gdb) c
Continuing.

Breakpoint 1, main () at src/context-demo.c:15
15	  Running = &taskA;
(gdb) 
```

## References  
[Install GCC for ARM](https://gist.github.com/joegoggins/7763637)  
[Simulation with QEMU on macOS](http://cgi.cs.indiana.edu/~geobrown/stm32/Main/Simulation)
