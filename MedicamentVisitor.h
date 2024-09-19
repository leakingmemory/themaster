//
// Created by sigsegv on 9/12/24.
//

#ifndef THEMASTER_MEDICAMENTVISITOR_H
#define THEMASTER_MEDICAMENTVISITOR_H

#include <concepts>
#include <utility>
#include <memory>
#include <medfest/Struct/Decoded/LegemiddelMerkevare.h>
#include <medfest/Struct/Decoded/LegemiddelVirkestoff.h>
#include <medfest/Struct/Decoded/Legemiddelpakning.h>

class MedicamentVisitorUnmatched : public std::exception {
public:
    const char * what() const noexcept override;
};

template <typename Visitor, typename Result> concept MedicamentVisitor = requires (Visitor visitor) {
    { visitor.Visit(std::declval<LegemiddelMerkevare>()) } -> std::convertible_to<Result>;
    { visitor.Visit(std::declval<LegemiddelVirkestoff>()) } -> std::convertible_to<Result>;
    { visitor.Visit(std::declval<Legemiddelpakning>()) } -> std::convertible_to<Result>;
};

class MedicamentVisitorBase {
public:
    template <typename Result, MedicamentVisitor<Result> Visitor> static Result Visit(const LegemiddelCore &legemiddelCore, Visitor &visitor) {
        const auto *merkevare = dynamic_cast<const LegemiddelMerkevare *>(&legemiddelCore);
        if (merkevare != nullptr) {
            return visitor.Visit(*merkevare);
        }
        const auto *virkestoff = dynamic_cast<const LegemiddelVirkestoff *>(&legemiddelCore);
        if (virkestoff != nullptr) {
            return visitor.Visit(*virkestoff);
        }
        const auto *pakning = dynamic_cast<const Legemiddelpakning *>(&legemiddelCore);
        if (pakning) {
            return visitor.Visit(*pakning);
        }
        throw MedicamentVisitorUnmatched();
    }
};

#endif //THEMASTER_MEDICAMENTVISITOR_H
