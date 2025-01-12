//
// Created by jan-e on 05.01.2025.
//

#ifndef W32STRINGS_H
#define W32STRINGS_H

#include <string>

class w32string {
private:
  std::string utf8str{};
public:
  explicit w32string(const std::string &utf8str);
  explicit w32string(std::string &&utf8str) noexcept;
  explicit w32string(const std::wstring &ws);
  operator std::string() const ;
  operator std::wstring() const ;
};

#ifdef WIN32
#define as_wstring_on_win32(str) (w32string(str).operator std::wstring())
#define from_wstring_on_win32(str) (w32string(str).operator std::string())
#else
#define as_wstring_on_win32(str) (str)
#define from_wstring_on_win32(str) (str)
#endif

#endif //W32STRINGS_H
