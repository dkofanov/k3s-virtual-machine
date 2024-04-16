#pragma once
#include "marker.h"
#include "../../common.h"
#include "instructions.h"

namespace compiler {

class Loop;

class BasicBlock : public Marker
{
public:
    static constexpr size_t TRUE_SUCC_IDX = 0;
    static constexpr size_t FALSE_SUCC_IDX = 1;
    
    static constexpr size_t ENTRY_BLOCK_IDX = 1;
    static constexpr size_t END_BLOCK_IDX = 0;

    BasicBlock();
    BasicBlock(Graph *g);

    bool Dominates(const BasicBlock *other) const;

    void SetId(size_t id)
    {
        id_ = id;
    }
    auto Id() const { return id_; }

    auto IsEntryBlock()
    {
        return id_ == ENTRY_BLOCK_IDX;
    }

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

    size_t GetPredIdx(BasicBlock *bb) const
    {
        ASSERT(HasPred(bb));
        auto it = std::find(preds_.begin(), preds_.end(), bb);
        return it - preds_.begin();
    }
    bool HasPred(BasicBlock *bb) const
    {
        return std::find(preds_.begin(), preds_.end(), bb) != preds_.end();
    }

    const auto &Preds() const
    {
        return preds_;
    }

    const auto &Succs() const
    {
        return succs_;
    }

    void SetLoop(Loop *l) { loop_ = l; }
    auto Loop() const { return loop_; }
    bool IsHeader() const;
    // TBD: impl InsertInst();

    void PushPhi(PhiInst *inst)
    {
        ASSERT(inst->BB() == nullptr);
        ASSERT(inst->IsPhi());
        if (IsEmpty<true>()) {
            InitEmptyBlock(inst);
            return;
        }
        ASSERT(last_phi_ != nullptr);
        last_phi_->SetNext(inst);
        inst->SetPrev(last_inst_);
        last_phi_ = inst;
        ASSERT(last_phi_->Next() == nullptr);
        inst->SetBB(this);
    }

    void PushBack(Inst *inst)
    {
        ASSERT(!inst->IsPhi());
        ASSERT(inst->BB() == nullptr);
        if (IsEmpty<false>()) {
            InitEmptyBlock(inst);
            return;
        }
        ASSERT(last_inst_ != nullptr);
        last_inst_->SetNext(inst);
        inst->SetPrev(last_inst_);
        last_inst_ = inst;
        inst->SetBB(this);
    }

    template <bool PHI>
    bool IsEmpty() const
    {
        if constexpr (PHI) {
            ASSERT(!((first_phi_ == nullptr) ^ (last_phi_ == nullptr)));
            return first_phi_ == nullptr;
        } else {
            ASSERT(!((first_inst_ == nullptr) ^ (last_inst_ == nullptr)));
            return first_inst_ == nullptr;
        }
    }

    void InitEmptyBlock(PhiInst *inst)
    {
        ASSERT(inst->BB() == nullptr);
        ASSERT((first_phi_ == nullptr) && (last_phi_ == nullptr));
        first_phi_ = inst;
        last_phi_ = inst;
        inst->SetBB(this);
    }

    void InitEmptyBlock(Inst *inst)
    {
        ASSERT(inst->BB() == nullptr);
        ASSERT(!inst->IsPhi());
        ASSERT((first_inst_ == nullptr) && (last_inst_ == nullptr));
        first_inst_ = inst;
        last_inst_ = inst;
        inst->SetBB(this);
    }

    bool CheckValid() const;
    
    template <bool DUMP_LIVENESS = false>
    void Dump() const;
    auto FirstPhi() const { return first_phi_; }
    auto LastPhi() const { return last_phi_; }
    auto FirstInst() const { return first_inst_; }
    void SetFirstInst(Inst *inst)
    {
        ASSERT((inst == nullptr) || !inst->IsPhi());
        first_inst_ = inst;
    }
    auto LastInst() const { return last_inst_; }
    void SetLastInst(Inst *inst)
    {
        ASSERT((inst == nullptr) || !inst->IsPhi());
        last_inst_ = inst;
    }

    using PredsT = Vector<BasicBlock *>;
    using SuccsT = Vector<BasicBlock *>;

    class PhiIter : public IteratorBase<PhiInst> {
    public:
        PhiIter(const BasicBlock *bb) : IteratorBase(bb->first_phi_) {}
        auto begin()
        {
            return *this;
        }
        void operator++()
        {
            val_ = val_->Next()->AsPhi();
        }
    };

    template <bool REVERSE = false>
    class InstIter : public IteratorBase<Inst> {
    public:
        InstIter(const BasicBlock *bb) : IteratorBase(REVERSE ? bb->last_inst_ : bb->first_inst_) {}
        auto begin()
        {
            return *this;
        }
        void operator++()
        {
            if constexpr (REVERSE) {
                val_ = val_->Prev();
            } else {
                val_ = val_->Next();
            }
        }
    };

    class AllInstIter : public IteratorBase<Inst> {
    public:
        AllInstIter(const BasicBlock *bb) : IteratorBase(bb->first_phi_)
        {
            if (val_ == nullptr) {
                val_ = bb->first_inst_;
            }
        }
        auto begin()
        {
            return *this;
        }
        void operator++()
        {
            ASSERT(val_ != nullptr);
            if ((val_->Next() == nullptr) && val_->IsPhi()) {
                val_ = val_->BB()->first_inst_;
            } else {
                val_ = val_->Next();
            }
        }
    };

    auto GetPhis() const
    {
        return PhiIter(this);
    }

    template <bool REVERSE = false>
    auto GetInsts() const
    {
        return InstIter<REVERSE>(this);
    }

    auto GetAllInsts() const
    {
        return AllInstIter(this);
    }

    void FindAndReplaceInPreds(BasicBlock *find, BasicBlock *replace)
    {
        ASSERT(find != replace);
        auto it = std::find(preds_.begin(), preds_.end(), find);
        ASSERT(it != preds_.end());
        *it = replace;
        ASSERT(std::find(preds_.begin(), preds_.end(), find) == preds_.end());
    }
    
    void MoveSuccsFrom(BasicBlock *bb)
    {
        succs_ = std::move(bb->succs_);
        bb->succs_.clear();
    }

private:
    // TBD: separate first-Phi last-Phi;
    PhiInst *first_phi_{};
    PhiInst *last_phi_{};
    Inst *first_inst_{};
    Inst *last_inst_{};
    class Loop *loop_;
    PredsT preds_;
    SuccsT succs_;
    size_t id_;
};

}
