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
#include <google/protobuf/text_format.h>
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

ABSL_FLAG(std::string, workspace_path, "", "Path to the bazel workspace");

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  auto workspace_path = std::filesystem::current_path();
  if (const auto custom_path = absl::GetFlag(FLAGS_workspace_path); !custom_path.empty()) {
    workspace_path = std::filesystem::absolute(custom_path);
  }

  mutants::Bazel bazel(workspace_path);

  const auto test_targets = bazel.FindTestTargets("//...");
  if (!test_targets.ok()) {
    std::cerr << "Could not find test targets: " << test_targets.status() << std::endl;
    return 1;
  }

  for (const auto &test_target : *test_targets) {
    auto tested_sources = bazel.FindTestedSourceFiles(test_target);
    if (!tested_sources.ok()) {
      std::cerr << "Failed to find source files under test: " << tested_sources.status() << std::endl;
      return 1;
    }

    std::cout << test_target << std::endl;
    for (const auto &tested_source : *tested_sources) {
      std::cout << "\t- " << tested_source << std::endl;
    }
  }

  return 0;
}
