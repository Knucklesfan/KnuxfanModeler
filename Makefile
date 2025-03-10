OBJS = main.o
HEADER	= 
OUT	= KMFModeler
CC	 = g++
FLAGS	 = $(INC) -g -c -Wall -O0 -D _LINUX -DCLIENT
INC=-I./include/
LFLAGS += -lassimp -L./lib/
all: $(OBJS)
	$(CC) -g $(OBJS)  -o $(OUT) $(LFLAGS)
main.o: source/main.cpp
	$(CC) $(FLAGS) source/main.cpp -std=c++17
