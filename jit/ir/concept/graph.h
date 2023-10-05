#pragma once
#include "common.h"
#include "basic_block.h"

namespace compiler {


class Graph;

extern Graph *GRAPH;

class Graph {
public:
    Graph()
    {
        // tmp solution for avoiding relocations.
        // TBD: replace to vector of pointers.
        blocks_.reserve(100);
    }

    auto NewBB()
    {
        blocks_.emplace_back();
        return blocks_.size() - 1;
    }
    auto IncInstId()
    {
        inst_id_++;
        return inst_id_;
    }
    auto GetBlocksCount()
    {
        return blocks_.size();
    }
    auto *GetBlockById()
    {
        return &blocks_[blocks_.size() - 1];
    }
    auto *GetBlockById(size_t id)
    {
        ASSERT(id < blocks_.size());
        return &blocks_[id];
    }

    void Dump() const
    {
        std::cout << "GRAPH(g0,\n"; 
        for (size_t i = 0; i < blocks_.size(); i++) {
            std::cout << "    BLOCK(b" << i << ",\n";
            blocks_[i].Dump();
            std::cout << "    );\n";
        }
        std::cout << ");\n";
    }

private:
    size_t inst_id_{};
    Vector<BasicBlock> blocks_{};
};


}
