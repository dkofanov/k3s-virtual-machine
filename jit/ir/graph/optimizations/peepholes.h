#pragma once

#include "../graph.h"

namespace compiler {

class Peepholes {
public:
    Peepholes(Graph *graph) : graph_(graph)
    {
        for (auto block : graph_->GetRPO()) {
            for (auto inst : block->GetInsts()) {
                TryApply(inst);
            }
        }
    }
private:
    void TryApply(Inst *inst)
    {
        switch (inst->GetOpcode())
        {
            case Inst::XOR: {
                Visit<Inst::XOR>(graph_, inst->AsXor());
                return;
            }
            case Inst::SUB: {
                Visit<Inst::SUB>(graph_, inst->AsSub());
                return;
            }
            case Inst::ASHR: {
                Visit<Inst::ASHR>(graph_, inst->AsAshr());
                return;
            }
            default: {
                return;
            }

        }
    }

    template <Inst::Opcode OPC>
    class Visit {};

    template <>
    class Visit<Inst::SUB>
    {
    public:
        Visit(Graph *g, SubInst *inst) : graph_(g), inst_(inst)
        {
            auto i0 = inst_->GetInput(0);
            auto i1 = inst_->GetInput(1);

            if (ConstFold(i0, i1)) {
                return;
            }

            if (i0->Equal(i1)) {
                inst_->ReplaceWith(graph_->New<ConstInst>(inst_->GetType(), 0));
                return;
            }

            if (SubNeg(i0, i1)) {
                return;
            }

            if (SubZero(i0, i1)) {
                return;
            }
        }

    private:
        bool SubNeg(Inst *i0, Inst *i1)
        {
            if (i1->IsNeg()) {
                auto inst_add = graph_->New<AddInst>();
                inst_add->SetType(inst_->GetType());
                auto new_i0 = i0;
                auto new_i1 = i1->AsNeg()->GetInput(0);
                inst_->ReplaceWith(inst_add);
                inst_add->SetInput(0, new_i0);
                inst_add->SetInput(1, new_i1);
                return true;
            }
            if (i0->IsNeg()) {
                auto inst_add = graph_->New<AddInst>();
                auto inst_neg = graph_->New<NegInst>();
                inst_add->SetType(inst_->GetType());
                inst_neg->SetType(inst_->GetType());

                auto new_i0 = i0->AsNeg()->GetInput(0);
                auto new_i1 = i1;
                inst_->ReplaceWith(inst_neg);
                inst_neg->Prepend(inst_add);
                inst_add->SetInput(0, new_i0);
                inst_add->SetInput(1, new_i1);
                inst_neg->SetInput(0, inst_add);
                return true;   
            }
            return false;
        }

        bool SubZero(Inst *i0, Inst *i1)
        {
            if (i1->IsConst() && (i1->AsConst()->GetImm() == 0)) {
                inst_->ReplaceWith(i0);
                return true;
            }
            if (i0->IsConst() && (i0->AsConst()->GetImm() == 0)) {
                auto inst_neg = graph_->New<NegInst>();
                inst_->ReplaceWith(inst_neg);
                inst_neg->SetInput(0, i1);
                return true;
            }
            return false;
        }

        bool ConstFold(Inst *i0, Inst *i1)
        {
            if (i0->IsConst() && i1->IsConst()) {
                auto new_const = i0->AsConst()->GetImm() - i1->AsConst()->GetImm();
                inst_->ReplaceWith(graph_->New<ConstInst>(inst_->GetType(), new_const));
                return true;
            }
            return false;
        }

    private:
        Graph *graph_ {};
        SubInst *inst_ {};
    };

    template <>
    class Visit<Inst::XOR>
    {
    public:
        Visit(Graph *g, XorInst *inst) : graph_(g), inst_(inst)
        {
            auto i0 = inst_->GetInput(0);
            auto i1 = inst_->GetInput(1);
            
            if (ConstFold(i1, i0)) {
                return;
            }

            if (i0->Equal(i1)) {
                inst_->ReplaceWith(graph_->New<ConstInst>(inst_->GetType(), 0));
                return;
            }

            if (XorNot(i0, i1)) {
                return;
            }

            if (XorNot(i1, i0)) {
                return;
            }

            if (XorZero(i0, i1)) {
                return;
            }
        }

    private:
        bool ConstFold(Inst *i0, Inst *i1)
        {
            if (i0->IsConst() && i1->IsConst()) {
                auto new_const = i0->AsConst()->GetImm() ^ i1->AsConst()->GetImm();
                inst_->ReplaceWith(graph_->New<ConstInst>(inst_->GetType(), new_const));
                return true;
            }
            return false;
        }

        bool XorNot(Inst *i0, Inst *i1)
        {
            if (i0->IsNot() && i0->AsNot()->GetInput(0)->Equal(i1)) {
                inst_->ReplaceWith(graph_->New<ConstInst>(inst_->GetType(), 0xFFFF'FFFF'FFFF'FFFFU));
                return true;
            }
            return false;
        }

        bool XorZero(Inst *i0, Inst *i1)
        {
            if (i0->IsConst() && (i0->AsConst()->GetImm() == 0)) {
                inst_->ReplaceWith(i1);
                return true;
            }
            return false;
        }
        
    private:
        Graph *graph_ {};
        XorInst *inst_ {};
    };

    template <>
    class Visit<Inst::ASHR>
    {
    public:
        Visit(Graph *g, AshrInst *inst) : graph_(g), inst_(inst)
        {
            auto i0 = inst_->GetInput(0);
            auto i1 = inst_->GetInput(1);
            
            if (ConstFold(i0, i1)) {
                return;
            }

            if (AshrZero(i0, i1)) {
                return;
            }

            if (Cast(i0, i1)) {
                return;
            }
        }

    private:
        bool ConstFold(Inst *i0, Inst *i1)
        {
            if (i0->IsConst() && i1->IsConst()) {
                ASSERT(i0->AsConst()->GetSizeInBits() >= i1->AsConst()->GetImm());
                
                auto new_const = std::bit_cast<int64_t>(i0->AsConst()->GetImm()) >> i1->AsConst()->GetImm();
                inst_->ReplaceWith(graph_->New<ConstInst>(inst_->GetType(), new_const));
                return true;
            }
            return false;
        }

        bool AshrZero(Inst *i0, Inst *i1)
        {
            if (i1->IsConst() && (i1->AsConst()->GetImm() == 0)) {
                inst_->ReplaceWith(i0);
                return true;
            }
            return false;
        }

        bool Cast(Inst *i0, Inst *i1)
        {
            if (!i1->IsConst() || !i0->IsShl() || !i0->AsShl()->GetInput(1)->Equal(i1)) {
                return false;
            }

            auto shift = i1->AsConst()->GetImm();

            auto cast = graph_->New<CastInst>(inst_->GetDownCastType(shift));
            inst_->ReplaceWith(cast);
            cast->SetInput<0>( i0->AsShl()->GetInput(0));
            return true;
        }
        
    private:
        Graph *graph_ {};
        AshrInst *inst_ {};
    };

private:
    Graph *graph_{};
};

}
