#include "PgConnection.h"
#include "PgResult.h"
#include <libpq-fe.h>

#include <memory>
#include <optional>
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

asio::awaitable<PgResult> PgConnection::async_query(const std::string &sql) {
    if (PQsendQuery(conn_, sql.c_str()) == 0) {
        throw std::runtime_error(PQerrorMessage(conn_));
    }

    co_await flush_outgoing();
    co_return co_await async_read();
}

asio::awaitable<void> PgConnection::async_connect() {
    while (true) {
        PostgresPollingStatusType status = PQconnectPoll(conn_);
        if (status == PGRES_POLLING_OK) {
            if (PQsetnonblocking(conn_, 1) == -1) {
                throw std::runtime_error(
                    "Failed to set Postgres connection to non-blocking mode");
            }
            co_return;
        }

        if (status == PGRES_POLLING_FAILED) {
            throw std::runtime_error(PQerrorMessage(conn_));
        }

        if (status == PGRES_POLLING_READING) {
            co_await wait_read();
        }

        if (status == PGRES_POLLING_WRITING) {
            co_await wait_write();
        }
    }
}

asio::awaitable<void> PgConnection::flush_outgoing() {
    while (true) {
        int flush_status = PQflush(conn_);
        if (flush_status == 0) {
            co_return;
        }

        if (flush_status == -1) {
            throw std::runtime_error(PQerrorMessage(conn_));
        }

        co_await wait_write();
        if (PQconsumeInput(conn_) == 0) {
            throw std::runtime_error(PQerrorMessage(conn_));
        }
    }
}

asio::awaitable<PgResult> PgConnection::async_read() {
    std::optional<PgResult> latest_result;
    bool query_failed = false;
    while (true) {
        co_await wait_read();
        // fill buffer with network data from socket
        if (PQconsumeInput(conn_) == 0) {
            throw std::runtime_error(PQerrorMessage(conn_));
        }

        while (PQisBusy(conn_) == 0) {
            PGresult *result = PQgetResult(conn_);
            if (result == nullptr) {

                if (query_failed) {
                    throw std::runtime_error("Postgres query failed");
                }

                if (!latest_result.has_value()) {
                    throw std::runtime_error(
                        "Query complete but didn't return PGresult");
                }

                co_return std::move(latest_result.value());
            }

            PgResult current_result(result);
            if (!PgResult::is_success(current_result.getResponseStatus())) {
                query_failed = true;
            }

            latest_result = std::move(current_result);
        }
    }
}

asio::awaitable<void> PgConnection::wait_read() {
    co_await socket_->async_wait(asio::posix::descriptor_base::wait_read,
                                 asio::use_awaitable);
}

asio::awaitable<void> PgConnection::wait_write() {
    co_await socket_->async_wait(asio::posix::descriptor_base::wait_write,
                                 asio::use_awaitable);
}
