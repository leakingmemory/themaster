//
// Created by sigsegv on 3/18/25.
//

#ifndef THEMASTER_EDITREFERENCENUMBERSDIALOG_H
#define THEMASTER_EDITREFERENCENUMBERSDIALOG_H

#include <wx/wx.h>
#include <wx/listctrl.h>

class EditReferenceNumbersDialog : public wxDialog {
private:
    wxListView *refNumbers;
    wxTextCtrl *addRef;
    wxButton *addButton;
public:
    EditReferenceNumbersDialog() = delete;
    EditReferenceNumbersDialog(wxWindow *, std::vector<std::string> refNumbers);
    EditReferenceNumbersDialog(const EditReferenceNumbersDialog &) = delete;
    EditReferenceNumbersDialog(EditReferenceNumbersDialog &&) = delete;
    EditReferenceNumbersDialog &operator =(const EditReferenceNumbersDialog &) = delete;
    EditReferenceNumbersDialog &operator =(EditReferenceNumbersDialog &&) = delete;
    bool HasReferenceNumber(const wxString &) const;
    bool HasReferenceNumber(const std::string &) const;
    [[nodiscard]] std::vector<std::string> GetReferenceNumbers() const;
    void OnEditAddReferenceNumber(const wxCommandEvent &e);
    void OnCharListEvent(wxKeyEvent &e);
    void OnCharEventEdit(wxKeyEvent &e);
    void OnAddReferenceNumber(const wxCommandEvent &e);
    void OnContextMenu(const wxContextMenuEvent &e);
    void OnCopyReferenceNumber(const wxCommandEvent &e);
    void OnRemoveReferenceNumber(const wxCommandEvent &e);
};


#endif //THEMASTER_EDITREFERENCENUMBERSDIALOG_H
