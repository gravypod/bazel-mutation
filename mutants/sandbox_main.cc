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
#include "mutants/sandbox.h"

ABSL_FLAG(std::string, workspace_path, "", "Path to the bazel workspace");

void AddFile(std::vector<mutants::Change> &changes, absl::string_view path, absl::string_view contents) {
  changes.push_back(mutants::Change{
                        path,
                        "",
                        diff::Difference::Create("", contents),
                    });
}

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  auto real_workspace_path = std::filesystem::current_path();
  if (const auto custom_path = absl::GetFlag(FLAGS_workspace_path); !custom_path.empty()) {
    real_workspace_path = std::filesystem::absolute(custom_path);
  }

  // Make a sandbox workspace with a new test!
  std::vector<mutants::Change> changes;
  AddFile(changes, "simplecc/simple.h", R"c(
#ifndef SIMPLECC
#define SIMPLECC
int thing();
#endif
)c");
  AddFile(changes,
          "simplecc/simple.cc", R"c(
int thing() { return 1; }
)c");
  AddFile(changes, "simplecc/simple_test.cc", R"c(
#include "simplecc/simple.h"

int main() {
  return thing();
}
)c");
  AddFile(changes, "simplecc/BUILD.bazel", R"build(
cc_library(
  name = "simple",
  srcs = ["simple.cc"],
  hdrs = ["simple.h"],
)

cc_test(
  name = "simple_test",
  srcs = ["simple_test.cc"],
  deps = [":simple"],
)
)build");
  const auto sandbox = *mutants::Sandbox::Create(real_workspace_path, changes);
  std::cout << "Using sandbox workspace: " << sandbox->Workspace() << std::endl;
  const auto &bazel = sandbox->Daemon();

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
