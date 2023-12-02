CC = g++
CFLAGS = -std=c++17 -s -Os #-g #-Os -s

LDFLAGS += -lws2_32 -Lyyjson

chat: minipost.o terminal.o completion.o sjson.o utils.o ./yyjson/src/yyjson.o
	$(CC) chat.cpp minipost.o terminal.o completion.o sjson.o utils.o ./yyjson/src/yyjson.o -o chat $(CFLAGS) $(LDFLAGS) -fmerge-all-constants  -fno-exceptions -fno-rtti  

clean:
	del *.o chat.exe