#pragma once
#include "marker.h"
#include "../../common.h"
#include "gen/inst_gen.h"

namespace compiler {

class BasicBlock : public Marker
{
public:
    static constexpr size_t TRUE_SUCC_IDX = 0;
    static constexpr size_t FALSE_SUCC_IDX = 1;

    BasicBlock();
    auto Id() const { return id_; }

    void SetTrueFalseSuccs(BasicBlock *t_succ, BasicBlock *f_succ)
    {
        ASSERT(succs_.size() == 0);
        succs_.resize(2);
        succs_[TRUE_SUCC_IDX] = t_succ;
        succs_[FALSE_SUCC_IDX] = f_succ;
        t_succ->preds_.push_back(this);
        f_succ->preds_.push_back(this);        
    }

    void SetSucc(BasicBlock *succ)
    {
        ASSERT(succs_.size() == 0);
        succs_.push_back(succ);
        succ->preds_.push_back(this);
    }
    const auto &Preds() const
    {
        return preds_;
    }

    const auto &Succs() const
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
        std::cout << "    // Preds: { ";
        for (auto i : preds_) {
            std::cout << "b" << i->Id() << " ";
        }
        std::cout << "}\n";

        auto cur = first_inst_; 
        do {
            cur->Dump();
            cur = cur->Next();
        } while (cur != nullptr);
        
        std::cout << "    // Succs: { ";
        for (auto i : succs_) {
            std::cout << "b" << i->Id() << " ";
        }
        std::cout << "}\n";
    }

    auto FirstInst() { return first_inst_; }

    using PredsT = Vector<BasicBlock *>;
    using SuccsT = Vector<BasicBlock *>;
private:
    // TBD: separate first-Phi last-Phi;
    Inst *first_inst_ {};
    Inst *last_inst_ {};
    PredsT preds_;
    SuccsT succs_;
    size_t id_;
};

}
