#pragma once

#include "../graph.h"

namespace compiler {

class LinearOrder {
public:

    LinearOrder(Graph *graph) : graph_(graph), mark_(graph->NewMarker())
    {
        ASSERT(graph_->IsLoopAnalysisValid());

        size_t blocks_count = graph_->GetBlocksCount();
        linear_order_.reserve(blocks_count);
        DFS(graph_->GetEntryBlock());
        // emplace number for the last block:
        linear_order_.emplace_back(nullptr, life_number_);
        ASSERT((blocks_count == linear_order_.size() - 1) && "There are unreachable blocks");
    }
    
    auto GetLinearOrder() &&
    {
        return std::move(linear_order_);
    }

    auto GetLifeNumber() const
    {
        return life_number_;
    }

private:
    bool IsMarked(const BasicBlock *bb) { return bb->IsMarked(mark_); }
    
    void DFS(BasicBlock *bb)
    {
        ASSERT(bb != nullptr);
        if (!AllSidePredsMarked(bb)) {
            return;
        }
        bb->Mark(mark_);
        AssignLifeNumbers(bb);

        auto loop = bb->Loop();
        auto loop_header = bb->Loop()->Header();
        auto loop_outer = bb->Loop()->Outer();

        ASSERT((graph_->RootLoop() == loop) || IsMarked(loop_header) || !loop->Reducible());

        for (auto *succ : bb->Succs()) {
            if (succ->Loop() != loop_outer) {
                if (!IsMarked(succ)) {
                    DFS(succ);
                }
            }
        }
        ASSERT(AllBackEdgesMarked(loop));
        for (auto *succ : bb->Succs()) {
            if (succ->Loop() == loop_outer) {
                if (!IsMarked(succ)) {
                    DFS(succ);
                }
            }
        }
    }

    bool AllSidePredsMarked(BasicBlock *bb)
    {
        if (!bb->IsHeader()) {
            for (const auto *pred : bb->Preds()) {
                if (!IsMarked(pred)) {
                    return false;
                }
            }
        } else {
            for (const auto *pred : bb->Preds()) {
                if ((pred->Loop() != bb->Loop()) && !IsMarked(pred)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool AllBackEdgesMarked(Loop *l)
    {
        for (const auto *be : l->BackEdges()) {
            if (!IsMarked(be)) {
                return false; 
            }
        }
        return true;
    }

    void AssignLifeNumbers(BasicBlock *bb)
    {
        linear_order_.emplace_back(bb, life_number_);
        for (auto phi : bb->GetPhis()) {
            phi->SetLifeNumber(life_number_);
        }
        life_number_ += 2;
        for (auto inst : bb->GetInsts()) {
            inst->SetLifeNumber(life_number_);
            life_number_ += 2;
        }
    }

private:
    Graph *graph_{};
    Vector<Graph::LinearOrderElement> linear_order_;
    Marker mark_;
    size_t life_number_{};
};

}
