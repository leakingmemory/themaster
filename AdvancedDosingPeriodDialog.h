//
// Created by sigsegv on 10/15/24.
//

#ifndef THEMASTER_ADVANCEDDOSINGPERIODDIALOG_H
#define THEMASTER_ADVANCEDDOSINGPERIODDIALOG_H

#include <wx/wx.h>
#include <memory>

class AdvancedDosingPeriod;

class AdvancedDosingPeriodDialog : public wxDialog {
private:
    std::shared_ptr<AdvancedDosingPeriod> dosingPeriod;
    wxSpinCtrlDouble *morgen;
    wxSpinCtrlDouble *formiddag;
    wxSpinCtrlDouble *middag;
    wxSpinCtrlDouble *ettermiddag;
    wxSpinCtrlDouble *kveld;
    wxSpinCtrlDouble *natt;
    wxSpinCtrl *duration;
    wxButton *ok;
public:
    AdvancedDosingPeriodDialog(wxWindow *parent);
    void OnOk(wxCommandEvent &e);
    std::shared_ptr<AdvancedDosingPeriod> GetDosingPeriod() const {
        return dosingPeriod;
    }
};


#endif //THEMASTER_ADVANCEDDOSINGPERIODDIALOG_H
