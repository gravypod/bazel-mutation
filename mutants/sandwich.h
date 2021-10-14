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

#ifndef MUTANTS_MUTANTS_SANDWICH_H_
#define MUTANTS_MUTANTS_SANDWICH_H_

#include "absl/strings/string_view.h"
#include "mutants/mutator.h"

namespace mutants {

struct Sandwich {
  absl::string_view before;
  absl::string_view during;
  absl::string_view after;

  static Sandwich From(absl::string_view source, const SourceRange &range);
};

}

#endif //MUTANTS_MUTANTS_SANDWICH_H_
