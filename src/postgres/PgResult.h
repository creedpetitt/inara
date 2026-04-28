#pragma once

#include <libpq-fe.h>

class PgResult {

  private:
    PGresult *res_ = nullptr;

  public:
    explicit PgResult(PGresult *res);
    PgResult(const PgResult &other) = delete;
    PgResult &operator=(const PgResult &other) = delete;
    PgResult(PgResult &&other) noexcept;
    PgResult &operator=(PgResult &&other) noexcept;
    ~PgResult();
};