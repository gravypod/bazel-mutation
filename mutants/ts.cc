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

#include "mutants/ts.h"
namespace mutants {

TSNode NodeWrapper::GetChildByName(absl::string_view field_name) const {
  return ts_node_child_by_field_name(node_, field_name.data(), field_name.size());
}

SourceRange NodeWrapper::GetRange() const {
  return {
      .start = ts_node_start_byte(node_),
      .stop = ts_node_end_byte(node_),
  };
}
void NodeWrapper::Walk(absl::FunctionRef<void(TSNode)> handler) {
  for (uint32_t i = 0; i < ts_node_child_count(node_); i++) {
    auto child = ts_node_child(node_, i);
    handler(child);
    NodeWrapper(child).Walk(handler);
  }
}

IfCondition IfStatement::GetCondition() const {
  return IfCondition(GetChildByName("condition"));
}

absl::StatusOr<IfStatement> IfStatement::From(TSNode node) {
  if (absl::string_view(ts_node_type(node)) != "if_statement") {
    return absl::InvalidArgumentError("Argument was not an if statement.");
  }
  return IfStatement(node);
}

NodeWrapper IfCondition::Expression() const {
  return NodeWrapper(ts_node_child(node_, 1));
}

}