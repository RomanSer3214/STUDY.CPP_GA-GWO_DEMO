#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1, Mac OS X and MSVC (Visual Studio)
#
# You will need GLFW (http://www.glfw.org):
# Linux:
#   apt-get install libglfw-dev
# Mac OS X:
#   brew install glfw
# MSYS2:
#   pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw
# Windows (MSVC):
#   Download GLFW precompiled binaries or build from source
#

#CXX = g++
#CXX = clang++

EXE = Demo_App
IMGUI_DIR = ../
SOURCES = main.cpp
SOURCES += $(IMGUI_DIR)/imgui/imgui.cpp $(IMGUI_DIR)/imgui/imgui_demo.cpp $(IMGUI_DIR)/imgui/imgui_draw.cpp $(IMGUI_DIR)/imgui/imgui_tables.cpp $(IMGUI_DIR)/imgui/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_DIR)/src/GA.cpp $(IMGUI_DIR)/src/GWO.cpp $(IMGUI_DIR)/src/DrawScene.cpp 
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)

CXXFLAGS = -std=c++20 -I$(IMGUI_DIR)/imgui -I$(IMGUI_DIR)/backends
CXXFLAGS += -g -Wall -Wformat
LIBS =

##---------------------------------------------------------------------
## OPENGL ES
##---------------------------------------------------------------------

## This assumes a GL ES library available in the system, e.g. libGLESv2.so
# CXXFLAGS += -DIMGUI_IMPL_OPENGL_ES2
# LINUX_GL_LIBS = -lGLESv2

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LINUX_GL_LIBS = -lGL
	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3`
	CXXFLAGS += `pkg-config --cflags glfw3`
endif

ifeq ($(UNAME_S), Darwin) #MAC OS
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -lglfw

	CXXFLAGS += -I/usr/local/include
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lglfw3 -lgdi32 -lopengl32 -limm32
	CXXFLAGS += -DIMGUI_IMPL_OPENGL_LOADER_GL3W
endif

ifeq ($(OS), Windows_NT)
    ifeq ($(shell uname -s),) # MSVC
		ECHO_MESSAGE = "Windows MSVC"
		CXX = cl
		LD = link
		EXE = test_app.exe
		OBJS = $(addsuffix .obj, $(basename $(notdir $(SOURCES))))
		
		# MSVC flags
		CXXFLAGS = -std:c++20 -I$(IMGUI_DIR)/imgui -I$(IMGUI_DIR)/backends
		CXXFLAGS += -Zi -W4 -EHsc
		CXXFLAGS += -D_CRT_SECURE_NO_WARNINGS
		
		# GLFW paths - adjust these to match your GLFW installation
		GLFW_INCLUDE = -Ipath_to_glfw_include
		GLFW_LIB = -LIBPATH:path_to_glfw_lib glfw3.lib
		
		LIBS = $(GLFW_LIB) opengl32.lib gdi32.lib
		
		# MSVC specific build rules
		%.obj:%.cpp
			$(CXX) $(CXXFLAGS) $(GLFW_INCLUDE) -c -Fo$@ $<
		
		%.obj:$(IMGUI_DIR)/imgui/%.cpp
			$(CXX) $(CXXFLAGS) $(GLFW_INCLUDE) -c -Fo$@ $<
		
		%.obj:$(IMGUI_DIR)/backends/%.cpp
			$(CXX) $(CXXFLAGS) $(GLFW_INCLUDE) -c -Fo$@ $<
		
		%.obj:$(IMGUI_DIR)/src/%.cpp
			$(CXX) $(CXXFLAGS) $(GLFW_INCLUDE) -c -Fo$@ $<
    endif
endif

##---------------------------------------------------------------------
## BUILD RULES (for non-MSVC platforms)
##---------------------------------------------------------------------

ifneq ($(OS), Windows_NT)
%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/imgui/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
endif

##---------------------------------------------------------------------
## LINK RULES
##---------------------------------------------------------------------

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

ifeq ($(OS), Windows_NT)
    ifeq ($(shell uname -s),) # MSVC
$(EXE): $(OBJS)
	$(LD) -out:$@ $** $(LIBS)
    else
$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)
    endif
else
$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)
endif

clean:
	rm -f $(EXE) $(OBJS)
