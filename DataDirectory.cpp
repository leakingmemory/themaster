//
// Created by sigsegv on 1/5/24.
//

#include "DataDirectory.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>

DataDirectory::DataDirectory(const std::string &parent, const std::string &name) : path(parent) {
    if (!path.ends_with("/")) {
        path.append("/");
    }
    path.append(name);
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
    }
}

DataDirectory DataDirectory::Home(const std::string &name) {
    const char *home = getenv("HOME");
    if (home == nullptr) {
        std::cerr << "No HOME environment variable\n";
        throw std::exception();
    }
    return {home, name};
}

DataDirectory DataDirectory::Config(const std::string &appname) {
    return Home(".config").Sub(appname);
}

DataDirectory DataDirectory::Data(const std::string &appname) {
    return Home(".local").Sub("share").Sub(appname);
}

DataDirectory DataDirectory::Sub(const std::string &name) const {
    return {path, name};
}

std::string DataDirectory::ReadFile(const std::string &filename) const {
    std::string fpath{path};
    if (!fpath.ends_with("/")) {
        fpath.append("/");
    }
    fpath.append(filename);
    if (!std::filesystem::exists(fpath)) {
        return {};
    }
    std::ifstream t(fpath);
    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)),
               std::istreambuf_iterator<char>());

    t.close();

    return str;
}

void DataDirectory::WriteFile(const std::string &filename, const std::string &content) const {
    std::string fpath{path};
    if (!fpath.ends_with("/")) {
        fpath.append("/");
    }
    fpath.append(filename);
    std::string tmpfilename{};
    {
        std::stringstream sstr{};
        sstr << fpath << ".";
        sstr << getpid();
        tmpfilename = sstr.str();
    }
    std::ofstream stream{tmpfilename};
    stream << content;
    stream.close();

    std::filesystem::rename(tmpfilename, fpath);
}

std::string DataDirectory::GetPath(const std::string &filename) const {
    std::string fpath{path};
    if (!fpath.ends_with("/")) {
        fpath.append("/");
    }
    fpath.append(filename);
    return fpath;
}
