CC = g++
CFLAGS = -std=c++17 -Os -s # -Wall

LDFLAGS += -lws2_32 -Lyyjson

chat: minipost.o terminal.o ./yyjson/src/yyjson.o
	$(CC) chat.cpp minipost.o terminal.o ./yyjson/src/yyjson.o -o chat $(CFLAGS) $(LDFLAGS) -fmerge-all-constants  -fno-exceptions -fno-rtti 

clean:
	rm *.o chat.exe

		
