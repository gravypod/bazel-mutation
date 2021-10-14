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

#ifndef MUTANTS_MUTANTS_EXECUTOR_H_
#define MUTANTS_MUTANTS_EXECUTOR_H_

#include <utility>

#include "mutants/sandbox.h"
#include "mutants/changes.h"
#include "mutants/mutator.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/container/flat_hash_map.h"

namespace mutants {

struct ExecutionResult {
  absl::flat_hash_set<std::string> caught_mutation;
  absl::flat_hash_set<std::string> missed_mutation;
  std::filesystem::path workspace_changes_directory;
  Change changes;
  absl::Duration duration;
};

class Executor {
 public:
  explicit Executor(Bazel bazel) : bazel_(std::move(bazel)) {}
  absl::StatusOr<std::vector<ExecutionResult>> ExecuteMutationTests(absl::string_view query);
 private:
  static inline std::vector<std::unique_ptr<Mutator>> GetMutators();
  const Bazel bazel_;
};

}

#endif //MUTANTS_MUTANTS_EXECUTOR_H_
