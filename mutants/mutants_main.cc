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

#include "mutants/bazel.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "mutants/executor.h"
#include "mutants/changes.h"
#include "mutants/mutator_java.h"

ABSL_FLAG(std::string, workspace_path, "", "Path to the bazel workspace");
ABSL_FLAG(std::string, test_query, "//...", "What bazel query to use to find test targets.");

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  auto real_workspace_path = std::filesystem::current_path();
  if (const auto custom_path = absl::GetFlag(FLAGS_workspace_path); !custom_path.empty()) {
    real_workspace_path = std::filesystem::absolute(custom_path);
  }

  mutants::Bazel bazel(real_workspace_path);
  mutants::Executor executor(bazel);

  const absl::StatusOr<std::vector<mutants::ExecutionResult>> results
      = executor.ExecuteMutationTests(absl::GetFlag(FLAGS_test_query));
  if (!results.ok()) {
    std::cerr << results.status() << std::endl;
    return 1;
  }

  std::cout << "Found " << results->size() << " results." << std::endl;
  for (const auto &result : *results) {
    const auto status = result.caught_mutation.empty() ?
                        "FAILED (missed mutation)" :
                        "PASSED (caught mutation)";
    absl::Duration execution_time = absl::Trunc(result.duration, absl::Seconds(1));
    // java/com/gravypod/simple/Basic.java (10 minutes): FAILED (missed mutation)
    std::cout << result.changes << " [" << execution_time << "]: " << status << std::endl;

    std::cout << "\t- Workspace: " << result.workspace_changes_directory << std::endl;
    std::cout << "\t- Caught:" << std::endl;
    for (const auto &test_target : result.caught_mutation) {
      std::cout << "\t\t* " << test_target << std::endl;
    }

    std::cout << "\t- Missed:" << std::endl;
    for (const auto &test_target : result.missed_mutation) {
      std::cout << "\t\t* " << test_target << std::endl;
    }
  }

  return 0;
}
