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

#include "third_party/random_string/random_string.h"
#include "third_party/tensorflow/status_matchers.h"
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "gtest/gtest-matchers.h"
#include <fstream>

std::filesystem::path TempDir(absl::string_view name) {
  return std::filesystem::path(testing::TempDir()) / random_string(6) / name;
}

std::string ReadFile(const std::filesystem::path &file) {
  std::ifstream handle(file);
  EXPECT_TRUE(handle.good());
  std::string result;
  handle >> result;
  return result;
}

TEST(ChangesTest, CanWriteChanges) {
  const auto target = TempDir("can_write_changes");
  std::vector<mutants::Change> changes;
  changes.push_back({"root.txt", "", diff::Difference::Create("", "Root")});
  changes.push_back({"nested/file.txt", "", diff::Difference::Create("", "Nested")});

  ASSERT_THAT(mutants::WriteChanges(changes, target), tensorflow::testing::IsOk());
  ASSERT_EQ("Root", ReadFile(target / "root.txt"));
  ASSERT_EQ("Nested", ReadFile(target / "nested/file.txt"));
}
