#include "terminal.hpp"

void Terminal::setTitle(std::string titleContent) {
  std::system(std::string("title " + titleContent).c_str());
}

void Terminal::resetColor() { std::cout << ANSI_COLOR_RESET; }

void Terminal::pause() {
#ifdef __WIN32__
  std::system("pause");
#endif
}

void Terminal::resetCursor() {
#ifdef __WIN32__
  COORD cursorPosition;
  cursorPosition.X = 0;
  cursorPosition.Y = 0;
  SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
#endif
}

void Terminal::clear() {
#ifdef __WIN32__
  std::system("cls");
  // resetCursor();
#else
  system("clear");
#endif
}

void Terminal::setupEncoding() {
// Set utf-8 support for windows
#ifdef __WIN32__
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif
}
