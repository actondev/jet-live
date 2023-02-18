#include "jet/live/CompileCommandsCompilationUnitsParser.hpp"
#include <catch.hpp>
#include <jet/live/DepfileDependenciesHandler.hpp>
#include <jet/live/Live.hpp>
#include <jet/live/LiveContext.hpp>
#include <jet/live/Utility.hpp>
#include <teenypath.h>

#include <iostream>

TEST_CASE("compile_commands.json parsing", "[compile_commands.json][fixme]")
{
  using namespace jet;

  const auto projectDir = TeenyPath::path(__FILE__).parent_path() / "assets" / "project-meson-style";

  LiveConfig config;
  config.compileCommandsPath = (projectDir / "meson_style_compile_commands.json").string();
  
  LiveContext ctx;
  ctx.liveConfig = config;
  ctx.events = jet::make_unique<AsyncEventQueue>();

  CompileCommandsCompilationUnitsParser parser;
  auto cunits = parser.parseCompilationUnits(&ctx);

  for(const auto &objCuPair : cunits) {
    const auto &obj = objCuPair.first;
    const auto &cu = objCuPair.second;
    std::cout << "obj " << obj << " cu " << cu.compilationCommandStr << "\n";
  }

  class TestLive : public Live {
    using Live::Live;

   public:
    void doUpdateDependencies(CompilationUnit &cu) {
      updateDependencies(cu);
    }
  };

  TestLive live({}, config);

  for(auto &pair : cunits) {
    auto &cu = pair.second;
    live.doUpdateDependencies(cu);
  }
  /** dependency file path -> set of sourceFilePaths. */
  std::cout << "inverse ...\n";
  for(const auto &[file, paths] : ctx.inverseDependencies) {
    std::cout << "inverse of " << file << "\n";
    for(const auto &path : paths) {
      std::cout << "  path" << path << "\n";
    }
  }
}
