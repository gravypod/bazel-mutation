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

#ifndef MUTANTS_MUTANTS_SANDBOX_H_
#define MUTANTS_MUTANTS_SANDBOX_H_

#include <unordered_map>
#include <utility>
#include <vector>
#include <memory>
#include <filesystem>
#include "absl/status/statusor.h"
#include "mutants/changes.h"
#include "mutants/bazel.h"

namespace mutants {

class Sandbox {
 public:
  Sandbox(std::filesystem::path sandbox_root, std::filesystem::path sandbox_workspace_root)
      : sandbox_root_(std::move(sandbox_root)),
        sandbox_workspace_root_(std::move(sandbox_workspace_root)) {
    bazel_ = Bazel(sandbox_workspace_root_);
  }
  // Unmount and remove all files created on disk for this sandbox.
  ~Sandbox();

  static absl::StatusOr<std::unique_ptr<Sandbox>> Create(
      const std::filesystem::path &base_workspace,
      const std::vector<Change> &changes
  );

  // The root of the fake bazel workspace we have created for this sandbox.
  const std::filesystem::path &Workspace() { return sandbox_workspace_root_; }
  [[nodiscard]] const Bazel &Daemon() const { return bazel_; }

 private:

  // Where on disk we store the data generated for this sandbox.
  std::filesystem::path sandbox_root_;

  // Where on disk the "fake" Bazel workspace is. This is the workspace
  // with our changes applied.
  std::filesystem::path sandbox_workspace_root_;
  Bazel bazel_;

};

}

#endif //MUTANTS_MUTANTS_SANDBOX_H_
