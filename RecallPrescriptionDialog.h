//
// Created by sigsegv on 5/7/24.
//

#ifndef DRWHATSNOT_RECALLPRESCRIPTIONDIALOG_H
#define DRWHATSNOT_RECALLPRESCRIPTIONDIALOG_H

#include <wx/wx.h>
#include <memory>
#include "MedicalCodedValue.h"

class FhirMedicationStatement;
class FhirBasic;

template <class T> concept ObjectWithGetDisplay = requires (const T &obj) {
    { obj.GetDisplay() } -> std::convertible_to<std::string>;
};

class RecallPrescriptionDialog : public wxDialog {
private:
    wxComboBox *recallCode;
    wxButton *recallButton;
    MedicalCodedValue reason{};
private:
    template <ObjectWithGetDisplay T> RecallPrescriptionDialog(wxWindow *parent, const T &);
public:
    RecallPrescriptionDialog(wxWindow *parent, const std::shared_ptr<FhirMedicationStatement> &);
    RecallPrescriptionDialog(wxWindow *parent, const std::shared_ptr<FhirBasic> &);
    void OnCancel(wxCommandEvent &e);
    void OnRecall(wxCommandEvent &e);
    void OnModified(wxCommandEvent &e);
    [[nodiscard]] MedicalCodedValue GetReason() const {
        return reason;
    }
};


#endif //DRWHATSNOT_RECALLPRESCRIPTIONDIALOG_H
