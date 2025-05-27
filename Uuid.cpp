//
// Created by sigsegv on 5/20/25.
//

#include "Uuid.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

std::string Uuid::RandomUuidString() {
    boost::uuids::random_generator generator;
    boost::uuids::uuid randomUUID = generator();
    return boost::uuids::to_string(randomUUID);
}