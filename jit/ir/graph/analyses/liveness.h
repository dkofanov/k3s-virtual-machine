#pragma once

#include "loop.h"
#include "linear_order.h"
#include "live_intervals.h"
#include "../graph.h"
#include <cstdint>

namespace compiler {

/**
 * Based on C.Wimmer's "Linear scan register allocation on SSA form".
 */
class LivenessAnalysis {
    class LiveRanges {
    public:
        void Append(uint32_t b, uint32_t e)
        {
            ranges_.emplace_back(b, e);
        }
        void AppendLoopRange(uint32_t b, uint32_t e)
        {
            loop_ranges_.emplace_back(b, e);
        }

        void FinalizeDef(uint32_t def)
        {
            ranges_.back().TrimBegin(def);
        }

        auto LoopRanges()
        {
            return loop_ranges_.rbegin();
        }
        auto LoopRangesEnd()
        {
            return loop_ranges_.rend();
        }

        auto Ranges()
        {
            return ranges_.rbegin();
        }
        auto RangesEnd()
        {
            return ranges_.rend();
        }

        bool CheckRanges()
        {
            auto it = Ranges();
            auto end = RangesEnd();
            ASSERT(it != end);
            while (std::next(it) != end) {
                const auto &range1 = *it;
                const auto &range2 = *std::next(it);
                ASSERT(range1.Begin() < range1.End());
                ASSERT(range2.Begin() < range2.End());
                ASSERT(range1.End() <= range2.Begin());
                ++it;
            }
            auto loop_it = LoopRanges();
            auto loop_end = LoopRangesEnd();
            if (loop_it == loop_end) {
                return true;
            }
            while (std::next(loop_it) != loop_end) {
                const auto &range1 = *loop_it;
                const auto &range2 = *std::next(loop_it);
                ASSERT(range1.Begin() < range1.End());
                ASSERT(range2.Begin() < range2.End());
                // Possible nested loops:
                ASSERT((range1.End() <= range2.Begin()) || ((range1.Begin() <= range2.Begin()) && (range2.End() <= range2.End())));
                ++loop_it;
            }
            return true;
        }

    private:
        // The order is reversed (last range is first):
        Vector<LiveRange> ranges_{};
        // Fix-up for loops:
        Vector<LiveRange> loop_ranges_{};
    };
public:
    auto *GetLiveSet(const BasicBlock *bb)
    {
        return &blocks_live_set_[bb->Id()];
    }

    auto *GetLiveRanges(const Inst *inst)
    {
        return &insts_live_ranges_[inst];
    }

    void AddRange(const Inst *inst, uint32_t b, uint32_t e)
    {
        GetLiveRanges(inst)->Append(b, e);
    }

    class LiveSet {
    public:
        void Extend(const Inst *inst)
        {
            live_insts_.emplace_back(inst);
        }

        bool Has(const Inst *inst) const
        {
            return std::find(live_insts_.begin(), live_insts_.end(), inst) != live_insts_.end();
        }

        void Remove(Inst *inst)
        {
            auto it = std::find(live_insts_.begin(), live_insts_.end(), inst);
            ASSERT(it != live_insts_.end());
            *it = nullptr;
        }

        const auto &LiveInsts()
        {
            return live_insts_;
        }

    private:
        Vector<const Inst *> live_insts_{};
    };

    LivenessAnalysis(Graph *graph) : graph_(graph)
    {  
        ASSERT(graph->IsLinearOrderValid());
        if (graph->IsReducible()) {
            blocks_live_set_.resize(graph->GetBlocksMaxId());
            CalculateLifeRanges();
            FillIntervals();
        }
    }

    void CalculateLifeRanges()
    {
        const auto &lo = graph_->GetLinearOrder();
        for (auto it = lo.rbegin() + 1; it != lo.rend(); it++) {
            InitializeBlock(it->block, it->ln, (it - 1)->ln);
            ProcessInstructions(*it);
            if (it->block->IsHeader()) {
                ProcessLoop(it);
            }
        }
    }

    void InitializeBlock(BasicBlock *block, uint32_t ln_begin, uint32_t ln_end)
    {
        auto *ls = GetLiveSet(block);
        ASSERT(ls->LiveInsts().empty());
        for (auto succ : block->Succs()) {
            for (const auto *inst : GetLiveSet(succ)->LiveInsts()) {
                if (inst != nullptr && !ls->Has(inst)) {
                    ls->Extend(inst);
                    AddRange(inst, ln_begin, ln_end);
                }
            }
            size_t pred_id = succ->GetPredIdx(block);
            for (auto phi : succ->GetPhis()) {
                auto *inst = phi->GetInput(pred_id);
                if (!GetLiveSet(block)->Has(inst)) {
                    ls->Extend(inst);
                    AddRange(inst, ln_begin, ln_end);
                }
            }
        }
    }
    
    void ProcessInstructions(const Graph::LinearOrderElement &lo_elem)
    {
        auto block = lo_elem.block;
        auto block_start_ln = lo_elem.ln;
        for (auto *inst : block->GetInsts<true>()) {
            if (inst->HasDst()) {
                if (inst->HasUsers()) {
                    GetLiveRanges(inst)->FinalizeDef(inst->LN());
                    GetLiveSet(block)->Remove(inst);
                } else {
                    GetLiveRanges(inst)->Append(inst->LN(), inst->LN() + 2);
                    ASSERT(!GetLiveSet(block)->Has(inst));
                }
            } else {
                ASSERT(!GetLiveSet(block)->Has(inst));
            }
            ASSERT(!inst->IsPhi());
            for (auto *input : inst->GetInputs()) {
                if (!GetLiveSet(block)->Has(input)) {
                    GetLiveRanges(input)->Append(block_start_ln, inst->LN());
                    GetLiveSet(block)->Extend(input);
                }
            }
        }
        for (auto *phi : block->GetPhis()) {
            GetLiveSet(block)->Remove(phi);
        }
    }

    void ProcessLoop(Vector<Graph::LinearOrderElement>::const_reverse_iterator rev_it)
    {
        auto header = rev_it->block;
        size_t begin = rev_it->ln;
        auto loop = header->Loop();

        while (std::prev(rev_it)->block->Loop() == loop) {
            --rev_it;
        }
        size_t end = (--rev_it)->ln;
        for (auto *alive_at_header : GetLiveSet(header)->LiveInsts()) {
            if (alive_at_header != nullptr) {
                GetLiveRanges(alive_at_header)->AppendLoopRange(begin, end);
            }
        }
    }

    void FillIntervals()
    {
        for (auto &[inst, ranges] : insts_live_ranges_) {
            ASSERT(ranges.CheckRanges());
            auto range_it = ranges.Ranges();
            ASSERT(!insts_live_intervals_.contains(inst));
            insts_live_intervals_.emplace(inst, *range_it);

            ++range_it;
        
            auto loop_range_it = ranges.LoopRanges();
            auto loop_range_it_end = ranges.LoopRangesEnd();
            auto loop_ln_begin = 0;
            auto loop_ln_end = 0;
            if (loop_range_it != loop_range_it_end) {
                loop_ln_begin = (*loop_range_it).Begin();
                loop_ln_end = (*loop_range_it).End();
            }

            auto range_it_end = ranges.RangesEnd();
            auto *interval = insts_live_intervals_.at(inst).GetCurrent();
            while (true) {
                // case 1. Check loop intersection:
                if ((loop_range_it != loop_range_it_end) && (loop_ln_begin <= interval->End())) {
                    ASSERT(loop_ln_begin > interval->Begin());
                    if (loop_ln_end <= interval->End()) {
                        // The loop range is already covered.
                        ++loop_range_it;
                        if (loop_range_it != loop_range_it_end) {
                            loop_ln_begin = (*loop_range_it).Begin();
                            loop_ln_end = (*loop_range_it).End();
                        }
                        continue;
                    }
                    interval->Prolong(loop_ln_end);
                    
                    // Skip ranges that are already in loop range:
                    while ((range_it != range_it_end) && ((*range_it).End() < loop_ln_end)) {
                        ++range_it;
                    }
                    if ((range_it != range_it_end) && ((*range_it).Begin() <= loop_ln_end)) {
                        interval->Prolong((*range_it).End());
                        ++range_it;
                    }
                    // Skip nested loops ranges:
                    while ((loop_range_it != loop_range_it_end) && ((*loop_range_it).End() < loop_ln_end)) {
                        ASSERT((*loop_range_it).Begin() >= loop_ln_begin);
                        ++loop_range_it;
                    }
                    if (loop_range_it != loop_range_it_end) {
                        loop_ln_begin = (*loop_range_it).Begin();
                        loop_ln_end = (*loop_range_it).End();
                    }
                    continue;
                }
                // case 2. Merge continuous ranges into interval:
                while ((range_it != range_it_end) && (interval->End() == (*range_it).Begin())) {
                    interval->Prolong((*range_it).End());
                    ++range_it;
                }
                if ((loop_range_it != loop_range_it_end) && (loop_ln_begin < interval->End())) {
                    // After merging there is an intersected loop.
                    continue;
                }
                // Begin new interval if there is a hole:
                if (range_it != range_it_end) {
                    ASSERT((*range_it).Begin() > interval->End());
                    ASSERT((loop_range_it == loop_range_it_end) || ((*range_it).Begin() < loop_ln_begin));
                    insts_live_intervals_.at(inst).AppendAfterHole(*range_it);
                    interval = insts_live_intervals_.at(inst).GetCurrent();
                    ++range_it;
                    continue;
                }
                break;
            }
            ASSERT(insts_live_intervals_.at(inst).CheckIntervals());
            FillIntervalUsepoints(inst);
        }
    }

    void FillIntervalUsepoints(const Inst *inst)
    {
        Set<uint32_t> use_points;
        
        auto *user = inst->FirstUser(); 
        while (user != nullptr) {
            use_points.insert(user->GetInst()->LN());
            user = user->GetNext();
        }
        List<uint32_t> use_points_list;
        for (auto i : use_points) {
            use_points_list.emplace_back(i);
        }
        auto &interval = insts_live_intervals_.at(inst);
        interval.SetUsepoints(std::move(use_points_list));
    }

    auto &&InstsLiveIntervals() &&
    {
        return std::move(insts_live_intervals_);
    }

private:
    Graph *graph_{};
    Vector<LiveSet> blocks_live_set_{};
    UnorderedMap<const Inst *, LiveRanges> insts_live_ranges_{};

    // Output:
    UnorderedMap<const Inst *, LiveInterval> insts_live_intervals_{};
};

}
