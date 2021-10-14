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

#include <random>
#include "mutants/sandbox.h"
#include "third_party/random_string/random_string.h"
#include "absl/strings/str_format.h"
#include <sys/mount.h>

namespace mutants {

std::filesystem::path CreateRandomDirectory() {
  return std::filesystem::temp_directory_path() / random_string(8);
}

absl::Status CreateAllDirectories(const std::vector<std::filesystem::path> &paths) {
  for (const auto &path : paths) {
    if (!std::filesystem::create_directories(path)) {
      //return absl::UnknownError("Could not create all directories.");
    }
  }
  return absl::OkStatus();
}

absl::Status Mount(absl::string_view source, absl::string_view target,
                   absl::string_view file_system_type, unsigned long mount_flags,
                   absl::string_view options) {
  int status = mount(
      source.data(),
      target.data(),
      file_system_type.data(),
      mount_flags,
      options.data()
  );
  const auto error = errno;

  if (status == 0) {
    return absl::OkStatus();
  }

  const auto command = absl::StrFormat(
      "mount(%s, %s, %s, %d, %s): ",
      source,
      target,
      file_system_type,
      mount_flags,
      options
  );

  switch (error) {
    case EACCES:
      return absl::PermissionDeniedError(absl::StrCat(command, "Unable to read from source"));
    default:
      // TODO support more error codes in the future.
      break;
  }

  return absl::UnknownError(absl::StrCat(command, "unknown mount error"));
}

absl::Status Unmount(absl::string_view target) {
  int status = umount2(target.data(), MNT_FORCE);
  if (status == 0) {
    return absl::OkStatus();
  }
  if (status == EBUSY) {
    return absl::FailedPreconditionError("Mount is still in use");
  }
  return absl::UnknownError("Failed to unmount.");
}

absl::Status CreateOverlay(
    const std::filesystem::path &lower,
    const std::filesystem::path &upper,
    const std::filesystem::path &work,
    const std::filesystem::path &target
) {
  // Mimics the behavior of the following shell command:
  // mount -t overlay overlay -o lowerdir=$lower,upperdir=$upper,workdir=$work "target"

  const auto options = absl::StrFormat(
      "lowerdir=%s,upperdir=%s,workdir=%s",
      lower, upper, work
  );

  return Mount("overlay", target.string(), "overlay", 0, options);
}

Sandbox::~Sandbox() {
  // Kill bazel daemon
  bazel_.Shutdown();
  if (const auto status = Unmount(sandbox_workspace_root_.c_str());
      !status.ok()) {
    std::cerr << status;
  }
}

absl::StatusOr<std::unique_ptr<Sandbox>> Sandbox::Create(
    const std::filesystem::path &base_workspace,
    const std::vector<Change> &changes
) {
  // Parent temp directory for us to store everything into. We use a single parent so it's
  // easy to execute one `rm -r` on the directory to clean up everything.
  const auto sandbox_root = CreateRandomDirectory();
  // We will write all of the WorkspaceChanges into this directory. This will become the `upperdir`
  // our in overlayfs mount.
  const auto sandbox_changes = sandbox_root / "changes";
  // This will become the `workdir` in the overlayfs mount.
  const auto scratch_space = sandbox_root / "scratch";

  const auto sandbox_workspace_root = sandbox_root / "workspace";

  // Make sure we can create all of our directories.
  if (const auto status = CreateAllDirectories({sandbox_root, sandbox_changes, scratch_space, sandbox_workspace_root}); !status.ok()) {
    return status;
  }

  if (const auto status = WriteChanges(changes, sandbox_changes); !status.ok()) {
    return status;
  }

  if (const auto status = CreateOverlay(base_workspace, sandbox_changes, scratch_space, sandbox_workspace_root); !status.ok()) {
    return status;
  }

  return std::make_unique<Sandbox>(sandbox_root, sandbox_workspace_root);
}

}
