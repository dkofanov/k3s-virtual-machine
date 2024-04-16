#include "basic_block.h"

namespace compiler {

size_t Inst::GetBBId()
{
    return bb_->Id();
}

bool Inst::Dominates(const Inst *other) const
{
    ASSERT(this != other);
    if (bb_ == other->bb_) {
        while (other != nullptr) {
            if (this == other->prev_) {
                return true;
            }
            other = other->prev_;
        }
        return false;
    }
    return bb_->Dominates(other->bb_);
}

}