#ifndef MUTANTS_THIRD_PARTY_DIFF_MATCH_PATCH_DIFF_H_
#define MUTANTS_THIRD_PARTY_DIFF_MATCH_PATCH_DIFF_H_

#include <string>
#include <memory>
#include <utility>
#include "absl/strings/string_view.h"

namespace diff {

struct Difference {
 public:
  explicit Difference(std::string patch) : patch(std::move(patch)) {}
  Difference() = default;
  static Difference Create(absl::string_view left, absl::string_view right);
  [[nodiscard]] std::string Apply(absl::string_view original) const;

  friend bool operator==(const Difference &lhs, const Difference &rhs);
  friend std::ostream &operator<<(std::ostream &os, const Difference &difference);
  template<typename H>
  friend H AbslHashValue(H h, const Difference &m);
  explicit operator std::string() const { return patch; }
 private:
  std::string patch;
};

template<typename H>
H AbslHashValue(H h, const Difference &c) {
  return H::combine(std::move(h), c.patch);
}

}

#endif //MUTANTS_THIRD_PARTY_DIFF_MATCH_PATCH_DIFF_H_
