CC = arm-none-eabi-gcc
CFLAGS = -g -Wall
SRC = src
OBJ = obj
KERNEL_MODULES = context context-demo
KERNEL_OBJS = $(KERNEL_MODULES:%=$(OBJ)/%.o)

main.elf: $(KERNEL_OBJS)
	$(CC) --specs=nosys.specs -o main.elf $(KERNEL_OBJS) -g3 -mcpu=cortex-m3 -mthumb

$(OBJ)/%.o: $(SRC)/%.c
	@test -d $(OBJ) || mkdir $(OBJ)
	$(CC) $(CFLAGS) -c -g3 -o $@ $<
