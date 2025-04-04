//
// Created by sigsegv on 1/5/24.
//

#ifndef DRWHATSNOT_DATADIRECTORY_H
#define DRWHATSNOT_DATADIRECTORY_H

#include <string>

class DataDirectory {
private:
    std::string path{};
public:
    DataDirectory(const std::string &parent, const std::string &name);
#ifdef WIN32
    static DataDirectory Appdata(const std::string &name);
#else
    static DataDirectory Home(const std::string &name);
#endif
    static DataDirectory Config(const std::string &appname);
    static DataDirectory Data(const std::string &appname);
    DataDirectory Sub(const std::string &name) const;
    std::string ReadFile(const std::string &filename) const;
    void WriteFile(const std::string &filename, const std::string &content) const;
    std::string GetPath(const std::string &filename) const;
};


#endif //DRWHATSNOT_DATADIRECTORY_H
