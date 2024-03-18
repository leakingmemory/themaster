//
// Created by sigsegv on 3/15/24.
//

#include "FestObjectStreamUi.h"
#include <wx/wx.h>
#include "DownloadFestDialog.h"

void FestObjectStreamUi::progress(size_t count, size_t total) {
    wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([this, count, total] () {
        downloadFestDialog.SetDecodingProgress((count * 100) / total);
    });
}

void FestObjectStreamUi::progress_finished() {
    wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([this] () {
        downloadFestDialog.SetDecodingProgress(100);
    });
}
