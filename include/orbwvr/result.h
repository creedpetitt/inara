#pragma once

#include <memory>

namespace orbwvr::postgres::detail {
class PgResult;
}
namespace orbwvr {
class result {
  public:
    explicit result(postgres::detail::PgResult &&internal_res);

  private:
    struct storage;
    std::shared_ptr<storage> storage_;
};
} // namespace orbwvr