#pragma once
#include "../../common.h"
#include "marker.h"
#include "basic_block.h"
#include "analyses/live_intervals.h"

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
    auto GetBlocksMaxId()
    {
        return blocks_.size();
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

    Loop *NewLoop(BasicBlock *header = nullptr);
    void AnalyzeLoops();
    void SetIrreducible()
    {
        reducible_ = false;
    }
    bool IsReducible()
    {
        return reducible_;
    }
    bool IsLoopAnalysisValid();
    void SetRootLoop(Loop *l);
    auto RootLoop()
    {
        return root_loop_;
    }

    void BuildLinearOrder();
    bool IsLinearOrderValid() const;
    const auto &GetLinearOrder()
    {
        return linear_order_;
    }
    auto MaxLifeNumber() const
    {
        return max_life_number_;
    }

    void BuildLiveness();

    void DumpRPO() const;
    void DumpLiveness() const;
    void Dump() const;

    auto NewMarker() { return marker_.NewMarker(); }

private:
    size_t inst_id_{};
    Marker marker_;
    Vector<BasicBlock> blocks_{};
    
    Vector<BasicBlock *> rpo_{};
    
    Vector<BasicBlock *> idoms_{};
    
    size_t loop_id_{};
    Loop *root_loop_{};
    bool reducible_{true};
    
    Vector<BasicBlock *> linear_order_{};
    size_t max_life_number_{};
    UnorderedMap<const Inst *, LiveIntervals> insts_live_intervals_{};
};


}
