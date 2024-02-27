#pragma once
#include "../../common.h"
#include "marker.h"
#include "basic_block.h"
#include "analyses/live_intervals.h"
#include <iterator>
#include <type_traits>
#include <utility>

namespace compiler {

class Graph;

extern Graph *GRAPH;
struct Location {
    enum Type {
        UNDEF,
        REG,
        SLOT,
    };
    Type type_{};
    size_t idx_{};
};

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

    template <typename T, typename... Args>
    T *New(Args&&... args)
    {
        static_assert(std::is_base_of_v<Inst, T>);
        return new T(std::forward<Args>(args)...);
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
    auto &InstsLiveIntervals()
    {
        return insts_live_intervals_;
    }
    
    size_t GetBlockLinearIDX(BasicBlock *bb)
    {
        auto it = std::find_if(linear_order_.begin(), linear_order_.end(), [bb](LinearOrderElement &e){
            return e.block == bb;
        });
        ASSERT(it != linear_order_.end());
        return std::distance(linear_order_.begin(), it);
    }
    uint32_t GetBlockStartLN(size_t block_linear_idx)
    {
        return linear_order_[block_linear_idx].ln;
    }
    uint32_t GetBlockEndLN(size_t block_linear_idx)
    {
        return linear_order_[block_linear_idx + 1].ln;
    }

    void AllocateRegisters();

    void ApplyPeepholes();
    void DumpRPO() const;
    void DumpLiveness() const;
    void DumpRegalloc() const;
    void Dump() const;

    auto NewMarker() { return marker_.NewMarker(); }

    struct LinearOrderElement {
        LinearOrderElement(BasicBlock *b, size_t b_ln) : block(b), ln(b_ln) {}
        BasicBlock *block{};
        size_t ln{};
    };
private:
    size_t inst_id_{};
    Marker marker_;
    Vector<BasicBlock> blocks_{};
    
    Vector<BasicBlock *> rpo_{};
    
    Vector<BasicBlock *> idoms_{};
    
    size_t loop_id_{};
    Loop *root_loop_{};
    bool reducible_{true};
    
    Vector<LinearOrderElement> linear_order_{};
    size_t max_life_number_{};
    UnorderedMap<const Inst *, LiveInterval> insts_live_intervals_{};

    size_t slots_used_{};
    Map<const LiveRange *, Location> locations_{};
};


}
