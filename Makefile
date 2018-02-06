CC = arm-none-eabi-gcc
CFLAGS = -O0 -g3 -std=c99 -Wall -mcpu=cortex-a8 -mthumb # -marm
AS = arm-none-eabi-as
ASFLAGS = -g3 -mcpu=cortex-a8
OBJDIR = obj
SRC = src
KERNEL_MODULES = context test kernel main
KERNEL_OBJS = $(KERNEL_MODULES:%=$(OBJDIR)/%.o)
KERNEL = kernel.elf
TOP = .

all:	_clearscreen	$(KERNEL)

$(KERNEL): $(KERNEL_OBJS)
	$(CC) $(CFLAGS) --specs=nosys.specs -o $(KERNEL) $(KERNEL_OBJS)

$(OBJDIR)/%.o: $(SRC)/%.c
	@test -d $(OBJDIR) || mkdir $(OBJDIR)
	$(CC) $(CFLAGS) -I $(TOP) -c -o $@ $<

$(OBJDIR)/context.o: $(SRC)/context.S
	@test -d $(OBJDIR) || mkdir $(OBJDIR)
	$(AS) $(ASFLAGS) -c -o $(OBJDIR)/context.o $(SRC)/context.S

QEMU = qemu-system-arm
# Use `qemu-system-arm -machine help -nographic` to check
MACHINE = realview-pb-a8
# Use 'qemu-system-arm -machine [Machine name] -cpu help` to check
CPU = cortex-a8

# try to generate a unique GDB port
GDBPORT	:= $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line
QEMUGDB = $(shell if $(QEMU) -nographic -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

QEMUOPTS = -kernel $(KERNEL) -machine $(MACHINE) -cpu $(CPU) \
	-nographic -monitor null -serial null -semihosting $(QEMUEXTRA)

.gdbinit: .gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu-gdb: $(KERNEL) .gdbinit
	@echo "***"
	@echo "*** Now run 'arm-none-eabi-gdb' in another terminal." 1>&2
	@echo "***"
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)

.PHONY: clean _clearscreen

clean:
	rm $(OBJDIR)/*.o

_clearscreen:
	clear
