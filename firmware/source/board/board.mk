# List of all the board related files.
ifeq ($(TARGET_PLATFORM),H7)
  BOARDSRC = ./source/board/board_h7.c
else
  BOARDSRC = ./source/board/board_l4.c
endif

# Required include directories
ifeq ($(TARGET_PLATFORM),H7)
  BOARDINC = ./source/board/h7
else
  BOARDINC = ./source/board/l4
endif

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
