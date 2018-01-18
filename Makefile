CC = arm-none-eabi-gcc
CFLAGS = -O0 -g3 -Wall -mcpu=cortex-a8 -mthumb
AS = arm-none-eabi-as
ASFLAGS = -g3 -Wall
OBJDIR = obj
KERN = kern
KERNEL_MODULES = context kern_util context-demo 
KERNEL_OBJS = $(KERNEL_MODULES:%=$(OBJDIR)/%.o)
KERNEL = kernel.elf
TOP = .

$(KERNEL): $(KERNEL_OBJS)
	$(CC) $(CFLAGS) --specs=nosys.specs -o $(KERNEL) $(KERNEL_OBJS)

$(OBJDIR)/%.o: $(KERN)/%.c
	@test -d $(OBJDIR) || mkdir $(OBJDIR)
	$(CC) $(CFLAGS) -I $(TOP) -c -o $@ $<

$(OBJDIR)/context.o: $(KERN)/context.S
	@test -d $(OBJDIR) || mkdir $(OBJDIR)
	$(AS) $(ASFLAGS) -c -o $(OBJDIR)/context.o $(KERN)/context.S

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


clean:
	rm $(OBJDIR)/*.o
