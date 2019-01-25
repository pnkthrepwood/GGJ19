EXEC	= slavery_resort

SRC_DIR = GGJ2k18

SRC	= $(wildcard $(SRC_DIR)/*.cpp)
OBJ	= $(patsubst $(SRC_DIR)/%, obj/%.o, $(SRC))

OPTIM	= 0 #(0 - 3)
DEBUG	= 1 #(0 or 1)
PROFILE	= 0 #(0 or 1)

#Bash needed for curly braces expansion
SHELL = bash
CXX	= g++

LIBS = -lsfml-window -lsfml-system -lsfml-graphics -lsfml-audio -lGL
INCLUDES = -I

CFLAGS	= -std=c++1y -pipe -Wall -Wno-unknown-pragmas -Wno-unused-parameter $(PROFILEFLAGS) $(DEBUGFLAGS) -O$(strip $(OPTIM))
LDFLAGS	= -pipe $(LIBS) $(PROFILEFLAGS) $(DEBUGFLAGS) -O$(strip $(OPTIM))

ifeq ($(strip $(PROFILE)),1)
	PROFILEFLAGS=-pg
endif
ifeq ($(strip $(DEBUG)),1)
	DEBUGFLAGS=-D_DEBUG -ggdb3
endif

$(EXEC): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o $(EXEC)

obj/main.cpp.o: $(SRC_DIR)/main.cpp $(SRC_DIR)/animation.h $(SRC_DIR)/gaem_constants.h $(SRC_DIR)/minion.h
	$(CXX) $(CFLAGS) -c $< -o $@

obj/%.cpp.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ) $(EXEC)

world:
	make clean && make

