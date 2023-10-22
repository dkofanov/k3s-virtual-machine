#include "basic_block.h"
namespace compiler {

size_t Inst::GetBBId()
{
    return bb_->Id();
}

}