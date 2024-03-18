//
// Created by sigsegv on 3/14/24.
//

#include "FestSerializerUi.h"
#include "DownloadFestDialog.h"

void FestSerializerUi::Progress(int done, int total) {
    wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([this, done, total] () {
        downloadFestDialog.SetSerializeProgress((done * 100) / total);
    });
}

void FestSerializerUi::ProgressFinished(bool success) {
    if (success) {
        wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([this] () {
            downloadFestDialog.SetSerializeProgress(100);
        });
    }
}