CC = arm-none-eabi-gcc
CFLAGS = -O0 -g3 -Wall -mcpu=cortex-a8 -mthumb
AS = arm-none-eabi-as
ASFLAGS = -g3 -Wall
SRC = src
OBJDIR = obj
KERNEL_MODULES = context kern_util kernel context-demo 
KERNEL_OBJS = $(KERNEL_MODULES:%=$(OBJDIR)/%.o)
KERNEL = kern/kernel.elf

$(KERNEL): $(KERNEL_OBJS)
	@test -d kern || mkdir kern
	$(CC) $(CFLAGS) --specs=nosys.specs -o $(KERNEL) $(KERNEL_OBJS)

$(OBJDIR)/%.o: $(SRC)/%.c
	@test -d $(OBJDIR) || mkdir $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/context.o: $(SRC)/context.S
	@test -d $(OBJDIR) || mkdir $(OBJDIR)
	$(AS) $(ASFLAGS) -c -o $(OBJDIR)/context.o $(SRC)/context.S

QEMU = qemu-system-arm
# try to generate a unique GDB port
GDBPORT	:= $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line
QEMUGDB = $(shell if $(QEMU) -nographic -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)


QEMUOPTS = -kernel $(KERNEL) -machine realview-pb-a8 -cpu cortex-a8 \
	-nographic -monitor null -serial null -semihosting $(QEMUEXTRA)

.gdbinit: .gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu-gdb: $(KERNEL) .gdbinit
	@echo "***"
	@echo "*** Now run 'arm-none-eabi-gdb'." 1>&2
	@echo "***"
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)


clean:
	rm $(OBJDIR)/*.o
