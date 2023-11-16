CC = g++
CFLAGS = -Os -s -std=c++17
LDFLAGS = -lws2_32

chat: yyjson.o minipost.o terminal.o
	$(CC) chat.cpp yyjson.o minipost.o terminal.o -o chat.exe $(CFLAGS) $(LDFLAGS) -fmerge-all-constants  -fno-exceptions -fno-rtti 

clean:
	del *.o chat.exe

