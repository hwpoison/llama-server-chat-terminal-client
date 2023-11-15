#ifndef __TERMINAL__
#define __TERMINAL__
#include <iostream>
#include "colors.h"
#ifdef __WIN32__
 #include <windows.h>
#endif

class Terminal {
   public:
    static void setTitle(std::string titleContent);
    static void resetColor();
    static void pause();
    static void resetCursor();
    static void clear();
    static void setupEncoding();
};
#endif