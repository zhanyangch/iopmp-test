ifndef CROSS_COMPILE
$(error You have to specify 'CROSS_COMPILE' to build ! E.g. export CROSS_COMPILE=/opt/riscv/bin/riscv64-unknown-elf- )
endif

CC	:= $(CROSS_COMPILE)gcc
C++	:= $(CROSS_COMPILE)g++
OBJDUMP	:= $(CROSS_COMPILE)objdump
OBJCOPY := $(CROSS_COMPILE)objcopy
AR	:= $(CROSS_COMPILE)ar
AS	:= $(CROSS_COMPILE)as
LD	:= $(CC)

ifeq ($(DEBUG),1)
	OPTIM   := -O0 -g3
else
	OPTIM   := -Os -g3
endif

PLAT	= VIRT

MTYPE	= DDR

PLATNAME = $(shell echo $(PLAT) | tr A-Z a-z)

# Define the directories
ROOT			= ..
SRC_PATH		= ../src
BSP_SOURCE_DIR		= $(SRC_PATH)/bsp
UTILITY_DIR		= $(SRC_PATH)/utility
DEMO_SOURCE_DIR         = $(SRC_PATH)/demo

BSP_SRCS = \
	$(BSP_SOURCE_DIR)/$(PLATNAME)/trap_handler.c \
	$(BSP_SOURCE_DIR)/$(PLATNAME)/init.S \
	$(BSP_SOURCE_DIR)/$(PLATNAME)/interrupt.c \
	$(BSP_SOURCE_DIR)/$(PLATNAME)/dma.c

BSP_IMSIC_SRCS = \
	$(BSP_SOURCE_DIR)/$(PLATNAME)/trap_handler_imsic.c \
	$(BSP_SOURCE_DIR)/$(PLATNAME)/init.S \
	$(BSP_SOURCE_DIR)/$(PLATNAME)/interrupt_imsic.c \
	$(BSP_SOURCE_DIR)/$(PLATNAME)/dma.c

UTILITY_SRCS = \
	$(UTILITY_DIR)/8250_uart.c \
	$(UTILITY_DIR)/riscv_iopmp.c \
	$(UTILITY_DIR)/printf.c


SRCS = $(BSP_SRCS) $(UTILITY_SRCS)
OBJS = $(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,${SRCS})))
IMSIC-SRCS = $(BSP_IMSIC_SRCS) $(UTILITY_SRCS)
IMSIC-OBJS = $(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,${IMSIC-SRCS})))

ifeq ($(V),1)
TRACE_CC  =
TRACE_C++ =
TRACE_LD  =
TRACE_AR  =
TRACE_AS  =
Q=
else
TRACE_CC  = @echo "  CC       " $<
TRACE_C++ = @echo "  C++      " $<
TRACE_LD  = @echo "  LD       " $@
TRACE_AR  = @echo "  AR       " $@
TRACE_AS  = @echo "  AS       " $<
Q=@
endif

ifeq ($(PLAT), VIRT)
RAM_START= 0x80000000
else
RAM_START= 0x0
endif

# Common compilation options

CFLAGS = $(OPTIM) -I$(BSP_SOURCE_DIR)/$(PLATNAME) -I$(UTILITY_DIR) -mcmodel=medany -fno-builtin-printf

ASFLAGS = -D__ASSEMBLY__ $(CFLAGS) -c

LDFLAGS = -Wl,--section-start=.init=$(RAM_START) -Wl,--entry=___init

LIBS	=

# Compilation rules
.SUFFIXES : %.o %.c %.S

%.o: %.c
	$(TRACE_CC)
	$(Q)$(CC) -c -MMD $(CFLAGS) -o $@ $<

%.o: %.S
	$(TRACE_CC)
	$(Q)$(CC) -c -MMD $(ASFLAGS) -o $@ $<

all: Test1 Test2 Test3 Test4 Test5 Test6 Test7 Test8 Test9 Test10 Test11 Test12 Test13 Test14 Test15 Test16 Test17

Test1  : $(DEMO_SOURCE_DIR)/Test1.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test2  : $(DEMO_SOURCE_DIR)/Test2.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test3 : $(DEMO_SOURCE_DIR)/Test3.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test4 : $(DEMO_SOURCE_DIR)/Test4.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test5 : $(DEMO_SOURCE_DIR)/Test5.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test6 : $(DEMO_SOURCE_DIR)/Test6.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test7 : $(DEMO_SOURCE_DIR)/Test7.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test8 : $(DEMO_SOURCE_DIR)/Test8.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test9 : $(DEMO_SOURCE_DIR)/Test9.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test10 : $(DEMO_SOURCE_DIR)/Test10.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test11 : $(DEMO_SOURCE_DIR)/Test11.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test12 : $(DEMO_SOURCE_DIR)/Test12.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test13 : $(DEMO_SOURCE_DIR)/Test13.o $(IMSIC-OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(IMSIC-OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test14 : $(DEMO_SOURCE_DIR)/Test14.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test15 : $(DEMO_SOURCE_DIR)/Test15.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test16 : $(DEMO_SOURCE_DIR)/Test16.o $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

Test17 : $(DEMO_SOURCE_DIR)/Test17.o $(IMSIC-OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(LD) -o $@.elf $(LDFLAGS) $(OBJS) $(LIBS) $<
	$(OBJCOPY) -O binary -S $@.elf $@.bin
	$(OBJDUMP) -d $@.elf > $@.asm
	@echo Completed

clean :
	@rm -f $(OBJS) $(OBJS:.o=.d) $(OBJS:.o=.gcda) $(OBJS:.o=.gcno)
	@rm -f $(IMSIC-OBJS) $(IMSIC-OBJS:.o=.d) $(IMSIC-OBJS:.o=.gcda) $(IMSIC-OBJS:.o=.gcno)
	@rm -f $(DEMO_SOURCE_DIR)/*.o $(DEMO_SOURCE_DIR)/*.d
	@rm -f *.elf
	@rm -f *.bin
	@rm -f *.asm

# Automatic dependency generation
ifneq ($(MAKECMDGOALS),clean)
-include $(OBJS:.o=.d)
endif
