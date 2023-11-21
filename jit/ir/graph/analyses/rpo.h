#pragma once

#include "../graph.h"

namespace compiler {

class RPO {
public:
    RPO(Graph *graph) : graph_(graph), mark_(graph->NewMarker())
    {
        size_t blocks_count = graph_->GetBlocksCount();
        rpo_.resize(blocks_count);
        DFS(graph_->GetEntryBlock(), &blocks_count);
        ASSERT((blocks_count == 0) && "There are unreachable blocks");
    }
    auto &&GetBlocks()
    {
        return std::move(rpo_);
    }
private:
    bool IsMarked(BasicBlock *bb) { return bb->IsMarked(mark_); }
    void DFS(BasicBlock *bb, size_t *idx)
    {
        ASSERT(bb != nullptr);
        bb->Mark(mark_);
        for (auto *succ : bb->Succs()) {
            if (!IsMarked(succ)) {
                DFS(succ, idx);
            }
        }
        rpo_[--(*idx)] = bb;
    }
    
private:
    Graph *graph_{};
    Vector<BasicBlock *> rpo_;
    Marker mark_;
};

}
