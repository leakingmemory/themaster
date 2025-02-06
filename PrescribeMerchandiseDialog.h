//
// Created by sigsegv on 1/17/25.
//

#ifndef THEMASTER_PRESCRIBEMERCHANDISEDIALOG_H
#define THEMASTER_PRESCRIBEMERCHANDISEDIALOG_H

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <map>
#include "MedicalCodedValue.h"
#include "DateOnly.h"
#include "MerchData.h"

class MerchTree;
class wxDatePickerCtrl;
class wxDateEvent;

class PrescribeMerchandiseDialog : public wxDialog {
private:
    std::map<wxTreeItemId,MerchRefundInfo> refundInfoMap{};
    MerchRefundInfo refundInfo{};
    wxTextCtrl *paragraph;
    wxTextCtrl *productGroup;
    wxTreeCtrl *treeView;
    wxTextCtrl *dssn;
    wxDatePickerCtrl *startDate{};
    wxDatePickerCtrl *expirationDate{};
    wxButton *okButton;
    wxButton *cancelButton;
public:
    PrescribeMerchandiseDialog(wxWindow *parent, const MerchTree &);
private:
    void OnSelectedRefundTreeItem(wxCommandEvent &);
    void OnDssnChanged(wxCommandEvent &e);
    void OnDateChanged(wxDateEvent &e);
public:
    [[nodiscard]] MerchData GetMerchData() const;
private:
    void RefreshButons();
    static bool IsValid(const MerchData &);
};


#endif //THEMASTER_PRESCRIBEMERCHANDISEDIALOG_H
