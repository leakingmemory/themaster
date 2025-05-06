//
// Created by sigsegv on 1/14/24.
//

#ifndef DRWHATSNOT_MAGISTRALBUILDERDIALOG_H
#define DRWHATSNOT_MAGISTRALBUILDERDIALOG_H

#include <wx/wx.h>
#include <map>
#include <memory>
#include "MedicalCodedValue.h"
#include "MagistralMedicament.h"

class TheMasterFrame;
class wxListView;
class wxSpinCtrlDouble;
class ComboSearchControl;
class FestDb;

class MagistralBuilderDialog : public wxDialog {
private:
    std::map<std::string,std::string> strengthUnits{};
    MagistralMedicament magistralMedicament{};
    wxListView *dilutionList;
    ComboSearchControl *dilutionSearch;
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
    MagistralBuilderDialog(TheMasterFrame *, const std::shared_ptr<FestDb> &);
    void OnAddDilution(wxCommandEvent &e);
    void OnAddSubstance(wxCommandEvent &e);
    void OnCancel(wxCommandEvent &e);
    void OnProceed(wxCommandEvent &e);
    [[nodiscard]] MagistralMedicament GetMagistralMedicament() const {
        return magistralMedicament;
    }
};


#endif //DRWHATSNOT_MAGISTRALBUILDERDIALOG_H
