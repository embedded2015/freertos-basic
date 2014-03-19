QEMU_STM32 ?= ../qemu_stm32/arm-softmmu/qemu-system-arm
BUILD_TARGET = $(OUTDIR)/$(TARGET)

qemu: $(BUILD_TARGET).bin $(QEMU_STM32)
	$(QEMU_STM32) -M stm32-p103 \
	    -monitor stdio \
	    -kernel $(BUILD_TARGET).bin

qemudbg: $(BUILD_TARGET).bin $(QEMU_STM32)
	$(QEMU_STM32) -M stm32-p103 \
	    -gdb tcp::3333 -S \
	    -kernel $(BUILD_TARGET).bin -semihosting
