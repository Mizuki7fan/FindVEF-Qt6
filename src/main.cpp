#include <QApplication>
#include <QFont>
#include <QStyleFactory>
#include <format>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "ui/windows/MainWindow.h"

// #define STRINGIFY(x) #x
// #define TOSTRING(x) STRINGIFY(x)
void printBuildTimeInfo() {
  // // 打印项目build以及exe生成的时间
  // std::cout << "This project was build on: " << TOSTRING(COMPILE_TIME)
  //           << std::endl;

  std::tm t = {};
  std::istringstream iss(__DATE__);
  if (iss >> std::get_time(&t, "%b %d %Y")) {
    iss.clear();
    iss.str(std::string());
    iss.str(__TIME__);
    if (iss >> std::get_time(&t, "%T")) {
      std::cout << std::format(
                       "The code was last modified on: \"{}-{}-{} {}:{}:{}\"",
                       1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday, t.tm_hour,
                       t.tm_min, t.tm_sec)
                << std::endl;
    } else {
      std::cerr << "Failed to parse time\n";
    }
  } else {
    std::cerr << "Failed to parse date\n";
  }
}

int main(int argc, char *argv[]) {
  // 要求命令行输出的文字编码为utf-8
  SetConsoleOutputCP(CP_UTF8);
  printBuildTimeInfo();
  QApplication app(argc, argv);
  QFont globalFont("Consolas", 12, QFont::Medium);
  app.setFont(globalFont);
  app.setStyle(QStyleFactory::create("windowsvista"));
  MainWindow w;
  //  w.ResetWindowSize();  // 重置窗口布局
  w.show();
  return app.exec();
}