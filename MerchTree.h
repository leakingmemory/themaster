//
// Created by sigsegv on 1/6/25.
//

#ifndef THEMASTER_MERCHTREE_H
#define THEMASTER_MERCHTREE_H

#include <vector>
#include <map>
#include <memory>
#include <variant>
#include "MedicalCodedValue.h"

class OppfMedForbrMatr;
class OppfNaringsmiddel;
class OppfBrystprotese;
class FestDb;
class Handelsvare;

template <typename T> concept CanGetHandelsvare = requires(const T &oppf) {
    { GetHandelsvare(oppf) } -> std::convertible_to<Handelsvare>;
};

struct MerchNode {
    std::vector<std::variant<std::string,std::shared_ptr<MerchNode>>> nodes{};
    std::string id{};
    MedicalCodedValue grp{};
};

struct MerchRefund {
    std::vector<MerchNode> nodes{};
    std::string id{};
    MedicalCodedValue refund{};
};

class MerchTree {
protected:
    typedef std::variant<OppfMedForbrMatr,OppfNaringsmiddel,OppfBrystprotese> ContainerElement;
public:
    virtual std::shared_ptr<ContainerElement> GetContainerElement(const std::string &) const = 0;
    virtual std::vector<MerchRefund> GetRefunds() const = 0;
};

class MerchTreeImpl : public MerchTree {
private:
    std::map<std::string,std::shared_ptr<ContainerElement>> refusjonToElement{};
    std::vector<MerchRefund> refunds{};
public:
    template <CanGetHandelsvare T> void MapElements(FestDb &festDb, const std::string &festVersion, const std::vector<T> &elements);
    MerchTreeImpl(FestDb &festDb, const std::string &festVersion, const std::vector<OppfMedForbrMatr> &medForbrMatr);
    MerchTreeImpl(FestDb &festDb, const std::string &festVersion, const std::vector<OppfNaringsmiddel> &naringsmidler);
    MerchTreeImpl(FestDb &festDb, const std::string &festVersion, const std::vector<OppfBrystprotese> &brystproteser);
    std::shared_ptr<ContainerElement> GetContainerElement(const std::string &) const override;
    [[nodiscard]] std::vector<MerchRefund> GetRefunds() const override;
};


#endif //THEMASTER_MERCHTREE_H
