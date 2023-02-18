
#include "DepfileDependenciesHandler.hpp"
#include <algorithm>
#include <fstream>
#include <streambuf>
#include <teenypath.h>
#include "jet/live/LiveContext.hpp"
#include <regex>

namespace jet
{
    std::unordered_set<std::string> DepfileDependenciesHandler::getDependencies(const LiveContext* context,
        CompilationUnit& cu)
    {
        std::unordered_set<std::string> deps;
        deps.insert(cu.sourceFilePath);

        // Trying deal with "filename.cpp.o.d" vs "filename.cpp.d" depfile names
        if (cu.depFilePath.empty()) {
            TeenyPath::path depfilePath{std::string(cu.objFilePath).append(".d")};
            if (depfilePath.exists()) {
                cu.depFilePath = depfilePath.string();
            }
        }

        if (cu.depFilePath.empty()) {
            auto depfilePath = cu.objFilePath;
            depfilePath.back() = 'd';
            if (TeenyPath::path{depfilePath}.exists()) {
                cu.depFilePath = depfilePath;
            }
        }

        if (cu.depFilePath.empty()) {
            context->events->addLog(LogSeverity::kWarning, "Empty depfile path for cu: " + cu.sourceFilePath);
            return deps;
        }

        std::ifstream f{cu.depFilePath};
        if (!f.is_open()) {
            context->events->addLog(LogSeverity::kWarning, "Cannot open depfile: " + cu.depFilePath);
            return deps;
        }

        // Relative paths expand to this
        TeenyPath::path baseDir{cu.compilationDirStr};

        std::string line;
        std::getline(f, line);

        /*
          The first line is a path to the .o file, eg in cmake:

          main.cpp.o: \
          main.cpp \
          lib1.hpp \
          lib2.hpp \
          lib3.hpp

          But, it can happen that first line contains also dependencies (happens with meson/ninja):

          main.cpp.o: main.cpp lib1.hpp \
          lib2.hpp \
          lib3.hpp
        */
        line = line.substr(line.find(':')+1);

        // the ugliness of \\\\ means essentially a backslash character
        std::regex dependencyRegex("([^\\\\ ]+)");

        do {
          auto begin = std::sregex_iterator(line.begin(), line.end(), dependencyRegex);
          const auto end = std::sregex_iterator();
          for (auto it = begin; it != end; ++it) {
            TeenyPath::path dependencyPath{it->str()};
            if(!dependencyPath.is_absolute()) {
              dependencyPath = (baseDir / dependencyPath);
            }
            dependencyPath = dependencyPath.lexically_normalized();

            std::string absPathStr = dependencyPath.string();
            const auto matchedFilter = std::find_if(context->dirFilters.cbegin(), context->dirFilters.cend(),
                                                    [&absPathStr](const std::string &dirFilter) {
                                                      return (absPathStr.find(dirFilter) != std::string::npos);
                                                    });
            if(matchedFilter == context->dirFilters.cend() && !context->dirFilters.empty()) {
              continue;
            }

            if (dependencyPath.exists()) {
              deps.insert(dependencyPath.string());
            } else {
              context->events->addLog(LogSeverity::kWarning, "Depfile doesn't exist: " + dependencyPath.string());
            }
          }
        } while (std::getline(f, line));

        return deps;
    }
}
