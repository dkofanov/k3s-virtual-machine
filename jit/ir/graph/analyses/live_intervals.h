#pragma once

#include <cstdint>
#include <algorithm>
#include "../../../common.h"


namespace compiler {

    class LiveRange {
    public:
        LiveRange(uint32_t b, uint32_t e) : b_{b}, e_{e} { ASSERT(b < e); }

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

        void SetBegin(size_t b)
        {
            b_ = b;
        }
        void SetEnd(size_t e)
        {
            e_ = e;
        }

    private:
        uint32_t b_{};
        uint32_t e_{};
    };

   class LiveInterval {
    public:
        explicit LiveInterval(const LiveRange &first_range) : intervals_{first_range} {}

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
        auto Intervals()
        {
            return intervals_.begin();
        }

        auto IntervalsEnd() const
        {
            return intervals_.end();
        }
        auto IntervalsEnd()
        {
            return intervals_.end();
        }
        auto Startpoint() const
        {
            return Intervals()->Begin();
        }
        auto &IntervalsBack() const
        {
            return intervals_.back();
        }
        auto Endpoint() const
        {
            return IntervalsBack().End();
        }

        auto FindRangeCoveringOrAfter(uint32_t point)
        {
            // TODO: Handle `ASSERT(Startpoint() <= point)`.  
            ASSERT(Startpoint() <= point);
            if (point >= Endpoint()) {
                intervals_.emplace_back(point, point + 2);
                return --intervals_.end();
            }
            auto r_it = std::find_if(intervals_.begin(), intervals_.end(), [point](LiveRange &r) {
                return (point <= r.Begin()) || (point < r.End());
            });
            return r_it;
        }
        auto SplitAt(uint32_t split_point)
        {
            auto r_it = FindRangeCoveringOrAfter(split_point);
            ASSERT(r_it != intervals_.end());
            auto begin= r_it->Begin();
            auto end = r_it->End();
            if (begin < split_point) {
                r_it->SetEnd(split_point);
                r_it++;
                return intervals_.emplace(r_it, split_point, end);
            } else {
                r_it->SetBegin(split_point);
                return r_it;
            }
        }

        void SetUsepoints(List<uint32_t> &&use_points)
        {
            use_points_ = use_points;
        }
        const auto &Usepoints() const
        {
            return use_points_;
        }
        auto &Usepoints()
        {
            return use_points_;
        }
        auto Usepoint(size_t idx) const
        {
            auto it = use_points_.begin();
            std::advance(it, idx);
            return *it;
        }
    private:
        // Use `List` instead of `Vector` because of range splitting during regalloc:
        List<LiveRange> intervals_{};

        List<uint32_t> use_points_{};
    };

}
