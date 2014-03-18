CROSS_COMPILE=arm-none-eabi-
QEMU_STM32 ?= ../qemu_stm32/arm-softmmu/qemu-system-arm

ARCH=CM3
VENDOR=ST
PLAT=STM32F10x
CODEBASE= freertos
CMSIS_LIB=$(CODEBASE)/libraries/CMSIS/$(ARCH)
STM32_LIB=$(CODEBASE)/libraries/STM32F10x_StdPeriph_Driver

CMSIS_PLAT_SRC = $(CMSIS_LIB)/DeviceSupport/$(VENDOR)/$(PLAT)

FREERTOS_SRC = $(CODEBASE)/libraries/FreeRTOS
FREERTOS_INC = $(FREERTOS_SRC)/include/                                       
FREERTOS_PORT_INC = $(FREERTOS_SRC)/portable/GCC/ARM_$(ARCH)/

HEAP_IMPL = heap_ww

all: main.bin

main.bin: test-romfs.o main.c clib.c fio.c shell.c host.c mmtest.c
	$(CROSS_COMPILE)gcc \
		-I. -I$(FREERTOS_INC) -I$(FREERTOS_PORT_INC) \
		-I$(CODEBASE)/libraries/CMSIS/CM3/CoreSupport \
		-I$(CODEBASE)/libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x \
		-I$(CODEBASE)/libraries/STM32F10x_StdPeriph_Driver/inc \
		-fno-common -O0 \
		-std=c99 -pedantic \
		-gdwarf-2 -ffreestanding -g3 \
		-mcpu=cortex-m3 -mthumb \
		-c \
		\
		$(CMSIS_LIB)/CoreSupport/core_cm3.c \
		$(CMSIS_PLAT_SRC)/system_stm32f10x.c \
		$(CMSIS_PLAT_SRC)/startup/gcc_ride7/startup_stm32f10x_md.s \
		$(STM32_LIB)/src/stm32f10x_rcc.c \
		$(STM32_LIB)/src/stm32f10x_gpio.c \
		$(STM32_LIB)/src/stm32f10x_usart.c \
		$(STM32_LIB)/src/stm32f10x_exti.c \
		$(STM32_LIB)/src/misc.c \
		\
		$(FREERTOS_SRC)/croutine.c \
		$(FREERTOS_SRC)/list.c \
		$(FREERTOS_SRC)/queue.c \
		$(FREERTOS_SRC)/tasks.c \
		$(FREERTOS_SRC)/portable/GCC/ARM_CM3/port.c \
		$(FREERTOS_SRC)/portable/MemMang/$(HEAP_IMPL).c \
		\
		stm32_p103.c \
		\
		romfs.c \
		hash-djb2.c \
		filesystem.c \
		fio.c \
		\
		osdebug.c \
		string-util.c \
		\
		main.c \
		clib.c \
		shell.c \
		host.c \
		mmtest.c
	$(CROSS_COMPILE)ld -Tmain.ld -nostartfiles -o main.elf \
		core_cm3.o \
		system_stm32f10x.o \
		startup_stm32f10x_md.o \
		stm32f10x_rcc.o \
		stm32f10x_gpio.o \
		stm32f10x_usart.o \
		stm32f10x_exti.o \
		misc.o \
		\
		croutine.o list.o queue.o tasks.o \
		port.o $(HEAP_IMPL).o \
		\
		stm32_p103.o \
		\
		romfs.o hash-djb2.o filesystem.o fio.o \
		\
		osdebug.o \
		string-util.o \
		clib.o \
		shell.o \
		host.o \
		mmtest.o \
		\
		main.o
	$(CROSS_COMPILE)objcopy -Obinary main.elf main.bin
	$(CROSS_COMPILE)objdump -S main.elf > main.list


mkromfs:
	gcc -o mkromfs mkromfs.c

CPU=arm
TARGET_FORMAT = elf32-littlearm
TARGET_OBJCOPY_BIN = $(CROSS_COMPILE)objcopy -I binary -O $(TARGET_FORMAT) --binary-architecture $(CPU)

test-romfs.o: mkromfs
	./mkromfs -d test-romfs test-romfs.bin
	$(TARGET_OBJCOPY_BIN) --prefix-sections '.romfs' test-romfs.bin test-romfs.o


qemu: main.bin $(QEMU_STM32)
	$(QEMU_STM32) -M stm32-p103 -kernel main.bin -semihosting

qemudbg: main.bin $(QEMU_STM32)
	$(QEMU_STM32) -M stm32-p103 \
		-gdb tcp::3333 -S \
		-kernel main.bin -semihosting

clean:
	rm -f *.o *.elf *.bin *.list mkromfs

