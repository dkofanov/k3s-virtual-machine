#pragma once

namespace compiler {

    class LiveRange {
    public:
        LiveRange(const BasicBlock *bb) : LiveRange(GetBlockStart(bb), GetBlockEnd(bb)) {}

        LiveRange(uint32_t b, uint32_t e) : b_{b}, e_{e} { ASSERT(b < e); }

        static uint32_t GetBlockStart(const BasicBlock *bb)
        {
            ASSERT((bb->FirstInst() != nullptr) || (bb->FirstPhi() != nullptr));
            return (bb->FirstInst() != nullptr) ? (bb->FirstInst()->LN() - 2) : bb->FirstPhi()->LN();
        }
        static uint32_t GetBlockEnd(const BasicBlock *bb)
        {
            ASSERT((bb->LastInst() != nullptr) || (bb->LastPhi() != nullptr));
            return (bb->LastInst() != nullptr) ? (bb->LastInst()->LN() + 2) : bb->LastPhi()->LN() + 2;
        }

        void TrimBegin(uint32_t def)
        {
            ASSERT(b_ < def);
            ASSERT(def < e_);
            b_ = def;
        }

        void Prolong(uint32_t end)
        {
            ASSERT(e_ <= end);
            e_ = end;
        }

        bool IntersectsWith(uint32_t b, uint32_t e)
        {
            ASSERT(b < e);
            ASSERT(b_ < e_);
            return !((b_ < e) || (b < e_));
        }

        size_t Begin() const
        {
            return b_;
        }
        size_t End() const
        {
            return e_;
        }

    private:
        uint32_t b_{};
        uint32_t e_{};
    };

   class LiveIntervals {
    public:
        LiveIntervals(const LiveRange &first_range) : intervals_{first_range} {}

        auto GetCurrent()
        {
            return &intervals_.back();
        }
    
        void ExtendActive(uint32_t def)
        {
            GetCurrent()->Prolong(def);
        }

        void AppendAfterHole(const LiveRange &range)
        {
            intervals_.emplace_back(range);
        }

        bool CheckIntervals()
        {
            auto it = intervals_.begin();
            auto end = intervals_.end();
            ASSERT((*it).Begin() < (*it).End());
            while (std::next(it) != end) {
                const auto &range1 = *it;
                const auto &range2 = *std::next(it);
                ASSERT(range1.Begin() < range1.End());
                ASSERT(range2.Begin() < range2.End());
                ASSERT(range1.End() < range2.Begin());
                ++it;
            }
            return true;
        }

        auto Intervals() const
        {
            return intervals_.begin();
        }
        auto IntervalsEnd() const
        {
            return intervals_.end();
        }
    private:
        Vector<LiveRange> intervals_{};
    };

}
