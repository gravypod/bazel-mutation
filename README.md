# Mutant Testing for Bazel

Validate your unit tests fail when your source code is intentionally broken.
This will help you find edge cases that your tests are not covering.

## TL;DR

```
$ bazel build //mutants:mutants_main
...
$ sudo bazel-out/k8-fastbuild/bin/mutants/mutants_main \ # Using sudo so our process can `mount()`
  --workspace_path ./mutants/testdata/simplejava \       # Sample repo with bad tests
  --test_query //...                                     # "Find all tests", can use a more limited scope for big repos
Found 2 results.
Change("java/com/gravypod/basic/Basic.java") [11s]: PASSED (caught mutation) # This mutation was caught by our tests!
	- Workspace: "/tmp/EtCLSkta/workspace"
	- Caught:
		* //java/com/gravypod/basic:BasicTest
	- Missed:
Change("java/com/gravypod/basic/Basic.java") [11s]: FAILED (missed mutation) # This mutation was not :(.
	- Workspace: "/tmp/i8l7PrTD/workspace" # You can check this folder to see what about the files changed
	- Caught:
	- Missed:
		* //java/com/gravypod/basic:BasicTest
```

## What

This tool can be used to validate that tests within a Bazel project
accurately validate the functionality of the code they are testing. Suppose
you have code that looks something like this:

```
class Letter {
  A, B, C, D, F;
  public static Letter getFromPercent(char grade) {
    if (grade > 90) return Letter.A;
    if (grade > 80) return Letter.B;
    if (grade > 70) return Letter.C;
    if (grade > 60) return Letter.D;
    return Letter.F;
  }
}

class LetterTest {
  public void testLetterAssignment() {
    assertEquals(Letter.A, Letter.getFromPercent(100));
    assertEquals(Letter.B, Letter.getFromPercent(85));
    assertEquals(Letter.C, Letter.getFromPercent(75));
    assertEquals(Letter.D, Letter.getFromPercent(65));
    assertEquals(Letter.F, Letter.getFromPercent(0));
  }
}
```

The above example has 100% test coverage. Every branch is exercised
but it is trivial to see where this breaks down: `grade > 90` should
be `grade >= 90`. These tests *run* every line but they *do not*
exercise the critical edge cases that effect the output of the code.
With a mutation testing tool you could quickly identify areas in
your code which do not handle these edge cases. It will do this by
generating a copy of this source code with mutations applied to
each if statement. For example, `if (grade > 90)` will be turned
into the following statements:

1. `if (grade >= 90)`
2. `if (grade < 90)`
3. `if (grade <= 90)`
4. `if (grade == 90)`

We will then execute the tests that are in the same package as the
source file. If all of the tests in that package pass we have identified
a corner case in the software where the behavior is not being asserted.
For the previously mentioned tests the output would look something like
this:

1. `if (grade >= 90)`: FAILED (tests passed, mutation not caught)
2. `if (grade < 90)`: PASSED (tests failed, mutation caught)
3. `if (grade <= 90)`: PASSED (tests failed, mutation caught)
4. `if (grade == 90)`: FAILED (tests passed, mutation not caught)

We can then use this information to identify corner cases we are not
handling in our source code or behavior we intended that is not
asserted within our tests.

## How

### Quering Bazel for Tests

1. Find `*_test`s to break.

   ```
   bazel query "kind(java_test, //...)"
   ```

   This query asks Bazel about where all of the Java tests are located in the
   repository.

2. Look for all of the libraries under test.

   ```
   bazel query "kind(java_library, attr('testonly', 0, deps(set(//java/some:target))))"
   ```

   This looks for all of the non-testonly libraries that are depended on by our
   target. These are deps that we want to mutate to find issues in.

3. Find all of the source files to modify.

   ```
   bazel query "kind('source file', deps(set(//java/some:library)))"
   ```

### Efficient "Sandboxes"

Since a Bazel repository can theoretically be massive, and we don't want to
mutate our code on disk and leave it dirty (may not have been committed yet),
we need to be tricky with how we create a fake workspace to perform experiments
in. We can get "scratch" environments by using temp directories merged with our
original Bazel directory using overlayfs. We can do this by running a command
somewhat like this:

```
temp_dir=$(mktemp -d)

# Create a folder we want to store changes into. Whenever we want to "diff" a
# source file from the repo we just write it into this file path...
diffs=$temp_dir/diffs
mkdir -p "$diffs"

# ....like this!
echo "... if (a == b) ..." > "$diffs/java/com/package/some/Library.java"

# This is required by overlay to have some temp space for house keeping.
work=$temp_dir/work
mkdir -p "$work"

# This is where our "new" bazel workspace will be located.
merged=$temp_dir/merged
mkdir -p "$merged"

# Mount the file system.
mount \
  -t overlay \
  overlay \
  -o lowerdir=/path/to/bazel/src,upperdir=$diffs,workdir=$work \
  "$merged"
```

This can all be done with the mount(2) syscall so it can be controlled
entirely programatically. Overlay file systems also operate at near the
original performance of your file systems so we mimimize performance
implications.

### Mutating Source Code

We use a library called [tree-sitter][] to make it easy to parse, and
explore, the AST of "every" language. We then use the data from the
AST to generate a [diff-match-patch][] that we apply to the "sandbox"
(defined above) before executing Bazel.

## TODO

1. Support other languages (C++, Golang, etc).
2. Improve test coverage using [bazel-integration-test][]
3. Generate sandboxes without `mount()` for rootless execution (slower)
4. Provide an RPC server for performing these tests on repos & pending PRs.


[bazel-integration-test]: https://github.com/bazelbuild/bazel-integration-testing
[tree-sitter]: https://tree-sitter.github.io/tree-sitter/
[diff-match-patch]: https://github.com/google/diff-match-patch
