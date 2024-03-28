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
    static constexpr size_t ENTRY_BLOCK_IDX = 1;
    static constexpr size_t END_BLOCK_IDX = 0;

    Graph(bool reset_ids)
    : blocks_id_offset_(reset_ids ? 0 : BLOCK_ID_)
    {
        if (reset_ids) {
            ResetIds();
        }
        // Create end-block:
        NewBB();
    }

    size_t NewBB()
    {
        ASSERT(BlockOffsetById(BLOCK_ID_ + 1) > blocks_.size());
        blocks_.resize(BlockOffsetById(BLOCK_ID_ + 1));
        blocks_[BlockOffsetById(BLOCK_ID_)] = new BasicBlock(this); 
        BLOCK_ID_++;
        return UpperBlockId() - 1;
    }

    template <typename T, typename... Args>
    T *New(Args&&... args)
    {
        static_assert(std::is_base_of_v<Inst, T>);
        return new T(std::forward<Args>(args)...);
    }

    static auto MaxInstId()
    {
        return INST_ID_;
    }

    bool IsSubgraph()
    {
        return blocks_id_offset_ != 0;
    }

    static auto IncInstId()
    {
        INST_ID_++;
        return INST_ID_;
    }
    size_t UpperBlockId()
    {
        return blocks_id_offset_ + blocks_.size();
    }

    const auto &GetBlocks()
    {
        return blocks_;
    }
    auto GetBlocksCount()
    {
        auto nils = std::count(blocks_.begin(), blocks_.end(), nullptr);
        return blocks_.size() - nils;
    }

    auto *GetBlockById()
    {
        return blocks_.back();
    }
    auto *GetBlockById(size_t id)
    {
        return blocks_[BlockOffsetById(id)];
    }
    auto *GetEntryBlock()
    {
        auto *start = blocks_[ENTRY_BLOCK_IDX];
        ASSERT(start->Preds().size() == 0);
        return start;
    }
    auto *GetEndBlock()
    {
        auto *end = blocks_[END_BLOCK_IDX];
        ASSERT(end->Succs().size() == 0);
        return end;
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

    void ApplyInlining(size_t depth = 0);
    // TODO: rework when IRBuilder is implemented:
    void SetCalleeInfo(Graph *callee)
    {
        callee_ = callee;
    }
    Graph *GetCalleeInfo([[maybe_unused]] CallInst *call)
    {
        return callee_;
    }
    void InsertInto(Graph *target)
    {
        // Check that target graph has null-blocks for the corresponding ids (a hole of nullptrs in `blocks_`).
        // It is natural for a sitatuion when after building of a subgraph some blocks are created in `target`-graph.
        // If there was no such creation it is likely bug-prone situation, in which
        // some required landing-blocks weren't created before insertion of the subgraph.
        ASSERT(blocks_id_offset_ > target->blocks_id_offset_);
        ASSERT(UpperBlockId() < target->UpperBlockId());
        auto id_begin = blocks_id_offset_;
        auto id_end = UpperBlockId();
        ASSERT(target->IsValidBlockId(id_begin));
        ASSERT(target->IsValidBlockId(id_end));

        auto nils = std::count_if(blocks_.begin(), blocks_.end(), [](auto b) { return b == nullptr; });
        ASSERT(nils == 0);
       
        auto begin = target->blocks_.begin() + target->BlockOffsetById(id_begin); 
        auto end = target->blocks_.begin() + target->BlockOffsetById(id_end); 
        auto nnils = std::count_if(begin, end, [](auto b) { return b != nullptr; });
        ASSERT(nnils == 0);

        std::copy(blocks_.begin(), blocks_.end(), begin);
    }

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
    bool IsValidBlockId(size_t id)
    {
        return BlockOffsetById(id) < blocks_.size();
    }

    size_t BlockOffsetById(size_t id)
    {
        id -= blocks_id_offset_;
        return id;
    }
    static void ResetIds()
    {
        BLOCK_ID_ = 0;
        INST_ID_ = 0;
    }

private:
    static size_t BLOCK_ID_;
    static size_t INST_ID_;
private:
    Marker marker_;
    const size_t blocks_id_offset_{};
    Vector<BasicBlock *> blocks_{};
    
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

    Graph *callee_{};
};


}
