#include "orbwvr.h"

#include <exception>
#include <iostream>

orbwvr::async<void> run_database_test() {
    try {
        orbwvr::database db(
            "postgresql://postgres:DB_NAME@localhost:PORT/postgres");

        // 1. Basic query with no parameters
        co_await db.query("SELECT 1");
        std::cout << "SUCCESSFULLY QUERIED DB" << std::endl;

        // 2. Query using inline initializer lists for parameters
        co_await db.query_params("SELECT $1::int", {"1"});
        std::cout << "SUCCESSFULLY QUERIED DB WITH PARAMS" << std::endl;

        // 3. Prepare a statement for later use
        co_await db.prepare("s1", "SELECT $1::int");
        std::cout << "SUCCESSFULLY PREPARED STATEMENT" << std::endl;

        // 4. Execute the prepared statement using inline parameters
        co_await db.query_prepared("s1", {"1"});
        std::cout << "SUCCESSFULLY QUERIED PREPARED STATEMENT" << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
    co_return;
}

int main() {
    try {
        orbwvr::sync_wait(run_database_test());
    } catch (const std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
    }
}
