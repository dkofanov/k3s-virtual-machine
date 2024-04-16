#pragma once

#include "../graph.h"
#include "../../../common_compiler.h"

namespace compiler {

class ChecksElimination {
public:
    ChecksElimination(Graph *graph) : graph_(graph)
    {
        for (auto block : graph_->GetRPO()) {
            auto inst = block->FirstInst();
            while (inst != nullptr) {
                auto next = inst->Next();
                TryApply(inst);
                inst = next;
            }
        }
    }
private:
    void TryApply(Inst *inst)
    {
        switch (inst->GetOpcode())
        {
            case Inst::CHECKNULL: {
                Visit<Inst::CHECKNULL>(inst);
                return;
            }
            case Inst::CHECKBOUNDS: {
                Visit<Inst::CHECKBOUNDS>(inst);
                return;
            }
            default: {
                return;
            }

        }
    }
    
    template <Opcode OPC>
    void Visit(Inst *i)
    {
        auto inst = static_cast<InstOpcode<OPC> *>(i);
        ASSERT(inst->GetOpcode() == OPC);
        for (auto i : unique_checks_.Get<OPC>()) {
            auto visited_inst = static_cast<InstOpcode<OPC> *>(i);
            if ((*inst == *visited_inst) && (visited_inst->Dominates(inst))) {
                inst->ReplaceWith(visited_inst);
                inst = nullptr;
                break;
            }
        }

        if (inst != nullptr) {
            unique_checks_.Get<OPC>().push_back(inst);
        }
    }



private:
    OpcodeMap<Vector<Inst *>, Inst::CHECKBOUNDS, Inst::CHECKNULL> unique_checks_;
    Graph *graph_{};
};

}
