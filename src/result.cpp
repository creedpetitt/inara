#include "orbwvr/result.h"
#include "postgres/PgResult.h"

#include <memory>
#include <utility>

namespace orbwvr {
struct result::storage {
    storage(postgres::detail::PgResult &&internal_res)
        : pg_res(std::move(internal_res)) {}
    postgres::detail::PgResult pg_res;
};
result::result(postgres::detail::PgResult &&internal_res)
    : storage_(std::make_shared<storage>(std::move(internal_res))) {}
} // namespace orbwvr