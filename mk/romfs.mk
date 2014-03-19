ROMDIR = $(DATDIR)/test-romfs
DAT += $(OUTDIR)/$(DATDIR)/test-romfs.o

$(OUTDIR)/$(ROMDIR).o: $(OUTDIR)/$(ROMDIR).bin
	@mkdir -p $(dir $@)
	@echo "    OBJCOPY "$@
	@$(CROSS_COMPILE)objcopy -I binary -O elf32-littlearm -B arm \
		--prefix-sections '.romfs' $< $@

$(OUTDIR)/$(ROMDIR).bin: $(ROMDIR) $(OUTDIR)/$(TOOLDIR)/mkromfs
	@mkdir -p $(dir $@)
	@echo "    MKROMFS "$@
	@$(OUTDIR)/$(TOOLDIR)/mkromfs -d $< $@

$(ROMDIR):
	@mkdir -p $@

$(OUTDIR)/%/mkromfs: %/mkromfs.c
	@mkdir -p $(dir $@)
	@echo "    CC      "$@
	@gcc -Wall -o $@ $^
