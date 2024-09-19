//
// Created by sigsegv on 9/13/24.
//

#include "MedicamentVisitor.h"

const char *MedicamentVisitorUnmatched::what() const noexcept {
    return "Visitor did not match an object type";
}