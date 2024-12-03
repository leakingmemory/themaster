//
// Created by sigsegv on 11/18/24.
//

#ifndef THEMASTER_REGISTERCAVEDIALOG_H
#define THEMASTER_REGISTERCAVEDIALOG_H

#include <wx/wx.h>
#include <memory>
#include <sfmbasisapi/fhir/value.h>

class FestDb;
class LegemiddelCore;
class FhirAllergyIntolerance;
class wxListView;
class wxListEvent;

struct CaveCoding {
    FhirCoding coding;
    std::string display;
    std::string type;

    CaveCoding(const FhirCoding &coding, const std::string &display, const std::string &type) : coding(coding), display(display), type(type) {}
    CaveCoding(FhirCoding &&coding, std::string &&display, std::string &&type) noexcept : coding(std::move(coding)), display(std::move(display)), type(std::move(type)) {}
    CaveCoding(const CaveCoding &) = default;
    CaveCoding(CaveCoding &&mv) : coding(std::move(mv.coding)), display(std::move(mv.display)), type(std::move(mv.type)) {}
    CaveCoding &operator = (const CaveCoding &) = default;
    CaveCoding &operator = (CaveCoding &&cp) noexcept {
        coding = std::move(cp.coding);
        display = std::move(cp.display);
        type = std::move(cp.type);
        return *this;
    }
};

class RegisterCaveDialog : public wxDialog {
private:
    std::vector<CaveCoding> availableCodings{};
    std::string id;
    std::string recordedDate;
    FhirReference recorder;
    FhirReference patient;
    wxListView *codingsListView;
    wxCheckBox *inactiveIngredient;
    wxComboBox *sourceOfInformation;
    wxComboBox *typeOfReaction;
    wxComboBox *verificationStatus;
    wxButton *addButton;
public:
    RegisterCaveDialog(wxWindow *parent, const std::vector<CaveCoding> &availableCodings, const FhirReference &recorder, const FhirReference &patient);
    RegisterCaveDialog(wxWindow *parent, const std::shared_ptr<FestDb> &festDb, const LegemiddelCore &, const FhirReference &recorder, const FhirReference &patient);
    RegisterCaveDialog(wxWindow *parent, const FhirAllergyIntolerance &);
    bool IsValid() const;
    void Revalidate();
    void RevalidateCommand(const wxCommandEvent &e);
    void ListUpdate(const wxListEvent &e);
    std::shared_ptr<FhirAllergyIntolerance> ToFhir() const;
};


#endif //THEMASTER_REGISTERCAVEDIALOG_H
