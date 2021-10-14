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

#include "mutants/mutator_java.h"

#include <string>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "absl/strings/string_view.h"

using testing::Eq;
using testing::UnorderedElementsAre;

absl::flat_hash_set<std::string> GetMutations(absl::string_view file, absl::string_view code) {
  const auto results = mutants::MutatorJava()
      .FindMutations(file, code);

  absl::flat_hash_set<std::string> patches;
  for (const auto &result : results) {
    patches.emplace(result.Apply(code));
  }
  return patches;
}

TEST(MutateJavaTest, FindMutations) {
  constexpr absl::string_view kBasicCode = R"(package com.gravypod.sample;
public class Main {
  public static void main(String[] args) {
    int a = 1;
    int b = 2;
    if (a < b) {
      return
    }
    System.exit(1);
  }
})";
  EXPECT_THAT(GetMutations("Basic.java", kBasicCode), UnorderedElementsAre(
      // Negation: if (a < b) -> (!(a < b))
      testing::HasSubstr("if (!(a < b)) {")
  ));
}
