CC = arm-none-eabi-gcc
CFLAGS = -O0 -g3 -Wall -mcpu=cortex-a8 -mthumb
AS = arm-none-eabi-as
ASFLAGS = -g3 -Wall
SRC = src
OBJ = obj
KERNEL_MODULES = context context-demo
KERNEL_OBJS = $(KERNEL_MODULES:%=$(OBJ)/%.o)

main.elf: $(KERNEL_OBJS)
	$(CC) $(CFLAGS) --specs=nosys.specs -o main.elf $(KERNEL_OBJS)

$(OBJ)/%.o: $(SRC)/%.c
	@test -d $(OBJ) || mkdir $(OBJ)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/context.o: $(SRC)/context.S
	@test -d $(OBJ) || mkdir $(OBJ)
	$(AS) $(ASFLAGS) -c -o $(OBJ)/context.o $(SRC)/context.S

clean:
	rm $(OBJ)/*.o
