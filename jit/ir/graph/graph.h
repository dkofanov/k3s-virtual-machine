#pragma once
#include "../../common.h"
#include "marker.h"
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

    Loop *NewLoop(BasicBlock *header = nullptr);

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
    auto *GetEntryBlock()
    {
        auto *start = GetBlockById(0);
        ASSERT(start->Preds().size() == 0);
        return start;
    }

    void BuildDomTree();
    bool IsDomTreeValid();
    auto IDomOf(const BasicBlock *b) { return idoms_[b->Id()]; }
    bool IsDominator(const BasicBlock *dom, const BasicBlock *block)
    {
        ASSERT(block != nullptr);
        ASSERT(dom != nullptr);
        
        if (dom == block) {
            return true;
        }
        auto idom = IDomOf(block);
        while (block != GetEntryBlock()) {
            if (idom == dom) {
                return true;
            }
            block = idom;
            idom = IDomOf(idom);
        }
        return false;
    }

    void BuildRPO();
    const auto &GetRPO() { return rpo_; }

    void AnalyzeLoops();
    void SetRootLoop(Loop *l);
    auto RootLoop()
    {
        return root_loop_;
    }

    void DumpRPO() const;
    void Dump() const;

    auto NewMarker() { return marker_.NewMarker(); }

private:
    size_t inst_id_{};
    size_t loop_id_{};
    Vector<BasicBlock> blocks_{};
    Vector<BasicBlock *> rpo_{};
    Vector<BasicBlock *> idoms_{};
    Loop *root_loop_{};
    Marker marker_;
};


}
