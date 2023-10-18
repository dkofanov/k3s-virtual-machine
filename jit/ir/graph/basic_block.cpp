#include "basic_block.h"

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

}
