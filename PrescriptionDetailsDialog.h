//
// Created by sigsegv on 5/2/24.
//

#ifndef DRWHATSNOT_PRESCRIPTIONDETAILSDIALOG_H
#define DRWHATSNOT_PRESCRIPTIONDETAILSDIALOG_H

#include <wx/wx.h>
#include <memory>
#include <string>
#include <vector>

class FhirMedicationStatement;
class FhirBasic;
class FhirExtension;
class FhirDosage;
class FhirIdentifier;
class wxListView;

class PrescriptionDetailsDialogEntry {
public:
    [[nodiscard]] virtual std::string GetDisplay() const = 0;
    [[nodiscard]] virtual std::vector<FhirDosage> GetDosage() const = 0;
    [[nodiscard]] virtual std::vector<std::shared_ptr<FhirExtension>> GetExtensions() const = 0;
    [[nodiscard]] virtual std::vector<FhirIdentifier> GetIdentifiers() const = 0;
    [[nodiscard]] virtual std::string GetEffectiveDateTime() const = 0;
};

class PrescriptionDetailsDialog : public wxDialog {
private:
    std::vector<std::shared_ptr<PrescriptionDetailsDialogEntry>> statements{};
    std::vector<std::shared_ptr<FhirExtension>> ereseptdosing{};
    wxListView *versionsView;
    wxButton *structuredDosing;
    wxListView *listView;
public:
    PrescriptionDetailsDialog(wxWindow *parent, const std::vector<std::shared_ptr<PrescriptionDetailsDialogEntry>> &);
    PrescriptionDetailsDialog(wxWindow *parent, const std::vector<std::shared_ptr<FhirMedicationStatement>> &);
    PrescriptionDetailsDialog(wxWindow *parent, const std::vector<std::shared_ptr<FhirBasic>> &);
private:
    void AddVersion(int row, const std::shared_ptr<PrescriptionDetailsDialogEntry> &);
    void DisplayStatement(const std::shared_ptr<PrescriptionDetailsDialogEntry> &);
public:
    void OnVersionSelect(wxCommandEvent &e);
    void OnStructuredDosing(wxCommandEvent &e);
};


#endif //DRWHATSNOT_PRESCRIPTIONDETAILSDIALOG_H
