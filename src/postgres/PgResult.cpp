#include "PgResult.h"

#include <libpq-fe.h>
#include <stdexcept>

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

ExecStatusType PgResult::getResponseStatus() const {
    if (res_ == nullptr) {
        throw std::runtime_error("PgResult is null");
    }

    return PQresultStatus(res_);
}

bool PgResult::is_success(ExecStatusType status) {
    return status == PGRES_TUPLES_OK || status == PGRES_COMMAND_OK;
}
