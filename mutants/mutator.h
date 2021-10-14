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

#ifndef MUTANTS_MUTANTS_MUTATOR_H_
#define MUTANTS_MUTANTS_MUTATOR_H_

#include <random>
#include <string_view>
#include "absl/strings/string_view.h"
#include "absl/container/flat_hash_set.h"
#include "third_party/diff_match_patch/diff.h"
#include <filesystem>

namespace mutants {
enum class SourceCondition {
  kNegate,
};

struct SourceRange {
  uint32_t start;
  uint32_t stop;
};

struct Mutation {
  SourceRange unary;
  SourceCondition condition;
};

class Mutator {
 public:
  [[nodiscard]] virtual absl::flat_hash_set<diff::Difference> FindMutations(const std::filesystem::path &file, absl::string_view source) const = 0;
};

}

#endif //MUTANTS_MUTANTS_MUTATOR_H_
