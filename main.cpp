#include <iostream>
#include <asio.hpp>

using asio::awaitable;
using asio::use_awaitable;
using asio::co_spawn;
using asio::detached;

awaitable<void> run_client_session(asio::io_context& ctx) {

    std::cout << "Running client session" << std::endl;
    co_return;
}

int main()
{
    try {
        asio::io_context io_context;
        asio::signal_set signals(io_context, SIGINT, SIGINT);
        signals.async_wait([&](auto, auto)
        {
            io_context.stop();
        });
        co_spawn(io_context, run_client_session(io_context), detached);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}