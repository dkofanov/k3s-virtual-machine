#pragma once

#include "./ir/graph/instructions.h"

namespace compiler {

// TODO: implement check whether OPCS are non-repeating
template <typename T, Opcode... OPCS>
class OpcodeMap
{
public:

    template <size_t IDX, Opcode OPC, Opcode ... OPCSS> 
    struct ReenumOpcodes
    {
        template <Opcode TARGET_OPC>
        constexpr static int GetIndex()
        {
            if constexpr (OPC == TARGET_OPC) {
                return IDX;
            } else {
                if constexpr (sizeof...(OPCSS) > 0) {
                    return ReenumOpcodes<IDX + 1, OPCSS...>::template GetIndex<TARGET_OPC>();
                } else {
                    return -1;
                }
            }
        }
    };

    template <Opcode TARGET_OPC>
    auto &Get()
    {
        return type_map_[index_map_.template GetIndex<TARGET_OPC>()];
    }

private:
    ReenumOpcodes<0, OPCS...> index_map_;
    std::array<T, sizeof...(OPCS)> type_map_;
};

}
