#pragma once
#include <cstdint>
#include <limits>
#include "../../common.h"

namespace compiler {
    
class Graph;

class Marker {
private:
    const auto &NewMarker()
    {
        ASSERT(mark_ < std::numeric_limits<decltype(mark_)>::max());
        mark_++;
        return *this;
    }
public:
    void Mark(Marker mark) { mark_ = mark.mark_; }
    bool IsMarked(Marker mark) const { return mark_ == mark.mark_; }
private:
    uint32_t mark_{};
friend Graph;    
};

}
