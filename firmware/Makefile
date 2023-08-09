##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Set the target platform, either L4, G4, or H7
TARGET_PLATFORM = L4

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O0 -g3 -ggdb -fomit-frame-pointer -falign-functions=16 --specs=nosys.specs
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -std=c++2a -fno-rtti -fno-exceptions
endif

# Enable this if you want the linker to remove unused code and data.
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
#  USE_LDOPT = -L.,-lzig
endif

# Enable this if you want link time optimizations (LTO).
ifeq ($(USE_LTO),)
  USE_LTO = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = yes
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 1024
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 2048
endif

# Enables the use of FPU (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = hard
endif

# FPU-related options.
ifeq ($(USE_FPU_OPT),)
ifeq ($(TARGET_PLATFORM),H7)
  USE_FPU_OPT = -mfloat-abi=$(USE_FPU) -mfpu=fpv5-d16
endif
ifeq ($(TARGET_PLATFORM),L4)
  USE_FPU_OPT = -mfloat-abi=$(USE_FPU) -mfpu=fpv4-sp-d16
endif
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, target, sources and paths
#

# Define project name here
PROJECT = ch

# Target settings.
ifeq ($(TARGET_PLATFORM),H7)
  MCU = cortex-m7
else
  MCU = cortex-m4
endif

# Imported source files and paths.
CHIBIOS  := ./ChibiOS
CONFDIR  := ./source/cfg
BUILDDIR := ./build
DEPDIR   := ./.dep

# Licensing files.
include $(CHIBIOS)/os/license/license.mk
# Startup files.
ifeq ($(TARGET_PLATFORM),H7)
  include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32h7xx.mk
else
  include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32l4xx.mk
endif
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
ifeq ($(TARGET_PLATFORM),H7)
  include $(CHIBIOS)/os/hal/ports/STM32/STM32H7xx/platform.mk
else
  include $(CHIBIOS)/os/hal/ports/STM32/STM32L4xx/platform.mk
endif
include ./source/board/board.mk
include $(CHIBIOS)/os/hal/osal/rt-nil/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/mk/port_v7m.mk
# Auto-build files in ./source recursively.
#include $(CHIBIOS)/tools/mk/autobuild.mk
ALLCSRC += $(wildcard source/*.c) $(wildcard source/periph/*.c)
ALLCPPSRC += $(wildcard source/*.cpp) $(wildcard source/periph/*.cpp)
ALLASMSRC += $(wildcard source/*.s)
# Other files (optional).
#include $(CHIBIOS)/test/lib/test.mk
#include $(CHIBIOS)/test/rt/rt_test.mk
#include $(CHIBIOS)/test/oslib/oslib_test.mk

# Define linker script file here
ifeq ($(TARGET_PLATFORM),H7)
  LDSCRIPT = source/ld/STM32H723xG.ld
else
  LDSCRIPT = source/ld/STM32L476xG.ld
endif

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(ALLCSRC)

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC = $(ALLCPPSRC)

# List ASM source files here.
ASMSRC = $(ALLASMSRC)

# List ASM with preprocessor source files here.
ASMXSRC = $(ALLXASMSRC)

# Inclusion directories.
INCDIR = $(CONFDIR) $(ALLINC) $(TESTINC) \
         source source/periph

# Define C warning options here.
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes -pedantic

# Define C++ warning options here.
CPPWARN = -Wall -Wextra -Wundef -pedantic -Wno-volatile

#
# Project, target, sources and paths
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS = -DCORTEX_ENABLE_WFI_IDLE=TRUE \
        -DPORT_USE_SYSCALL=TRUE \
        -DPORT_USE_GUARD_MPU_REGION=MPU_REGION_0 \
        -DTARGET_PLATFORM_$(TARGET_PLATFORM)

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ifeq ($(TARGET_PLATFORM),L4)
  ULIBS = -lm
else
  ULIBS =
endif

#
# End of user section
##############################################################################

##############################################################################
# Common rules
#

RULESPATH = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk
include $(RULESPATH)/arm-none-eabi.mk
include $(RULESPATH)/rules.mk

#
# Common rules
##############################################################################

##############################################################################
# Custom rules
#

#
# Custom rules
##############################################################################
