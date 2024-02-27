#pragma once

namespace compiler {

struct RegDescSample
{
    struct GPR
    {
        static constexpr auto Size() {
            return 4U;
        }
    };
};

}  // namespace compiler
