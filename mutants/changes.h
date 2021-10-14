/*
 * Copyright 2023 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MUTANTS_MUTANTS_CHANGES_H_
#define MUTANTS_MUTANTS_CHANGES_H_

#include <unordered_map>
#include <filesystem>
#include <utility>
#include <vector>
#include "absl/status/status.h"
#include "absl/container/flat_hash_map.h"
#include "third_party/diff_match_patch/diff.h"

namespace mutants {

struct Change {
  std::filesystem::path relative_path;
  std::string original;
  diff::Difference difference;
};

bool operator==(const Change &lhs, const Change &rhs);

template<typename H>
H AbslHashValue(H h, const Change &c) {
  return H::combine(std::move(h), c.relative_path.native(), c.difference);
}

std::ostream &operator<<(std::ostream &os, const Change &change);

absl::Status WriteChanges(
    const std::vector<Change> &changes,
    const std::filesystem::path &target
);
}

#endif //MUTANTS_MUTANTS_CHANGES_H_
