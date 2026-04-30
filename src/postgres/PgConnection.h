#pragma once

#include "PgResult.h"

#include <libpq-fe.h>

#include <memory>
#include <string>

#include <asio.hpp>

class PgConnection {

  private:
    PGconn *conn_ = nullptr;
    std::unique_ptr<asio::posix::stream_descriptor> socket_;
    ConnStatusType getStatus() const;
    asio::awaitable<void> flush_outgoing();
    asio::awaitable<PgResult> read_results();
    asio::awaitable<void> wait_readable();
    asio::awaitable<void> wait_writeable();

  public:
    PgConnection(asio::io_context &ctx, const std::string &conn_string);
    PgConnection(const PgConnection &other) = delete;
    PgConnection &operator=(const PgConnection &other) = delete;
    PgConnection(PgConnection &&other) noexcept;
    PgConnection &operator=(PgConnection &&other) noexcept;
    ~PgConnection();

    bool is_open() const;
    asio::awaitable<PgResult> query(const std::string &sql);
    asio::awaitable<void> connect();
};