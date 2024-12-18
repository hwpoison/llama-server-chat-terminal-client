#ifndef __TERMINAL__
#define __TERMINAL__
#include <iostream>

#include "colors.h"
#ifdef __WIN32__
    #include <windows.h>
#endif

class Terminal {
public:
    static void setTitle(std::string_view titleContent);
    static void resetColor();
    static void resetCursor();
    static void resetAll();
    static void pause();
    static void clear();
    static void setupEncoding();
};
#endif