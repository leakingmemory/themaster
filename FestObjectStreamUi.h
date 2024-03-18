//
// Created by sigsegv on 3/15/24.
//

#ifndef DRWHATSNOT_FESTOBJECTSTREAMUI_H
#define DRWHATSNOT_FESTOBJECTSTREAMUI_H

#include <medfest/FestObjectStream.h>

class DownloadFestDialog;

class FestObjectStreamUi : public FestObjectStream {
private:
    DownloadFestDialog &downloadFestDialog;
public:
    template <FestSourceStream Source> FestObjectStreamUi(DownloadFestDialog &downloadFestDialog, std::shared_ptr<Source> source) : FestObjectStream(source), downloadFestDialog(downloadFestDialog) {
    }
    void progress(size_t count, size_t total) override;
    void progress_finished() override;
};


#endif //DRWHATSNOT_FESTOBJECTSTREAMUI_H
