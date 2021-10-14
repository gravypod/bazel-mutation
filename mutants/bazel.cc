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
#include "third_party/subprocess/subprocess.hpp"
#include "absl/strings/str_format.h"
#include "absl/strings/str_replace.h"
#include "absl/status/statusor.h"
#include <filesystem>

namespace mutants {

void Bazel::Shutdown() {
  const auto cwd = std::string(cwd_);

  auto process = subprocess::Popen(
      {std::string(executable_), "shutdown"},
      subprocess::output("/dev/null"),
      subprocess::error("/dev/null"),
      subprocess::cwd{cwd}
  );
  process.wait();
}

absl::StatusOr<blaze_query::QueryResult> Bazel::FindTargets(
    absl::string_view query) const {
  const auto cwd = std::string(cwd_);
  auto process = subprocess::Popen(
      {std::string(executable_), "query", "--noorder_results", "--output=proto", std::string(query)},
      subprocess::output{subprocess::PIPE},
      subprocess::error("/dev/null"),
      subprocess::cwd{cwd}
  );

  auto data = process.communicate().first.buf;

  blaze_query::QueryResult result;
  result.ParseFromArray(data.data(), data.size());

  if (process.wait() != 0) {
    return absl::UnknownError("Failed to execute Bazel query");
  }
  return result;
}

absl::StatusOr<absl::flat_hash_set<std::string>> Bazel::FindTestTargets(absl::string_view test_target) const {
  const auto query_result = this->FindTargets(
      absl::StrFormat("kind(.*_test, '%s')", test_target)
  );
  if (!query_result.ok()) {
    return query_result.status();
  }

  absl::flat_hash_set<std::string> test_targets;
  for (const auto &target : query_result->target()) {
    test_targets.emplace(target.rule().name());
  }
  return test_targets;
}

absl::StatusOr<absl::flat_hash_set<std::string>> Bazel::FindTestedSourceFiles(absl::string_view test_target) const {
  const auto query_result = FindTargets(
      absl::StrFormat("kind('source file', deps(kind(.*_library, attr(testonly, 0, deps('%s', 1)))) intersect siblings('%s'))", test_target, test_target)
  );
  if (!query_result.ok()) {
    return query_result.status();
  }

  absl::flat_hash_set<std::string> source_files;
  for (const auto &target : query_result->target()) {
    const auto &target_name = target.source_file().name();
    source_files.emplace(absl::StrReplaceAll(target_name.substr(2), {
        {":", "/"},
    }));
  }
  return source_files;
}

BazelTestStatus Bazel::Test(absl::string_view test_target) const {
  const auto cwd = std::string(cwd_);
  auto process = subprocess::Popen(
      {std::string(executable_), "test", std::string(test_target)},
      subprocess::output("/dev/null"),
      subprocess::error("/dev/null"),
      subprocess::cwd{cwd}
  );

  // More details available here:
  // https://docs.bazel.build/versions/0.21.0/guide.html#what-exit-code-will-i-get
  switch (process.wait()) {
    case 0:
      return BazelTestStatus::kSuccess;
    case 1:
      return BazelTestStatus::kBuildFailed;
    case 3:
      return BazelTestStatus::kTestFailed;
    case 4:
      return BazelTestStatus::kNoTestsFound;
    default:
      return BazelTestStatus::kUnknown;
  }

}

}
