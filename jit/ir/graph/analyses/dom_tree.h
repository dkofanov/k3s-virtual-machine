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
        ASSERT((blocks_count == blocks_count) && "There are unreachable blocks");
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
                //ASSERT(NumOf(semidom_tmp) < NumOf(bb));
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
        //buckets_[semidom->Id()].push_back(bb);
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

}
