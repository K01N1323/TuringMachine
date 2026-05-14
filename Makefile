CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iimgui -Iimgui/backends -I/Users/nikolaj/TuringMachineMainRepo/vendor/GLFW/glfw-3.4.bin.MACOS/include

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S), Darwin)
	GLFW_PATH = /Users/nikolaj/TuringMachineMainRepo/vendor/GLFW/glfw-3.4.bin.MACOS
	ifeq ($(UNAME_M), arm64)
		LIBS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo $(GLFW_PATH)/lib-arm64/libglfw3.a
	else
		LIBS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo $(GLFW_PATH)/lib-x86_64/libglfw3.a
	endif
else
	LIBS = -lGL -lglfw
endif

IMGUI_SRCS = imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp
IMGUI_OBJS = $(IMGUI_SRCS:.cpp=.o)

CORE_SRCS = main.cpp MemoryManager.cpp TuringMachine.cpp Compiler.cpp MacrosForTuring.cpp
CORE_OBJS = $(CORE_SRCS:.cpp=.o)

GUI_SRCS = GUI.cpp
GUI_OBJS = $(GUI_SRCS:.cpp=.o)

TARGET = turing_machine

all: $(TARGET)

$(TARGET): $(CORE_OBJS) $(GUI_OBJS) $(IMGUI_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(CORE_OBJS) $(GUI_OBJS) $(IMGUI_OBJS) $(LIBS)

headless: CXXFLAGS += -DHEADLESS
headless: $(CORE_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(CORE_OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(CORE_OBJS) $(GUI_OBJS) $(IMGUI_OBJS) $(TARGET)

.PHONY: all headless clean
