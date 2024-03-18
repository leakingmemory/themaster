//
// Created by sigsegv on 3/14/24.
//

#ifndef DRWHATSNOT_FESTSERIALIZERUI_H
#define DRWHATSNOT_FESTSERIALIZERUI_H

#include <medfest/FestSerializer.h>

class DownloadFestDialog;

class FestSerializerUi : public FestSerializer {
private:
    DownloadFestDialog &downloadFestDialog;
public:
    FestSerializerUi(DownloadFestDialog &downloadFestDialog, const std::string &filename) : FestSerializer(filename),
    downloadFestDialog(downloadFestDialog) {}
    void Progress(int done, int total) override;
    void ProgressFinished(bool success) override;
};


#endif //DRWHATSNOT_FESTSERIALIZERUI_H
