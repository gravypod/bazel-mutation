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

#ifndef MUTANTS_MUTANTS_BAZEL_H_
#define MUTANTS_MUTANTS_BAZEL_H_

#include <sys/param.h>
#include "absl/strings/string_view.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "third_party/bazel/build.pb.h"
#include "absl/status/statusor.h"
#include <filesystem>
#include <utility>

namespace mutants {

enum class BazelTestStatus {
  kUnknown,
  kSuccess,
  kBuildFailed,
  kTestFailed,
  kNoTestsFound,
};

inline static constexpr absl::string_view BazelTestStatusString(BazelTestStatus status) {
  switch (status) {
    case BazelTestStatus::kSuccess:
      return "BazelTestStatus::kSuccess";
    case BazelTestStatus::kBuildFailed:
      return "BazelTestStatus::kBuildFailed";
    case BazelTestStatus::kTestFailed:
      return "BazelTestStatus::kTestFailed";
    case BazelTestStatus::kNoTestsFound:
      return "BazelTestStatus::kNoTestsFound";
    case BazelTestStatus::kUnknown:
    default:
      return "BazelTestStatus::kUnknown";
  }
}

class Bazel {
 public:
  explicit Bazel(absl::string_view executable, const std::filesystem::path &cwd) : executable_(executable), cwd_(std::filesystem::absolute(cwd)) {}
  explicit Bazel(const std::filesystem::path &cwd) :
      Bazel("bazel", cwd) {}
  Bazel() :
      Bazel("bazel", std::filesystem::current_path()) {}

  void Shutdown();

  [[nodiscard]] absl::StatusOr<blaze_query::QueryResult> FindTargets(absl::string_view query) const;

  [[nodiscard]] absl::StatusOr<absl::flat_hash_set<std::string>> FindTestTargets(absl::string_view test_target) const;

  [[nodiscard]] absl::StatusOr<absl::flat_hash_set<std::string>> FindTestedSourceFiles(absl::string_view test_target) const;

  [[nodiscard]] BazelTestStatus Test(absl::string_view test_target) const;

  // Current workspace.
  [[nodiscard]] const std::filesystem::path &workspace() const { return cwd_; }

 private:
  // What bazel executable to use. This can be "bazel", "bazelisk", or a full path to somewhere on
  // disk (supports an npm workflow).
  std::string executable_;
  std::filesystem::path cwd_;
};

}

#endif //MUTANTS_MUTANTS_BAZEL_H_
