#pragma once

#include "PgResult.h"
#include "asio/awaitable.hpp"

#include <libpq-fe.h>

#include <memory>
#include <string>

#include <asio.hpp>

class PgConnection {

  private:
    PGconn *conn_ = nullptr;
    std::unique_ptr<asio::posix::stream_descriptor> socket_;

  public:
    PgConnection(asio::io_context &ctx, const std::string &conn_string);
    PgConnection(const PgConnection &other) = delete;
    PgConnection &operator=(const PgConnection &other) = delete;
    PgConnection(PgConnection &&other) noexcept;
    PgConnection &operator=(PgConnection &&other) noexcept;
    ~PgConnection();

    bool is_open() const;
    ConnStatusType getStatus() const;
    asio::awaitable<PgResult> async_query(const std::string &sql);
    asio::awaitable<void> async_connect();
};