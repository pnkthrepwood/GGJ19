EXEC	= haxoria

SRC_DIR = .
IMGUI_DIR = imgui

SRC	  = $(wildcard $(SRC_DIR)/*.cpp)
IMGUI_SRC = $(wildcard $(IMGUI_DIR)/*.cpp)

OBJ	  = $(patsubst $(SRC_DIR)/%, obj/%.o, $(SRC)) $(patsubst $(IMGUI_DIR)/%, obj/imgui/%.o, $(IMGUI_SRC))

OPTIM	= 0 #(0 to 3)
DEBUG	= 1 #(0 or 1)
PROFILE	= 0 #(0 or 1)

#Bash needed for curly braces expansion
SHELL = bash
CXX	= g++

CURRENT_DIR = $(shell pwd)

LIBS = -lsfml-window -lsfml-system -lsfml-graphics -lsfml-audio -lsfml-network -lGL
INCLUDES = -I $(CURRENT_DIR)/imgui

CFLAGS	= -std=c++1y -pipe -Wall -Wno-unknown-pragmas -Wno-unused-parameter $(PROFILEFLAGS) $(DEBUGFLAGS) -O$(strip $(OPTIM)) $(INCLUDES)
LDFLAGS	= -pipe $(LIBS) $(PROFILEFLAGS) $(DEBUGFLAGS) -O$(strip $(OPTIM))

ifeq ($(strip $(PROFILE)),1)
	PROFILEFLAGS=-pg
endif
ifeq ($(strip $(DEBUG)),1)
	DEBUGFLAGS=-D_DEBUG -ggdb3
endif

$(EXEC): directories $(OBJ)
	echo $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o $(EXEC)

obj/Haxoria.cpp.o: $(SRC_DIR)/Haxoria.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

obj/imgui/%.cpp.o: $(IMGUI_DIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

obj/%.cpp.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ) $(EXEC)

world:
	make clean && make

directories:
	mkdir -p obj
	mkdir -p obj/imgui
