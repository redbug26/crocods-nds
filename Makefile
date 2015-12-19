export DEVKITARM := /e/devkitPro/devkitARM
export DEVKITPRO := /e/devkitPro/

#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	$(shell basename $(CURDIR))
export TOPDIR		:=	$(CURDIR)

ICON 		:= -b $(CURDIR)/logo.bmp "CrocoDS;redbug;http://deadketchup.kyuran.be/"

#---------------------------------------------------------------------------------
# path to tools - this can be deleted if you set the path in windows
#---------------------------------------------------------------------------------
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

.PHONY: $(TARGET).arm7 $(TARGET).arm9

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: $(TARGET).ds.gba 

#dlditool mpcf.dldi $(TARGET).nds

$(TARGET).ds.gba	: $(TARGET).nds

#---------------------------------------------------------------------------------
$(TARGET).nds	:	$(TARGET).arm7 $(TARGET).arm9
	ndstool	-c $(TARGET).nds -7 $(TARGET).arm7 -9 $(TARGET).arm9 $(ICON)
	dsbuild $(TARGET).nds -o $(TARGET).ds.gba
	padbin 512 $(TARGET).nds
	cat $(TARGET).nds other/imagefcsr.img  > $(TARGET)_fs.nds
	other/dlditool.exe other/fcsr.dldi $(TARGET)_fs.nds
	
	
#---------------------------------------------------------------------------------
$(TARGET).arm7	: arm7/$(TARGET).elf
$(TARGET).arm9	: arm9/$(TARGET).elf

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	rm -f $(TARGET).ds.gba $(TARGET).nds $(TARGET).arm7 $(TARGET).arm9
