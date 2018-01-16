CC = arm-none-eabi-gcc
CFLAGS = -O0 -g3 -Wall -mcpu=cortex-a8 -mthumb
AS = arm-none-eabi-as
ASFLAGS = -g3 -Wall
SRC = src
OBJ = obj
KERN = kern
KERNEL_MODULES = context context-demo
KERNEL_OBJS = $(KERNEL_MODULES:%=$(OBJ)/%.o)

$(KERN)/kernel.elf: $(KERNEL_OBJS)
	@test -d $(KERN) || mkdir $(KERN)
	$(CC) $(CFLAGS) --specs=nosys.specs -o $(KERN)/kernel.elf $(KERNEL_OBJS)

$(OBJ)/%.o: $(SRC)/%.c
	@test -d $(OBJ) || mkdir $(OBJ)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/context.o: $(SRC)/context.S
	@test -d $(OBJ) || mkdir $(OBJ)
	$(AS) $(ASFLAGS) -c -o $(OBJ)/context.o $(SRC)/context.S

QEMU = qemu-system-arm
# try to generate a unique GDB port
GDBPORT	:= $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -nographic -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

KERNEL = $(KERN)/kernel.elf
QEMUOPTS = -kernel $(KERN)/kernel.elf -machine realview-pb-a8 -cpu cortex-a8 \
	-nographic -monitor null -serial null -semihosting $(QEMUEXTRA)

.gdbinit: .gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu-gdb: $(KERNEL) .gdbinit
	@echo "***"
	@echo "*** Now run 'arm-none-eabi-gdb'." 1>&2
	@echo "***"
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)


clean:
	rm $(OBJ)/*.o
