//
// Created by sigsegv on 10/18/24.
//

#ifndef THEMASTER_ERESEPTDOSINGDIALOG_H
#define THEMASTER_ERESEPTDOSINGDIALOG_H

#include <wx/wx.h>
#include <vector>
#include <memory>

class FhirExtension;
class wxListView;

class EreseptdosingDialog : public wxDialog {
private:
    std::vector<std::shared_ptr<FhirExtension>> ereseptdosing;
    wxListView *info;
    wxListView *dpInfo;
public:
    EreseptdosingDialog(wxWindow *parent, const std::vector<std::shared_ptr<FhirExtension>> &ereseptdosing);
    void DisplayEreseptdosing(const std::shared_ptr<FhirExtension> &);
    void OnDosingPeriodSelected(wxCommandEvent &e);
};


#endif //THEMASTER_ERESEPTDOSINGDIALOG_H
