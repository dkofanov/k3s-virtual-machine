#pragma once

#include "../graph.h"
#include "../gen/graph_ctor.h"


namespace compiler {



class Inlining {
public:
    static constexpr auto DEPTH_LIMIT = 4U;
    static constexpr auto INSTS_LIMIT = 1024U;

    Inlining(Graph *caller, size_t depth) : graph_(caller), depth_(depth)
    {
        if (Graph::MaxInstId() >= INSTS_LIMIT || depth_ >= DEPTH_LIMIT) {
            return;
        }
        for (auto *b : graph_->GetBlocks()) {
            if (b == nullptr) {
                continue;
            }
            for (auto i : b->GetInsts()) {
                if (i->IsCall()) {
                    Inline(i->AsCall());
                    if (Graph::MaxInstId() >= INSTS_LIMIT) {
                        return;
                    }
                } 
            }
        }
    }

private:
    // TODO: Consider domtree fixup.
    void Inline(CallInst *call)
    {
        call_ = call;
        callee_ = BuildSubgraph();
        callee_->ApplyInlining(depth_++);
        SplitCallBlock();
        FixParametersDF();
        FixReturn();
        FixCF();
    }

    void SplitCallBlock()
    {
        ASSERT(GRAPH == graph_);
        call_block_ = call_->BB();
        cont_block_ = call_->SplitBlockAfter();
    }

    Graph *BuildSubgraph()
    {
        return graph_->GetCalleeInfo(call_);
    }

    void FixParametersDF()
    {
        auto param_idx = 0;
        auto inst = callee_->GetEntryBlock()->FirstInst();
        while (inst != nullptr) {
            auto next = inst->Next();
            if (inst->IsParameter()) {
                inst->ReplaceWith(call_->GetInput(param_idx));
                param_idx++;
            }
            inst = next;
        }
    }

    // TODO: Consider replace with ReturnInlined/CallInlined
    void FixReturn()
    {
        auto eb = callee_->GetEndBlock();
        bool use_ret = call_->HasUsers();
        // TODO: Support graphs with no exit:
        ASSERT(eb->Preds().size() >= 1);
        auto rets_count = eb->Preds().size();

        if (call_->HasUsers()) {
            if (eb->Preds().size() > 1) {
                FixMultipleReturns();
                return;
            }
            FixSingleReturn();
            return;
        }

        // Discard return value if present.
        for (auto pred : eb->Preds()) {
            auto ret = pred->LastInst();
            ASSERT(ret->IsReturn() || ret->IsReturnVoid());
            ret->ReplaceWith(nullptr);
        }
    }

    void FixSingleReturn()
    {
        auto eb = callee_->GetEndBlock();
        ASSERT(eb->Preds().size() == 1);
        ASSERT(eb->Preds().back()->LastInst()->IsReturn());
        auto ret = eb->Preds().back()->LastInst()->AsReturn();
        call_->ReplaceWith(ret->GetInput(0));
        ret->ReplaceWith(nullptr);
    }

    void FixMultipleReturns()
    {
        auto eb = callee_->GetEndBlock();
        auto rets_count = eb->Preds().size();
        PhiInst *phi = graph_->New<PhiInst>(rets_count);
        eb->PushPhi(phi);
        auto ret_counter = 0;
        for (auto pred : eb->Preds()) {
            ASSERT(pred->LastInst()->IsReturn());
            auto ret = pred->LastInst()->AsReturn();
            phi->SetInput(ret_counter, ret->GetInput(0));
            ret->ReplaceWith(nullptr);
            ret_counter++;
        }
        call_->ReplaceWith(phi);
    }

    void FixCF()
    {
        callee_->InsertInto(graph_);
        call_block_->SetSucc(callee_->GetEntryBlock());
        callee_->GetEndBlock()->SetSucc(cont_block_);
    }

private:
    Graph *graph_{};
    size_t depth_{};

    CallInst *call_{}; 
    Graph *callee_{}; 
    BasicBlock *call_block_{}; 
    BasicBlock *cont_block_{}; 
};

}  // namespace compiler