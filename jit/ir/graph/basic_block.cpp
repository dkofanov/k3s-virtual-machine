#include "basic_block.h"
#include "analyses/loop_analyzer.h"

namespace compiler {

bool BasicBlock::CheckValid() const
{
    ASSERT((first_inst_ == nullptr) || (first_inst_ == nullptr));
    auto cur = first_inst_;
    auto last = last_inst_;
        
    bool phi_ended = false;
    do {
        phi_ended |= !cur->IsPhi();
        if (cur->IsPhi()) {
            auto phi = cur->AsPhi();
            ASSERT(!phi_ended && "phi should be at the start only");
            ASSERT(phi->GetInputsCount() == Preds().size());
        }
            
        if (cur->Next() == nullptr) {
            ASSERT(cur == last_inst_);
            break;
        }
        cur = cur->Next();
    } while (true);

       
    ASSERT(Succs().size() <= 2);
    return true;
}

bool BasicBlock::IsHeader() const
{
    return loop_->Header() == this;
}

void BasicBlock::Dump() const
{
    std::cout << "    // Loop: " << loop_->Id() << "\n";
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
}
