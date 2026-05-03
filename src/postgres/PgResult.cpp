#include "PgResult.h"

#include <libpq-fe.h>
#include <stdexcept>
#include <string_view>

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

int PgResult::rows() const {
    ensure_result();
    return PQntuples(res_);
}

int PgResult::columns() const {
    ensure_result();
    return PQnfields(res_);
}

std::string_view PgResult::column_name(int col) const {
    ensure_result();
    const char *name = PQfname(res_, col);
    if (name == nullptr) {
        throw std::out_of_range("Column index out of range");
    }
    return std::string_view(name);
}

bool PgResult::is_null(int row, int col) const {
    ensure_result();
    check_bounds(row, col);
    return (PQgetisnull(res_, row, col) == 1);
}

std::string_view PgResult::value(int row, int col) const {
    ensure_result();
    check_bounds(row, col);
    const char * value = PQgetvalue(res_, row, col);  
    int len = length(row, col);
    return std::string_view(value, static_cast<size_t>(len));
}

int PgResult::length(int row, int col) const {
    ensure_result();
    check_bounds(row, col);
    return PQgetlength(res_, row, col);
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

void PgResult::ensure_result() const {
    if (res_ == nullptr) {
        throw std::runtime_error("PgResult is null");
    }
}

void PgResult::check_bounds(int row, int col) const {
    if (row < 0 || row >= rows()) {
        throw std::out_of_range("Row index out of range");
    }
    if (col < 0 || col >= columns()) {
        throw std::out_of_range("Column index out of range");
    }
}
