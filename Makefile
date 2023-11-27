CC = g++
CFLAGS = -std=c++17 -Os -s #-g3 # -Os -s # -Wall

LDFLAGS += -lws2_32 -Lyyjson -fmerge-all-constants  -fno-exceptions -fno-rtti 

chat: minipost.o terminal.o completion.o sjson.o utils.o ./yyjson/src/yyjson.o
	$(CC) chat.cpp minipost.o terminal.o completion.o sjson.o utils.o ./yyjson/src/yyjson.o -o chat $(CFLAGS) $(LDFLAGS) 

clean:
	rm *.o chat.exe

		
