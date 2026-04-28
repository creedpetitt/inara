#include "PgConnection.h"
#include "asio/posix/descriptor_base.hpp"
#include "asio/use_awaitable.hpp"

#include <libpq-fe.h>

#include <memory>
#include <stdexcept>
#include <utility>

#include <asio.hpp>

PgConnection::PgConnection(asio::io_context &ctx,
                           const std::string &conn_string) {
    conn_ = PQconnectStart(conn_string.c_str());

    if (conn_ == nullptr) {
        throw std::runtime_error("Failed to allocate Postgres connection");
    }

    if (getStatus() == CONNECTION_BAD) {
        throw std::runtime_error(PQerrorMessage(conn_));
    }

    int conn_fd = PQsocket(conn_);

    if (conn_fd < 0) {
        throw std::runtime_error("Could not retrieve socket from Postgres");
    }

    socket_ = std::make_unique<asio::posix::stream_descriptor>(ctx, conn_fd);
}

PgConnection::PgConnection(PgConnection &&other) noexcept {
    conn_ = other.conn_;
    socket_ = std::move(other.socket_);
    other.conn_ = nullptr;
}

PgConnection &PgConnection::operator=(PgConnection &&other) noexcept {
    if (this != &other) {
        if (conn_ != nullptr) {
            PQfinish(conn_);
        }
        conn_ = other.conn_;
        other.conn_ = nullptr;
        socket_ = std::move(other.socket_);
    }
    return *this;
}

PgConnection::~PgConnection() {
    if (conn_ != nullptr) {
        PQfinish(conn_);
    }
}

bool PgConnection::is_open() const { return PQstatus(conn_) == CONNECTION_OK; }

ConnStatusType PgConnection::getStatus() const {
    if (conn_ == nullptr) {
        return CONNECTION_BAD;
    }

    return PQstatus(conn_);
}

asio::awaitable<void> PgConnection::async_connect() {
    while (true) {
        PostgresPollingStatusType status = PQconnectPoll(conn_);

        if (status == PGRES_POLLING_OK) {
            co_return;
        }

        if (status == PGRES_POLLING_FAILED) {
            throw std::runtime_error(PQerrorMessage(conn_));
        }

        if (status == PGRES_POLLING_READING) {
            co_await socket_->async_wait(
                asio::posix::descriptor_base::wait_read, asio::use_awaitable);
        }

        if (status == PGRES_POLLING_WRITING) {
            co_await socket_->async_wait(
                asio::posix::descriptor_base::wait_write, asio::use_awaitable);
        }
    }
}
