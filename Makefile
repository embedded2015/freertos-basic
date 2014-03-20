TARGET = main
.DEFAULT_GOAL = all

CROSS_COMPILE ?= arm-none-eabi-
CC := $(CROSS_COMPILE)gcc
CFLAGS = -fno-common -O0 \
	 -std=c99 -pedantic \
	 -gdwarf-2 -ffreestanding -g3 \
	 -mcpu=cortex-m3 -mthumb \
	 -Wall -Werror \
	 -Tmain.ld -nostartfiles \
	 -DUSER_NAME=\"$(USER)\"

ARCH = CM3
VENDOR = ST
PLAT = STM32F10x

LIBDIR = .
CODEBASE = freertos
CMSIS_LIB = $(CODEBASE)/libraries/CMSIS/$(ARCH)
STM32_LIB = $(CODEBASE)/libraries/STM32F10x_StdPeriph_Driver

CMSIS_PLAT_SRC = $(CMSIS_LIB)/DeviceSupport/$(VENDOR)/$(PLAT)

FREERTOS_SRC = $(CODEBASE)/libraries/FreeRTOS
FREERTOS_INC = $(FREERTOS_SRC)/include/                                       
FREERTOS_PORT_INC = $(FREERTOS_SRC)/portable/GCC/ARM_$(ARCH)/

OUTDIR = build
SRCDIR = src\
         $(CMSIS_LIB)/CoreSupport \
         $(STM32_LIB)/src \
         $(CMSIS_PLAT_SRC) \
	 $(FREERTOS_SRC)
INCDIR = include \
         $(CMSIS_LIB)/CoreSupport \
         $(STM32_LIB)/inc \
         $(CMSIS_PLAT_SRC) \
	 $(FREERTOS_INC) \
	 $(FREERTOS_PORT_INC)
INCLUDES = $(addprefix -I,$(INCDIR))
DATDIR = data
TOOLDIR = tool
TMPDiR = output

HEAP_IMPL = heap_ww
SRC = $(wildcard $(addsuffix /*.c,$(SRCDIR))) \
      $(wildcard $(addsuffix /*.s,$(SRCDIR))) \
      $(FREERTOS_SRC)/portable/MemMang/$(HEAP_IMPL).c \
      $(FREERTOS_SRC)/portable/GCC/ARM_CM3/port.c \
      $(CMSIS_PLAT_SRC)/startup/gcc_ride7/startup_stm32f10x_md.s
OBJ := $(addprefix $(OUTDIR)/,$(patsubst %.s,%.o,$(SRC:.c=.o)))
DEP = $(OBJ:.o=.o.d)
DAT =
 
MAKDIR = mk
MAK = $(wildcard $(MAKDIR)/*.mk)

include $(MAK)


all: $(OUTDIR)/$(TARGET).bin $(OUTDIR)/$(TARGET).lst

$(OUTDIR)/$(TARGET).bin: $(OUTDIR)/$(TARGET).elf
	@echo "    OBJCOPY "$@
	@$(CROSS_COMPILE)objcopy -Obinary $< $@

$(OUTDIR)/$(TARGET).lst: $(OUTDIR)/$(TARGET).elf
	@echo "    LIST    "$@
	@$(CROSS_COMPILE)objdump -S $< > $@

$(OUTDIR)/$(TARGET).elf: $(OBJ) $(DAT)
	@echo "    LD      "$@
	@echo "    MAP     "$(OUTDIR)/$(TARGET).map
	@$(CROSS_COMPILE)gcc $(CFLAGS) -Wl,-Map=$(OUTDIR)/$(TARGET).map -o $@ $^

$(OUTDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "    CC      "$@
	@$(CROSS_COMPILE)gcc $(CFLAGS) -MMD -MF $@.d -o $@ -c $(INCLUDES) $<

$(OUTDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo "    CC      "$@
	@$(CROSS_COMPILE)gcc $(CFLAGS) -MMD -MF $@.d -o $@ -c $(INCLUDES) $<

clean:
	rm -rf $(OUTDIR) $(TMPDIR)

-include $(DEP)



