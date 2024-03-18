//
// Created by sigsegv on 3/11/24.
//

#ifndef DRWHATSNOT_FESTDBUI_H
#define DRWHATSNOT_FESTDBUI_H

#include <string>

class wxWindow;
class DownloadFestDialog;

class FestDbUi {
private:
    wxWindow *parent;
public:
    FestDbUi(wxWindow *parent) : parent(parent) {}
    void UpdateFromFile(DownloadFestDialog &dialog, const std::string &filename);
    void Update();
};


#endif //DRWHATSNOT_FESTDBUI_H
