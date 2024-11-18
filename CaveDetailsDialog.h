//
// Created by sigsegv on 11/14/24.
//

#ifndef THEMASTER_CAVEDETAILSDIALOG_H
#define THEMASTER_CAVEDETAILSDIALOG_H

#include <wx/wx.h>
#include <memory>

class FhirAllergyIntolerance;

class CaveDetailsDialog : public wxDialog {
public:
    CaveDetailsDialog(wxWindow *parent, const std::shared_ptr<FhirAllergyIntolerance> &);
};


#endif //THEMASTER_CAVEDETAILSDIALOG_H
