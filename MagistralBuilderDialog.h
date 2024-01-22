//
// Created by sigsegv on 1/14/24.
//

#ifndef DRWHATSNOT_MAGISTRALBUILDERDIALOG_H
#define DRWHATSNOT_MAGISTRALBUILDERDIALOG_H

#include <wx/wx.h>
#include <map>
#include "MedicalCodedValue.h"

class TheMasterFrame;
class wxListView;
class wxSpinCtrlDouble;

enum class DilutionType {
    AD, QS
};

struct Dilution {
    std::string name;
    DilutionType dilution;
};

struct Substance {
    std::string name;
    double strength;
    std::string strengthUnit;
};

struct MagistralMedicament {
    std::vector<Dilution> dilutions{};
    std::vector<Substance> substances{};
    MedicalCodedValue form{};
    double amount;
    std::string amountUnit;
    std::string instructions;
};

class MagistralBuilderDialog : public wxDialog {
private:
    std::map<std::string,std::string> strengthUnits{};
    MagistralMedicament magistralMedicament{};
    wxListView *dilutionList;
    wxComboBox *dilutionSearch;
    wxComboBox *adqsSelect;
    wxListView *substanceList;
    wxComboBox *substanceSearch;
    wxSpinCtrlDouble *substanceStrength;
    wxComboBox *substanceStrengthUnit;
    wxComboBox *medicamentFormCtrl;
    wxSpinCtrlDouble *amountCtrl;
    wxComboBox *amountUnitCtrl;
    wxTextCtrl *instructionsCtrl;
public:
    MagistralBuilderDialog(TheMasterFrame *);
    void OnAddDilution(wxCommandEvent &e);
    void OnAddSubstance(wxCommandEvent &e);
    void OnCancel(wxCommandEvent &e);
    void OnProceed(wxCommandEvent &e);
};


#endif //DRWHATSNOT_MAGISTRALBUILDERDIALOG_H
