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

#ifndef MUTANTS_MUTANTS_MUTATOR_JAVA_H_
#define MUTANTS_MUTANTS_MUTATOR_JAVA_H_

#include "mutants/mutator.h"
#include "third_party/tree_sitter/api.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace mutants {
class MutatorJava : public mutants::Mutator {
 public:
  [[nodiscard]] absl::flat_hash_set<diff::Difference> FindMutations(const std::filesystem::path &file, absl::string_view source) const final;
 private:
  [[nodiscard]] bool CanMutate(const std::filesystem::path &path) const;
  [[nodiscard]] std::vector<Mutation> FindPossibleMutations(absl::string_view source) const;
  [[nodiscard]] std::string ApplyMutation(absl::string_view source, Mutation mutation) const;
};
}

#endif //MUTANTS_MUTANTS_MUTATOR_JAVA_H_
