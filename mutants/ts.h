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

#ifndef MUTANTS_MUTANTS_TS_H_
#define MUTANTS_MUTANTS_TS_H_

#include <vector>
#include "third_party/tree_sitter/api.h"
#include "absl/strings/string_view.h"
#include "mutants/mutator.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace mutants {

class NodeWrapper {
 public:
  explicit NodeWrapper(TSNode node) : node_(node) {};

  [[nodiscard]] TSNode GetChildByName(absl::string_view field_name) const;
  void Walk(absl::FunctionRef<void(TSNode node)> handler);
  [[nodiscard]] SourceRange GetRange() const;

 protected:
  TSNode node_;
};

class IfCondition : public NodeWrapper {
 public:
  explicit IfCondition(TSNode node) : NodeWrapper(node) {}
  [[nodiscard]] NodeWrapper Expression() const;
};

class IfStatement : public NodeWrapper {
 public:
  explicit IfStatement(TSNode node) : NodeWrapper(node) {}

  static absl::StatusOr<IfStatement> From(TSNode node);

  [[nodiscard]] IfCondition GetCondition() const;
};

}

#endif //MUTANTS_MUTANTS_TS_H_
