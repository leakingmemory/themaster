//
// Created by sigsegv on 3/13/24.
//

#ifndef DRWHATSNOT_DOWNLOADFESTDIALOG_H
#define DRWHATSNOT_DOWNLOADFESTDIALOG_H

#include <wx/wx.h>

class DownloadFestDialog : public wxDialog {
private:
    wxGauge *downloadProgress;
    wxGauge *decodingProgress;
    wxGauge *serializeProgress;
public:
    DownloadFestDialog(wxWindow *parent);
    void SetDownloadProgress(int prcnt);
    void SetDecodingProgress(int prcnt);
    void SetSerializeProgress(int prcnt);
};


#endif //DRWHATSNOT_DOWNLOADFESTDIALOG_H
