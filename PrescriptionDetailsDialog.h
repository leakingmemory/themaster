//
// Created by sigsegv on 5/2/24.
//

#ifndef DRWHATSNOT_PRESCRIPTIONDETAILSDIALOG_H
#define DRWHATSNOT_PRESCRIPTIONDETAILSDIALOG_H

#include <wx/wx.h>
#include <memory>

class FhirMedicationStatement;

class PrescriptionDetailsDialog : public wxDialog {
public:
    PrescriptionDetailsDialog(wxWindow *parent, const std::shared_ptr<FhirMedicationStatement> &);
};


#endif //DRWHATSNOT_PRESCRIPTIONDETAILSDIALOG_H
