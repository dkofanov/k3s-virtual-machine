#pragma once
#include "common.h"
#include "inst_gen.h"

namespace compiler {

class BasicBlock
{
public:
    static constexpr size_t TRUE_SUCC_IDX = 0;
    static constexpr size_t FALSE_SUCC_IDX = 1;

    BasicBlock();
    auto Id() { return id_; }

    void SetTrueFalseSucc(BasicBlock *t_succ, BasicBlock *f_succ)
    {
        ASSERT(succs_.size() == 0);
        succs_.resize(2);
        succs_[TRUE_SUCC_IDX] = t_succ->Id();
        succs_[FALSE_SUCC_IDX] = f_succ->Id();
        t_succ->preds_.push_back(Id());
        f_succ->preds_.push_back(Id());        
    }

    void SetSucc(BasicBlock *succ)
    {
        ASSERT(succs_.size() == 0);
        succs_.push_back(succ->Id());
        succ->preds_.push_back(Id());
    }
    const auto &GetPreds() const
    {
        return preds_;
    }

    const auto &GetSuccs() const
    {
        return succs_;
    }

    // TBD: impl InsertInst();

    void PushFront(Inst *inst)
    {
        ASSERT(inst->BB() == nullptr);
        ASSERT(inst->IsPhi());
        if (IsEmpty()) {
            InitEmptyBlock(inst);
            return;
        }
        ASSERT(first_inst_ != nullptr);
        inst->SetNext(first_inst_);
        first_inst_ = inst;
        inst->SetBB(this);
    }

    void PushBack(Inst *inst)
    {
        ASSERT(!inst->IsPhi());
        ASSERT(inst->BB() == nullptr);
        if (IsEmpty()) {
            InitEmptyBlock(inst);
            return;
        }
        ASSERT(last_inst_ != nullptr);
        last_inst_->SetNext(inst);
        last_inst_ = inst;
        inst->SetBB(this);
    }

    bool IsEmpty() const
    {
        ASSERT(!((first_inst_ == nullptr) ^ (last_inst_ == nullptr)));
        return first_inst_ == nullptr;
    }

    void InitEmptyBlock(Inst *inst)
    {
        ASSERT(inst->BB() == nullptr);
        ASSERT((first_inst_ == nullptr) && (last_inst_ == nullptr));
        first_inst_ = inst;
        last_inst_ = inst;
        inst->SetBB(this);
    }

    bool CheckValid() const;
    
    void Dump() const
    {
        std::cout << "// Preds: { ";
        for (auto i : preds_) {
            std::cout << i << " ";
        }
        std::cout << "}\n";

        auto cur = first_inst_; 
        do {
            cur->Dump();
            cur = cur->Next();
        } while (cur != nullptr);
        
        std::cout << "// Succs: { ";
        for (auto i : succs_) {
            std::cout << i << " ";
        }
        std::cout << "}\n";
    }

    using Preds = Vector<size_t>;
    using Succs = Vector<size_t>;
private:
    // TBD: separate first-Phi last-Phi;
    Inst *first_inst_ {};
    Inst *last_inst_ {};
    Preds preds_;
    Succs succs_;
    size_t id_;
};

}
