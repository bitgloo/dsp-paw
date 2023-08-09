CXX = g++

CXXFILES := \
    source/serial/src/serial.cc \
    source/imgui/backends/imgui_impl_sdl2.cpp \
    source/imgui/backends/imgui_impl_opengl2.cpp \
    source/imgui/imgui.cpp \
    source/imgui/imgui_draw.cpp \
    source/imgui/imgui_tables.cpp \
    source/imgui/imgui_widgets.cpp \
    source/ImGuiFileDialog/ImGuiFileDialog.cpp \
    source/ImGuiColorTextEdit/TextEditor.cpp \
    $(wildcard source/stmdsp/*.cpp) \
    $(wildcard source/*.cpp)

CXXFLAGS := -std=c++20 -O2 \
            -Isource -Isource/imgui -Isource/stmdsp -Isource/serial/include \
            -Isource/ImGuiColorTextEdit -Isource/ImGuiFileDialog \
            $(shell sdl2-config --cflags) \
            -Wall -Wextra -pedantic #-DSTMDSP_DISABLE_FORMULAS

ifeq ($(OS),Windows_NT)
CXXFILES += source/serial/src/impl/win.cc \
            source/serial/src/impl/list_ports/list_ports_win.cc
CXXFLAGS += -DSTMDSP_WIN32 -Wa,-mbig-obj
LDFLAGS = -mwindows -lSDL2 -lopengl32 -lsetupapi -lole32
OUTPUT := stmdspgui.exe
else
CXXFILES += source/serial/src/impl/unix.cc \
            source/serial/src/impl/list_ports/list_ports_linux.cc
LDFLAGS = $(shell sdl2-config --libs) -lGL -lpthread
OUTPUT := stmdspgui
endif

OFILES := $(patsubst %.cc, %.o, $(patsubst %.cpp, %.o, $(CXXFILES)))

all: $(OUTPUT)

$(OUTPUT): $(OFILES)
	@echo "  LD    " $(OUTPUT)
	@$(CXX) $(OFILES) -o $(OUTPUT) $(LDFLAGS)

clean:
	@echo "  CLEAN"
	@rm -f $(OFILES) $(OUTPUT)

