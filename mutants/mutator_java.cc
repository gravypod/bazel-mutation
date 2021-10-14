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

#include "third_party/tree_sitter/api.h"
#include "third_party/tree_sitter/parser.h"
#include "third_party/tree_sitter/java/java.h"
#include "absl/status/status.h"
#include "mutants/ts.h"
#include "mutants/sandwich.h"
#include "absl/strings/match.h"

#include <vector>

namespace mutants {

bool MutatorJava::CanMutate(const std::filesystem::path &path) const {
  const auto extension = path.extension().string();
  const auto mutate = absl::EndsWith(extension, "java");
  return mutate;
}

Mutation NegateIfStatement(IfStatement statement) {
  return {
      .unary = statement.GetCondition()
          .Expression()
          .GetRange(),
      .condition = SourceCondition::kNegate,
  };
}

std::vector<Mutation> GetMutations(TSTree *tree) {
  auto root = ts_tree_root_node(tree);

  std::vector<Mutation> mutations;

  NodeWrapper(root).Walk([&mutations](TSNode node) {
    const auto if_statement = IfStatement::From(node);
    if (if_statement.ok()) {
      mutations.emplace_back(NegateIfStatement(*if_statement));
    }
  });

  return mutations;
}

std::vector<Mutation> MutatorJava::FindPossibleMutations(absl::string_view source) const {
  auto *parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_java());
  auto *tree = ts_parser_parse_string(parser, nullptr, source.data(), source.size());
  auto mutations = GetMutations(tree);
  ts_tree_delete(tree);
  ts_parser_delete(parser);
  return mutations;
}

std::string MutatorJava::ApplyMutation(absl::string_view source, Mutation mutation) const {
  switch (mutation.condition) {
    case SourceCondition::kNegate:
      const auto sandwich = Sandwich::From(source, mutation.unary);
      return absl::StrCat(sandwich.before, "!(", sandwich.during, ")", sandwich.after);
  }
  return std::string(source);
}

absl::flat_hash_set<diff::Difference> MutatorJava::FindMutations(const std::filesystem::path &file, absl::string_view source) const {
  absl::flat_hash_set<diff::Difference> result;
  if (!CanMutate(file)) {
    return result;
  }
  for (const auto &mutation : FindPossibleMutations(source)) {
    result.emplace(diff::Difference::Create(source, ApplyMutation(source, mutation)));
  }

  return result;
}

}
