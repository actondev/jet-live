
#pragma once

#include <string>
#include <vector>

namespace jet
{
    /**
     * Configuration parameters.
     */
    struct LiveConfig
    {
        /**
         * The maximum amount of possible worker threads used by the library.
         * Usually all these threads are busy compiling new code.
         */
        size_t workerThreadsCount = 4;

        /**
         * If `true`, also reload code when app receives `SIGUSR1`.
         */
        bool reloadOnSignal = true;

        /**
         * Where to search for the `compile_commands.json`.
         * If empty or not found, will try to find it in the parent directory of the running binary.
         */
         std::string compileCommandsPath;
    };
}
