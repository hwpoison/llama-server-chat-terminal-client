CC = g++
CFLAGS = -std=c++17 -s -Oz  -fvisibility=hidden -fmerge-all-constants  -fno-exceptions -fno-rtti # -static -Wall
LDFLAGS += -Lyyjson
OBJECTS := minipost.o terminal.o completion.o sjson.o utils.o ./yyjson/src/yyjson.o

ifeq ($(OS),Windows_NT)
	LDFLAGS += -lws2_32
	CLEAN_COMMAND := del /F /Q
	YYJSON_SRC_DIR=yyjson\\src\\
else
	CLEAN_COMMAND := rm -f
	YYJSON_SRC_DIR=yyjson/src/
endif

chat: $(OBJECTS)
	$(CC) chat.cpp $(OBJECTS) -o chat $(CFLAGS) $(LDFLAGS) $(DFLAGS)

clean:
	$(CLEAN_COMMAND) *.o $(YYJSON_SRC_DIR)*.o chat.exe