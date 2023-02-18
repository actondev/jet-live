#include <catch.hpp>
#include <jet/live/DepfileDependenciesHandler.hpp>
#include <jet/live/Live.hpp>
#include <jet/live/LiveContext.hpp>
#include <jet/live/Utility.hpp>
#include <teenypath.h>

TEST_CASE("cmake & meson/ninja .o.d dependency file parsing", "[dependencies]")
{
  using namespace jet;

  LiveContext ctx;
  ctx.events = jet::make_unique<AsyncEventQueue>();

  const auto projectDir = TeenyPath::path(__FILE__).parent_path() / "assets" / "project-meson-style";
  ctx.dirFilters.insert(projectDir.string());
  jet::DepfileDependenciesHandler depsHandler;

  jet::CompilationUnit cuCmake;
  cuCmake.compilationDirStr = (projectDir / "build").string();
  cuCmake.sourceFilePath = (projectDir / "src/mylib.cpp").string();
  cuCmake.depFilePath = (projectDir / "build/mylib.p/cmake_style_src_mylib.cpp.o.d").string();

  const auto depsCmake = depsHandler.getDependencies(&ctx, cuCmake);

  // NB: in the .o.d file, there's an absolute path (not matching the filter),
  // and a non existent ../subprojects/foo/enums.hpp entry.
  // That leaves 2 valid dependencies: mylib.cpp and Live.hpp
  REQUIRE(depsCmake.size() == 2);

  auto cuMeson = cuCmake;
  cuCmake.depFilePath = (projectDir / "build/mylib.p/meson_style_src_mylib.cpp.o.d").string();

  const auto depsMeson = depsHandler.getDependencies(&ctx, cuMeson);

  REQUIRE(depsCmake == depsMeson);

  // Removin the directory filter for the dependency watching
  // NB: this might be fragile, /usr/include/stdc-predef.h should exist in the machine where this test runs
  ctx.dirFilters = {};
  const auto depsCmakeNoFilter = depsHandler.getDependencies(&ctx, cuCmake);
  REQUIRE(depsCmakeNoFilter.size() == 3);
  const auto depsMesonNoFilter = depsHandler.getDependencies(&ctx, cuMeson);
  REQUIRE(depsMesonNoFilter.size() == 3);
}
