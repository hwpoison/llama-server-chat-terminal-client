#include "terminal.hpp"

void Terminal::setTitle(std::string_view titleContent) {
#ifdef __WIN32__
    SetConsoleTitleA(titleContent.data());
#endif
}

void Terminal::resetColor() { 
  std::cout << ANSI_COLOR_RESET; 
}

void Terminal::pause() {
    std::cout << "Press a key to continue...";
    std::cin.get();
}

void Terminal::resetCursor() {
#ifdef __WIN32__
    COORD cursorPosition = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
#endif
}

void Terminal::clear() {
#ifdef __WIN32__
    std::system("cls");
#else
    std::system("clear");
#endif
}

void Terminal::setupEncoding() {
#ifdef __WIN32__
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}
