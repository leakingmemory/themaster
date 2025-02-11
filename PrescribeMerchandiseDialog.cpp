//
// Created by sigsegv on 1/17/25.
//

#include "PrescribeMerchandiseDialog.h"
#include <wx/treectrl.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include "MerchTree.h"
#include "WxDateConversions.h"
#include <medfest/Struct/Decoded/OppfHandelsvare.h>
#include "DateOnly.h"

PrescribeMerchandiseDialog::PrescribeMerchandiseDialog(wxWindow *parent, const MerchTree &merchTree) : wxDialog(parent, wxID_ANY, wxT("Prescribe merchandise/nourishment")) {
    DateOnly startDate = DateOnly::Today();
    DateOnly endDate = startDate;
    endDate.AddYears(1);
    auto *superSizer = new wxBoxSizer(wxVERTICAL);
    auto *mainSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *col1Sizer = new wxBoxSizer(wxVERTICAL);
    treeView = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(300, 400), (wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_HIDE_ROOT));
    auto rootId = treeView->AddRoot(wxT("Paragraphs"));
    for (const auto &refund : merchTree.GetRefunds()) {
        auto refundId = treeView->AppendItem(rootId, wxString::FromUTF8(refund.refund.GetDisplay()));
        std::vector<std::tuple<wxTreeItemId,std::vector<std::shared_ptr<MerchNode>>>> nodeLists{};
        {
            std::vector<std::shared_ptr<MerchNode>> nodes{};
            for (const auto &node : refund.nodes) {
                nodes.emplace_back(std::make_shared<MerchNode>(node));
            }
            nodeLists.emplace_back(refundId, nodes);
        }
        while (!nodeLists.empty()) {
            wxTreeItemId wxItemId{};
            std::vector<std::shared_ptr<MerchNode>> nodes{};
            {
                auto iterator = nodeLists.begin();
                wxItemId = std::get<0>(*iterator);
                nodes = std::get<1>(*iterator);
                nodeLists.erase(iterator);
            }
            std::vector<std::tuple<wxTreeItemId,std::string>> items{};
            for (const auto &node : nodes) {
                auto wxNodeId = treeView->AppendItem(wxItemId, wxString::FromUTF8(node->grp.GetDisplay()));
                MerchRefundInfo refundInfo{
                    .productGroup = node->grp,
                    .paragraph = refund.refund
                };
                refundInfoMap.insert_or_assign(wxNodeId, refundInfo);
                if (!node->nodes.empty()) {
                    std::vector<std::shared_ptr<MerchNode>> subNodes{};
                    for (const auto &subVariant : node->nodes) {
                        if (std::holds_alternative<std::shared_ptr<MerchNode>>(subVariant)) {
                            subNodes.emplace_back(std::get<std::shared_ptr<MerchNode>>(subVariant));
                        } else {
                            items.emplace_back(wxNodeId, std::get<std::string>(subVariant));
                        }
                    }
                    if (!subNodes.empty()) {
                        nodeLists.emplace_back(wxNodeId, subNodes);
                    }
                }
            }
            for (const auto &item : items) {
                auto wxNodeId = std::get<0>(item);
                auto id = std::get<1>(item);
                auto element = merchTree.GetContainerElement(id);
                if (!element) {
                    continue;
                }
                struct {
                    std::string operator() (const OppfMedForbrMatr &oppf) {
                        return oppf.GetMedForbrMatr().GetNavn();
                    }
                    std::string operator() (const OppfNaringsmiddel &oppf) {
                        return oppf.GetNaringsmiddel().GetNavn();
                    }
                    std::string operator() (const OppfBrystprotese &oppf) {
                        return oppf.GetBrystprotese().GetNavn();
                    }
                } GetNameVisitor;
                std::string name = std::visit(GetNameVisitor, *element);
                treeView->AppendItem(wxNodeId, wxString::FromUTF8(name));
            }
        }
    }
    col1Sizer->Add(treeView, 1, wxALL | wxEXPAND, 5);
    mainSizer->Add(col1Sizer, 1, wxALL | wxEXPAND, 5);
    auto *col2Sizer = new wxBoxSizer(wxVERTICAL);
    auto *paragraphSizer = new wxBoxSizer(wxHORIZONTAL);
    paragraphSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Paragraph: ")), 0, wxALL | wxEXPAND, 5);
    paragraph = new wxTextCtrl(this, wxID_ANY);
    paragraphSizer->Add(paragraph, 1, wxALL | wxEXPAND, 5);
    col2Sizer->Add(paragraphSizer, 0, wxALL | wxEXPAND, 5);
    auto productGroupSizer = new wxBoxSizer(wxHORIZONTAL);
    productGroupSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Product group: ")), 0, wxALL | wxEXPAND, 5);
    productGroup = new wxTextCtrl(this, wxID_ANY);
    productGroupSizer->Add(productGroup, 1, wxALL | wxEXPAND, 5);
    col2Sizer->Add(productGroupSizer, 0, wxALL | wxEXPAND, 5);
    auto *dssnSizer = new wxBoxSizer(wxHORIZONTAL);
    dssn = new wxTextCtrl(this, wxID_ANY);
    dssnSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Dssn: ")));
    dssnSizer->Add(dssn, 1, wxALL | wxEXPAND, 5);
    col2Sizer->Add(dssnSizer, 0, wxALL | wxEXPAND, 5);
    auto *startDateSizer = new wxBoxSizer(wxHORIZONTAL);
    startDateSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Start: ")), 0, wxEXPAND | wxALL, 5);
    this->startDate = new wxDatePickerCtrl(this, wxID_ANY, ToWxDateTime(startDate));
    startDateSizer->Add(this->startDate, 1, wxEXPAND | wxALL, 5);
    col2Sizer->Add(startDateSizer, 0, wxALL | wxEXPAND, 5);
    auto *expirationDateSizer = new wxBoxSizer(wxHORIZONTAL);
    expirationDateSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Expires: ")), 0, wxEXPAND | wxALL, 5);
    expirationDate = new wxDatePickerCtrl(this, wxID_ANY, ToWxDateTime(endDate));
    expirationDateSizer->Add(expirationDate, 0, wxEXPAND | wxALL, 5);
    col2Sizer->Add(expirationDateSizer, 0, wxALL | wxEXPAND, 5);
    mainSizer->Add(col2Sizer, 1, wxALL | wxEXPAND, 5);
    treeView->Bind(wxEVT_TREE_SEL_CHANGED, &PrescribeMerchandiseDialog::OnSelectedRefundTreeItem, this, wxID_ANY);
    dssn->Bind(wxEVT_TEXT, &PrescribeMerchandiseDialog::OnDssnChanged, this, wxID_ANY);
    this->startDate->Bind(wxEVT_DATE_CHANGED, &PrescribeMerchandiseDialog::OnDateChanged, this, wxID_ANY);
    this->expirationDate->Bind(wxEVT_DATE_CHANGED, &PrescribeMerchandiseDialog::OnDateChanged, this, wxID_ANY);
    superSizer->Add(mainSizer, 1, wxALL | wxEXPAND, 5);
    auto *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    okButton = new wxButton(this, wxID_OK, wxT("Ok"));
    buttonsSizer->Add(okButton, 1, wxALL | wxEXPAND, 5);
    cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"));
    buttonsSizer->Add(cancelButton, 1, wxALL | wxEXPAND, 5);
    superSizer->Add(buttonsSizer, 0, wxCENTER | wxALL, 5);
    okButton->Enable(false);
    SetSizerAndFit(superSizer);
}

void PrescribeMerchandiseDialog::OnSelectedRefundTreeItem(wxCommandEvent &) {
    auto selectedId = treeView->GetSelection();
    auto iterator = refundInfoMap.find(selectedId);
    if (iterator != refundInfoMap.end()) {
        refundInfo = iterator->second;
    } else {
        refundInfo = {};
    }
    paragraph->SetValue(wxString::FromUTF8(refundInfo.paragraph.GetDisplay()));
    productGroup->SetValue(wxString::FromUTF8(refundInfo.productGroup.GetDisplay()));
    RefreshButons();
}

void PrescribeMerchandiseDialog::OnDssnChanged(wxCommandEvent &e) {
    RefreshButons();
}

void PrescribeMerchandiseDialog::OnDateChanged(wxDateEvent &e) {
    RefreshButons();
}

MerchData PrescribeMerchandiseDialog::GetMerchData() const {
    return {
        .refund = refundInfo,
        .dssn = dssn->GetValue().ToStdString(),
        .startDate = ToDateOnly(startDate->GetValue()),
        .expirationDate = ToDateOnly(expirationDate->GetValue())
    };
}

void PrescribeMerchandiseDialog::RefreshButons() {
    auto data = GetMerchData();
    auto valid = IsValid(data);
    okButton->Enable(valid);
}

bool PrescribeMerchandiseDialog::IsValid(const MerchData &data) {
    if (data.refund.productGroup.GetCode().empty()) {
        return false;
    }
    if (data.refund.paragraph.GetCode().empty()) {
        return false;
    }
    if (data.dssn.empty()) {
        return false;
    }
    if (!data.startDate.operator bool()) {
        return false;
    }
    if (!data.expirationDate.operator bool()) {
        return false;
    }
    if (data.expirationDate < data.startDate) {
        return false;
    }
    return true;
}
