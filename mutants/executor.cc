// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "mutants/executor.h"

#include "absl/container/flat_hash_map.h"
#include "mutants/mutator_java.h"
#include <fstream>

namespace mutants {

absl::StatusOr<absl::flat_hash_set<Change>> GetPossibleMutations(
    const Bazel &bazel,
    const std::vector<std::unique_ptr<Mutator>> &mutators,
    absl::string_view source_file
) {
  const auto path = (bazel.workspace() / source_file).lexically_normal();
  std::ifstream handle(path, std::ifstream::in);
  if (!handle.is_open()) {
    std::cerr << "Failed to read from file: " << source_file << std::endl;
  }
  std::stringstream stream;
  stream << handle.rdbuf();
  const std::string source_code = stream.str();

  absl::flat_hash_set<Change> changes;
  for (const auto &mutator : mutators) {
    for (const auto &mutation : mutator->FindMutations(source_file, source_code)) {
      changes.emplace(Change{source_file, std::string(source_code), mutation});
    }
  }
  return changes;
}

absl::StatusOr<std::vector<ExecutionResult>> Executor::ExecuteMutationTests(absl::string_view query) {
  // Locate all mutations we can possibly provide.
  const auto mutators = GetMutators();

  std::vector<ExecutionResult> results;

  const auto bazel_workspace = bazel_.workspace();
  const auto test_targets = bazel_.FindTestTargets(query);

  if (!test_targets.ok()) {
    return test_targets.status();
  }

  // Mapping of source file -> targets that directly test it
  absl::flat_hash_map<std::string, absl::flat_hash_set<absl::string_view>> source_file_tests;
  for (const auto &test_target : *test_targets) {
    const auto sources = bazel_.FindTestedSourceFiles(test_target);
    if (!sources.ok()) {
      return sources.status();
    }
    for (const auto &source : *sources) {
      source_file_tests[source].emplace(test_target);
    }
  }

  absl::flat_hash_map<std::string, absl::flat_hash_set<Change>> source_file_changes;
  for (const auto &[source_file, _] : source_file_tests) {
    const auto changes = GetPossibleMutations(bazel_, mutators, source_file);
    if (!changes.ok()) {
      return changes.status();
    }
    source_file_changes[source_file] = *changes;
  }

  for (const auto &[source_file, changes] : source_file_changes) {
    for (const auto &change : changes) {
      const auto start = absl::Now();
      const auto sandbox = Sandbox::Create(bazel_workspace, {change});
      if (!sandbox.ok()) {
        return sandbox.status();
      }

      ExecutionResult execution_result;
      execution_result.changes = change;

      const auto sandbox_bazel = (*sandbox)->Daemon();
      for (const auto &test_target : source_file_tests[source_file]) {
        const auto status = sandbox_bazel.Test(test_target);
        // In these cases the tests behaved as desired. They detected the mutation!
        if (status == BazelTestStatus::kTestFailed || status == BazelTestStatus::kNoTestsFound) {
          execution_result.caught_mutation.emplace(test_target);
        } else {
          execution_result.missed_mutation.emplace(test_target);
        }
      }

      execution_result.workspace_changes_directory = sandbox_bazel.workspace();
      execution_result.duration = absl::Now() - start;
      results.emplace_back(execution_result);
    }
  }
  return results;
}

std::vector<std::unique_ptr<Mutator>> Executor::GetMutators() {
  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<MutatorJava>());
  return mutators;
}

}
