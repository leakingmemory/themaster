//
// Created by sigsegv on 5/18/24.
//

#ifndef DRWHATSNOT_FESTDIFFVERSIONSDIALOG_H
#define DRWHATSNOT_FESTDIFFVERSIONSDIALOG_H

#include <wx/wx.h>
#include <string>
#include <memory>

class FestDb;
class OppfRefusjon;
class OppfLegemiddelMerkevare;
class OppfLegemiddelVirkestoff;
class OppfLegemiddelpakning;
class OppfLegemiddeldose;
class OppfKodeverk;
class Element;
template <class T> struct FestDiff;

class FestDiffVersionsDialog : public wxDialog {
private:
    std::string fromVersion;
    std::string toVersion;
    std::shared_ptr<FestDiff<OppfRefusjon>> refusjon{};
    std::shared_ptr<FestDiff<OppfLegemiddelMerkevare>> legemiddelMerkevare{};
    std::shared_ptr<FestDiff<OppfLegemiddelVirkestoff>> legemiddelVirkestoff{};
    std::shared_ptr<FestDiff<OppfLegemiddelpakning>> legemiddelpakning{};
    std::shared_ptr<FestDiff<OppfLegemiddeldose>> legemiddeldose{};
    std::shared_ptr<FestDiff<OppfKodeverk>> kodeverk{};
    std::shared_ptr<FestDiff<Element>> atc{};
public:
    FestDiffVersionsDialog(wxWindow *parent, const std::string &, const std::string &);
    void RunDiff(const std::function<void (int toplevelDone,int toplevelMax, int addsAndRemovesDone, int addsAndRemovesMax, int modificatiosDone, int modificationsMax)> &progress, const std::shared_ptr<FestDb> &db);
    void OnRemoved(wxCommandEvent &);
    void OnModifiedPrevious(wxCommandEvent &e);
    void OnModifiedNew(wxCommandEvent &e);
    void OnAdded(wxCommandEvent &e);
};


#endif //DRWHATSNOT_FESTDIFFVERSIONSDIALOG_H
