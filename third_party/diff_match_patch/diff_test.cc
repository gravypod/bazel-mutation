#include "third_party/diff_match_patch/diff.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using testing::Eq;

constexpr absl::string_view kOriginal = R"(package com.gravypod.sample;
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

constexpr absl::string_view kResult = R"(package com.gravypod.sample;
public class Main {
  public static void main(String[] args) {
    int a = 1;
    int b = 2;
    if (!(a < b)) {
      return
    }
    System.exit(1);
  }
})";

TEST(DiffTest, CanApplyDiffs) {
  const auto result = diff::Difference::Create(kOriginal, kResult);
  std::string output = result.Apply(kOriginal);
  EXPECT_THAT(output, Eq(kResult));
}
