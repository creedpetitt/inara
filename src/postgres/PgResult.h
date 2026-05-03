#pragma once

#include <libpq-fe.h>
#include <string_view>

class PgResult {

  private:
    PGresult *res_ = nullptr;
    void ensure_result() const;
    void check_bounds(int row, int col) const;

  public:
    explicit PgResult(PGresult *res);
    PgResult(const PgResult &other) = delete;
    PgResult &operator=(const PgResult &other) = delete;
    PgResult(PgResult &&other) noexcept;
    PgResult &operator=(PgResult &&other) noexcept;
    ~PgResult();

    int rows () const;
    int columns() const; 
    std::string_view column_name(int column) const;
    bool is_null(int row, int column) const;
    std::string_view value(int row, int column) const ;
    int length(int row, int column) const;

    ExecStatusType getResponseStatus() const;
    static bool is_success(ExecStatusType status);
};