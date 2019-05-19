// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/torque/source-positions.h"
#include "src/torque/torque-compiler.h"

namespace v8 {
namespace internal {
namespace torque {

int WrappedMain(int argc, const char** argv) {
  std::string output_directory;
  std::vector<std::string> files;

  for (int i = 1; i < argc; ++i) {
    // Check for options
    if (!strcmp("-o", argv[i])) {
      output_directory = argv[++i];
      continue;
    }

    // Otherwise it's a .tq file. Remember it for compilation.
    files.emplace_back(argv[i]);
  }

  TorqueCompilerOptions options;
  options.output_directory = output_directory;
  options.collect_language_server_data = false;
  options.force_assert_statements = false;

  TorqueCompilerResult result = CompileTorque(files, options);

  // PositionAsString requires the SourceFileMap to be set to
  // resolve the file name. Needed to report errors and lint warnings.
  SourceFileMap::Scope source_file_map_scope(result.source_file_map);

  if (result.error) {
    TorqueError& error = *result.error;
    if (error.position) std::cerr << PositionAsString(*error.position) << ": ";
    std::cerr << "Torque error: " << error.message << "\n";
    v8::base::OS::Abort();
  }

  for (const LintError& error : result.lint_errors) {
    std::cerr << PositionAsString(error.position)
              << ": Lint error: " << error.message << "\n";
  }

  if (!result.lint_errors.empty()) v8::base::OS::Abort();

  return 0;
}

}  // namespace torque
}  // namespace internal
}  // namespace v8

int main(int argc, const char** argv) {
  return v8::internal::torque::WrappedMain(argc, argv);
}
