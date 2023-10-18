#pragma once
#include <cstdint>

namespace compiler {
    
    class Marker {
    public:
        const auto &NewMarker()
        {
            mark_++;
            return *this;
        }
        void Mark(Marker mark) { mark_ = mark.mark_; }
        bool IsMarked(Marker mark) const { return mark_ == mark.mark_; }
    private:
        uint32_t mark_{};       
    };
}