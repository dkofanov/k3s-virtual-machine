#include "../graph.h"

namespace compiler {

class DomTree {
public:
    DomTree(Graph *graph) : graph_(graph), mark_(graph->NewMarker())
    {
        EnumerateBlocks();
        CollectSemiDoms();
        CollectIdoms();
    }
    
    auto &&GetBlocks()
    {
        return std::move(idoms_);
    }
private:
    void EnumerateBlocks()
    {
        size_t blocks_count = graph_->GetBlocksCount();
        v_numbers_.resize(blocks_count);
        sorted_.resize(blocks_count);
        spanning_tree_imm_anc_.resize(blocks_count);
        idoms_.resize(blocks_count);
        size_t idx = 0;
        DFS(graph_->GetEntryBlock(), &idx);

        if (blocks_count != idx) {
            // There are unreachable blocks.
            UNREACHABLE();
        }
        ASSERT(v_numbers_[0] == 1);
    }
    void DFS(BasicBlock *bb, size_t *idx)
    {
        ASSERT(bb != nullptr);
        bb->Mark(mark_);
        ++(*idx);
        v_numbers_[bb->Id()] = *idx;
        sorted_[sorted_.size() - *idx] = bb;
        for (auto *succ : bb->Succs()) {
            if (!IsMarked(succ)) {
                SetAncestor(succ, bb);
                DFS(succ, idx);
            }
        }
    }

    void SetAncestor(BasicBlock *block, BasicBlock *anc)
    {
        spanning_tree_imm_anc_[block->Id()] = anc->Id();
    }
    
    void CollectSemiDoms()
    {
        semidoms_.resize(v_numbers_.size());
    
        for (auto *bb : sorted_) {
            if (graph_->GetEntryBlock() != bb) {
                FindSemiDom(bb);
            }
        }
    }

    BasicBlock *FindSemiDom(BasicBlock *bb)
    {
        ASSERT(graph_->GetEntryBlock() != bb);
        BasicBlock *semidom = nullptr;
        for (const auto &pred : bb->Preds()) {
            BasicBlock *semidom_tmp = nullptr;
            if (NumOf(bb) > NumOf(pred)) {
                semidom_tmp = pred;
            } else {
                mark_ = graph_->NewMarker();
                ASSERT(NumOf(bb) < NumOf(pred));
                semidom_tmp = FindPredWithLessThan(NumOf(bb), pred);
            }
            if (semidom_tmp != nullptr) {
                if ((NumOf(semidom_tmp) < NumOf(semidom)) || (semidom == nullptr)) {
                    ASSERT(NumOf(semidom_tmp) != 0);
                    semidom = semidom_tmp;
                }
            }
        }
        ASSERT(semidom != nullptr);
        semidoms_[bb->Id()] = semidom;
        return semidom;
    }

    BasicBlock *FindPredWithLessThan(size_t num, BasicBlock *bb)
    {
        ASSERT(num <= NumOf(bb));
        bb->Mark(mark_);
        BasicBlock *semidom = nullptr;
        for (const auto &pred : bb->Preds()) {
            BasicBlock *semidom_tmp = nullptr;
            if (num > NumOf(pred)) {
                semidom_tmp = pred;
                ASSERT(!IsMarked(semidom_tmp));
            } else {
                if (!IsMarked(pred)) {
                    ASSERT(num <= NumOf(pred));
                    semidom_tmp = FindPredWithLessThan(num, pred);
                }
            }
            if (semidom_tmp != nullptr) {
                if ((NumOf(semidom_tmp) < NumOf(semidom)) || (semidom == nullptr)) {
                    ASSERT(NumOf(semidom_tmp) != 0);
                    semidom = semidom_tmp;
                }
            }
        }
        return semidom;
    }

    void CollectIdoms()
    {
        idoms_[0] = graph_->GetEntryBlock();
        for (auto bb_it = sorted_.rbegin() + 1; bb_it != sorted_.rend(); ++bb_it) {
            auto bb = *bb_it;
            ASSERT(bb->Id() != 0);
            auto *semidom = semidoms_[bb->Id()];
            ASSERT(semidom != nullptr);
            size_t u = bb->Id();
            size_t u_min = u;
            size_t semidom_id = semidom->Id();
            while (u != semidom_id) {
                if (NumOf(semidoms_[u]) < NumOf(semidoms_[u_min])) {
                    u_min = u;
                }
                ASSERT(u != 0);
                u = spanning_tree_imm_anc_[u];
            }
            if (NumOf(semidoms_[u_min]) == NumOf(semidoms_[bb->Id()])) {
                idoms_[bb->Id()] = semidom;
            } else {
                ASSERT(idoms_[u_min] != nullptr);
                idoms_[bb->Id()] = idoms_[u_min];
            }
        }
    }

    size_t NumOf(size_t bb_id) const { return v_numbers_[bb_id]; }
    size_t NumOf(const BasicBlock *bb) const { return (bb != nullptr) ? v_numbers_[bb->Id()] : 0; }

    bool IsMarked(BasicBlock *bb) { return bb->IsMarked(mark_); }
    
private:
    Graph *graph_{};
    Vector<size_t> v_numbers_;
    Vector<BasicBlock *> sorted_;
    Vector<BasicBlock *> semidoms_;
    Vector<size_t> spanning_tree_imm_anc_;
    Vector<BasicBlock *> idoms_;
    Marker mark_;
};

// This class is intended to check the result of the algorithm.
// It builds DomTree via block-exclusion and check dominators found 
// with the main algorithm.
class DomTreeCheck {
public:
    DomTreeCheck(Graph *graph) : graph_(graph)
    {
        reached_blocks_.resize(graph_->GetBlocksCount());
        all_blocks_.resize(graph_->GetBlocksCount());
        
        CollectBlocks(graph_->GetEntryBlock());
        std::swap(reached_blocks_, all_blocks_);
        
        for (size_t i = 0; i < all_blocks_.size(); i++) {
            ASSERT(all_blocks_[i] != nullptr);
            ASSERT(all_blocks_[i]->Id() == i);
            ignored_block_ = all_blocks_[i];
            CollectBlocks(graph_->GetEntryBlock());
            for (size_t j = 0; j < reached_blocks_.size(); j++) {
                if (reached_blocks_[j] == nullptr) {
                    auto unreached_block = graph_->GetBlockById(j);
                    ASSERT(graph_->IsDominator(ignored_block_, unreached_block));
                }
            }
            reached_blocks_.clear();
            reached_blocks_.resize(all_blocks_.size());
        }
    }
    
    void CollectBlocks(BasicBlock *bb)
    {
        ASSERT(bb != nullptr);

        reached_blocks_[bb->Id()] = bb;

        for (auto *succ : bb->Succs()) {
            if (!IsCollected(succ) && (succ != ignored_block_)) {
                CollectBlocks(succ);
            }
        }
    }
    bool IsCollected(BasicBlock *block)
    {
        ASSERT(block != nullptr);
        if (reached_blocks_[block->Id()] != nullptr) {
            ASSERT(reached_blocks_[block->Id()] == block);
            return true;
        }
        return false;
    }
private:
    Graph *graph_{};
    Vector<BasicBlock *> all_blocks_{};
    Vector<BasicBlock *> reached_blocks_{};
    BasicBlock *ignored_block_{};
};

}
