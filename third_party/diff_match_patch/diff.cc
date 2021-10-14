#include "third_party/diff_match_patch/diff.h"
#include "third_party/diff_match_patch/diff_match_patch.h"
#include "absl/strings/string_view.h"
#include <memory>
#include <ostream>
#include <string>

namespace diff {
Difference Difference::Create(absl::string_view left, absl::string_view right) {
  diff_match_patch dmp;
  Difference difference;
  difference.patch = dmp.patch_toText(dmp.patch_make(left, right));
  return difference;
}
std::string Difference::Apply(absl::string_view original) const {
  diff_match_patch dmp;
  return dmp.patch_apply(dmp.patch_fromText(patch), original).first;
}

bool operator==(const Difference &lhs, const Difference &rhs) {
  return lhs.patch == rhs.patch;
}
std::ostream &operator<<(std::ostream &os, const Difference &difference) {
  return os << difference.patch;
}

}