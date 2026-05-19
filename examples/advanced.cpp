#include "orbwvr/database.h"

#include <exception>
#include <iostream>
#include <vector>

orbwvr::async<void> run_database_test() {
    try {
        orbwvr::database db(
            "postgresql://postgres:DB_NAME@localhost:PORT/postgres");

        // 1. DDL using basic query()
        std::cout << "Creating table..." << std::endl;
        co_await db.query("DROP TABLE IF EXISTS test_users");
        co_await db.query("CREATE TABLE test_users (id SERIAL PRIMARY KEY, "
                          "name TEXT, age INT, score REAL)");
        std::cout << "SUCCESSFULLY EXECUTED DDL" << std::endl;

        // 2. Insert data using inline initializer lists (no explicit std::vector declaration needed!)(
        std::cout << "Inserting first user..." << std::endl;
        co_await db.query_params(
            "INSERT INTO test_users (name, age, score) VALUES ($1, $2, $3)",
            {"Alice", "30", "95.5"});
        std::cout << "SUCCESSFULLY INSERTED WITH PARAMS" << std::endl;

        // 3. Prepare a statement
        std::cout << "Preparing insert statement..." << std::endl;
        co_await db.prepare(
            "insert_user",
            "INSERT INTO test_users (name, age, score) VALUES ($1, $2, $3)");
        std::cout << "SUCCESSFULLY PREPARED STATEMENT" << std::endl;

        // 4. Execute the prepared statement multiple times using inline lists
        std::cout << "Inserting users via prepared statement..." << std::endl;
        co_await db.query_prepared("insert_user", {"Bob", "25", "88.0"});
        co_await db.query_prepared("insert_user", {"Charlie", "35", "92.1"});
        std::cout << "SUCCESSFULLY QUERIED PREPARED STATEMENT" << std::endl;

        // 5. Query the data back out using params
        std::cout << "Selecting data..." << std::endl;
        co_await db.query_params(
            "SELECT name, score FROM test_users WHERE age > $1", {"28"});
        std::cout << "SUCCESSFULLY SELECTED DATA" << std::endl;

        // Clean up
        co_await db.query("DROP TABLE test_users");
        std::cout << "SUCCESSFULLY CLEANED UP" << std::endl;

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
