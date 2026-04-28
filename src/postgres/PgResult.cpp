#include "PgResult.h"

#include <libpq-fe.h>

PgResult::PgResult(PGresult *res) { res_ = res; }

PgResult::PgResult(PgResult &&other) noexcept {
    res_ = other.res_;
    other.res_ = nullptr;
}

PgResult &PgResult::operator=(PgResult &&other) noexcept {
    if (this != &other) {
        if (res_ != nullptr) {
            PQclear(res_);
        }
        res_ = other.res_;
        other.res_ = nullptr;
    }
    return *this;
}

PgResult::~PgResult() {
    if (res_ != nullptr) {
        PQclear(res_);
    }
}