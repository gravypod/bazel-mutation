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

#include "mutants/changes.h"
#include "absl/status/statusor.h"
#include "absl/status/status.h"
#include <fstream>

namespace mutants {

bool operator==(const Change &lhs, const Change &rhs) {
  return lhs.relative_path == rhs.relative_path &&
      lhs.difference == rhs.difference;
}

std::ostream &operator<<(std::ostream &os, const Change &change) {
  return os << "Change(" << change.relative_path << ")";
}

absl::Status WriteFile(
    const std::string &path,
    const std::string &data
) {
  std::ofstream handle;
  handle.open(path, std::ifstream::out);
  if (!handle.is_open()) {
    return absl::UnknownError("Failed to open handle");
  }
  handle << data;
  handle.close();
  return absl::OkStatus();
}

absl::Status WriteChanges(
    const std::vector<Change> &changes,
    const std::filesystem::path &target
) {
  for (const auto &change : changes) {
    const auto &relative_path = change.relative_path;

    if (!relative_path.is_relative()) {
      return absl::FailedPreconditionError("provided a non-relative file path");
    }

    const auto absolute_path = (target / relative_path).lexically_normal();

    const auto parent_directory = absolute_path.parent_path();
    if (!std::filesystem::exists(parent_directory)) {
      if (!std::filesystem::create_directories(parent_directory)) {
        return absl::UnknownError("Failed to reconstruct the directory tree");
      }
    }

    const auto new_contents = change.difference.Apply(change.original);

    if (auto status = WriteFile(absolute_path, new_contents); !status.ok()) {
      return status;
    }
  }

  return absl::OkStatus();
}

}