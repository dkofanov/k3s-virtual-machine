#pragma once

#include "../graph.h"
#include "live_intervals.h"
#include "reg_descr.h"
#include <cstddef>
#include <cstdint>
#include <regex>
#include <variant>

namespace compiler {

template <typename RegDescr>
class RegAlloc {
public:
    RegAlloc(Graph *graph) : graph_(graph), insts_live_intervals_(graph_->InstsLiveIntervals())
    {
        AllocateRegisters();
        MergePhis();
    }

    void AllocateRegisters()
    {
        for (auto [block, ln] : graph_->GetLinearOrder()) {
            if (block == nullptr) {
                return;
            }
            for (auto inst : block->GetAllInsts()) {
                ExpireOldIntervals(inst->LN());
                FillInputs(inst);
                if (!inst->HasDst()) {
                    continue;
                }
                ASSERT(insts_live_intervals_.contains(inst));
                auto *inst_interval = GetInterval(inst);
                if (active_.size() == REGS_COUNT) {
                    SpillUnusedAndActivate(inst_interval, inst_interval->Startpoint());
                } else {
                    ASSERT(active_.size() < REGS_COUNT);
                    auto free_reg = AllocateFreeReg();
                    Set<Location::REG>(*inst_interval, free_reg);
                    InsertActive(inst_interval);
                }
            }
        }
    }

    auto *GetInterval(Inst *inst)
    {
        return &insts_live_intervals_.at(inst);
    }

    void FillInputs(Inst *inst)
    {
        auto inputs = inst->GetInputs();
        auto ln = inst->LN();
        if (inst->IsPhi()) {
            // Phis' inputs are handled in the separate pass.
            return;
        }
        // TODO: Resolve calling convention here.
        for (auto input : inputs) {
            auto *ii = GetInterval(input);
            if (!IsActive(ii, ln)) {
                if (active_.size() == REGS_COUNT) {
                    // Accuired register should be non-input
                    // (as spilled interval has furthest use):
                    SpillUnusedAndActivate(ii, ln);
                } else {
                    ASSERT(active_.size() < REGS_COUNT);
                    auto free_reg = AllocateFreeReg();
                    auto ir_it = ii->SplitAt(ln - 1);
                    Set<Location::REG>(ir_it, ii->IntervalsEnd(), free_reg);
                    InsertActive(ii);
                }
            }
        }
    }

    bool IsActive(const LiveInterval *interval, [[maybe_unused]] uint32_t ln)
    {
        bool is_reg_loc = CurrentLocation(interval).type_ == Location::REG;
        [[maybe_unused]] bool found_in_active = std::find(active_.begin(), active_.end(), interval) != active_.end();
        [[maybe_unused]] bool is_expiring = interval->IntervalsBack().End() == ln;
        ASSERT((found_in_active == is_reg_loc) || (is_expiring && is_reg_loc && !found_in_active));
        return is_reg_loc;
    }

    void ExpireOldIntervals(uint32_t point)
    {
        // `active_` is sorted by increasing endpoint.
        for (auto j_it = active_.begin(); j_it != active_.end();) {
            if ((*j_it)->Endpoint() > point) {
                break;
            }
            j_it = Recycle(j_it);
        }
    }

    auto Recycle(List<LiveInterval *>::iterator j_it)
    {
        auto range_ptr = &(*j_it)->IntervalsBack();
        auto &loc = locations_.at(range_ptr);
        if (loc.type_ == Location::REG) {
            ASSERT(busy_regs_[loc.idx_]);
            busy_regs_[loc.idx_] = false;
        } else if (loc.type_ == Location::SLOT) {
            ASSERT(busy_slots_[loc.idx_]);
            busy_slots_[loc.idx_] = false;
        }
        return active_.erase(j_it);
    }

    size_t AllocateFreeReg()
    {
        for (size_t i = 0; i < REGS_COUNT; i++) {
            if (!busy_regs_[i]) {
                busy_regs_[i] = true;
                return i;
            }
        }
        UNREACHABLE();
    }

    // 'Unused' i.e has furhtest use from `spill_point`. 
    void SpillUnusedAndActivate(LiveInterval *used_int, uint32_t spill_point)
    {
        /*
         * `spill_point` may be less than `i->Startpoint()` (phi-input from backedge)
         *  as well as greater than `i->Endpoint()` (phi-input from if-else merging).
         */
        auto unused_int_it = GetUnusedIntervalToSpill(spill_point);
        
        auto reg_idx = Get<Location::REG>(*unused_int_it);
        ASSERT(busy_regs_[reg_idx]);

        auto unused_range_it = (*unused_int_it)->SplitAt(spill_point - 1);
        auto used_range_it = used_int->SplitAt(spill_point);

        // Re-assign reg to another interval:
        Set<Location::REG>(used_range_it, used_int->IntervalsEnd(), reg_idx);

        // Spill to slot. Slots are allocated incrementally, there is no reuse:
        Set<Location::SLOT>(unused_range_it, (*unused_int_it)->IntervalsEnd(), slots_used_);
        busy_slots_[slots_used_] = true;
        slots_used_++;

        // Update `active_`:
        active_.erase(unused_int_it);
        InsertActive(used_int);
    }

    auto GetUnusedIntervalToSpill(uint32_t split_point)
    {
        // This should be called when there are no free registers:
        ASSERT(active_.size() == REGS_COUNT);
    
        auto active_interval_it = active_.begin();
        auto furthest_usepoint = 0;
        auto furthest_usepoint_interval_it = active_interval_it;

        for (; active_interval_it != active_.end(); active_interval_it++) {
            auto &usepoints =  (*active_interval_it)->Usepoints();
            auto usepoints_it = usepoints.begin();
            // Remove expired usepoints.
            // Usepoint at `split_point` is considered expired:
            while ((usepoints_it != usepoints.end()) && (*usepoints_it <= split_point)) {
                usepoints_it = usepoints.erase(usepoints_it);
            }

            if (usepoints_it == usepoints.end()) {
                furthest_usepoint_interval_it = active_interval_it;
            } else if (*usepoints_it >= furthest_usepoint) {
                furthest_usepoint_interval_it = active_interval_it;
                furthest_usepoint = *usepoints_it;
            }
        }

        return furthest_usepoint_interval_it;
    }

    void InsertActive(LiveInterval *i)
    {
        auto end = i->Endpoint();
        auto it = std::find_if(active_.rbegin(), active_.rend(), [end](LiveInterval *j){
            return end >= j->Endpoint(); 
        });
        active_.insert(it.base(), i);
    }

    template <Location::Type LOC>
    void Set(const LiveInterval &i, size_t idx)
    {
        Set<LOC>(i.Intervals(), i.IntervalsEnd(), idx);
    }

    template <Location::Type LOC>
    void Set(List<LiveRange>::const_iterator begin, List<LiveRange>::const_iterator end, size_t idx)
    {
        for (auto it = begin; it != end; it++) {
            auto range_ptr = &(*it);
            locations_[range_ptr] = Location{LOC, idx};
        }
    }
    void Set(List<LiveRange>::const_iterator begin, List<LiveRange>::const_iterator end, Location loc)
    {
        for (auto it = begin; it != end; it++) {
            auto range_ptr = &(*it);
            locations_[range_ptr] = loc;
        }
    }
    Location Get(List<LiveRange>::const_iterator it)
    {
        return locations_.at(&(*it));
    }

    template <Location::Type LOC>
    size_t Get(const LiveInterval *i) const 
    {
        ASSERT(CurrentLocation(i).type_ == LOC);
        auto range_ptr = &i->IntervalsBack();
        return locations_.at(range_ptr).idx_;
    }
    
    Location CurrentLocation(const LiveInterval *i) const
    {
        auto range_ptr = &i->IntervalsBack();
        return locations_.at(range_ptr);
    }
    Location FirstLocation(const LiveInterval *i) const
    {
        auto range_ptr = &(*i->Intervals());
        return locations_.at(range_ptr);
    }

    void MergePhis()
    {
        for (auto [block, ln] : graph_->GetLinearOrder()) {
            if (block == nullptr) {
                break;
            }
            for (auto phi : block->GetPhis()) {
                ASSERT(block->Preds().size() == phi->GetInputsCount());
                for (size_t i = 0; i < phi->GetInputsCount(); i++) {
                    auto phi_input = phi->GetInput(i);
                    auto phi_pred = block->Preds()[i];
                    auto phi_first_location = FirstLocation(GetInterval(phi));
                    ProcessPhiInput(phi_input, phi_pred, phi_first_location);
                }
            }
        }
    }
    void ProcessPhiInput(Inst *phi_input, BasicBlock *phi_pred, Location phi_first_location)
    {
        ASSERT(phi_first_location.type_ == Location::REG);
        auto l_idx = graph_->GetBlockLinearIDX(phi_pred);
        auto split_point = graph_->GetBlockEndLN(l_idx);
        auto phi_input_interval = GetInterval(phi_input);
        auto phi_input_new_range_begin = phi_input_interval->SplitAt(split_point - 1);
        if ((*phi_input_new_range_begin).End() == split_point) {
            // Move for expiring range:
            Set<Location::REG>(phi_input_new_range_begin, std::next(phi_input_new_range_begin), phi_first_location.idx_);
        } else {
            // Preserve live range location:
            auto phi_input_new_range_end = phi_input_interval->SplitAt(split_point);
            auto loc = Get(std::prev(phi_input_new_range_begin));
            Set<Location::REG>(phi_input_new_range_begin, phi_input_new_range_end, phi_first_location.idx_);
            Set(phi_input_new_range_end, std::next(phi_input_new_range_end), loc);
        }
    }

    auto SlotsUsed() const
    {
        return slots_used_;
    }
    auto &&Locations() &&
    {
        return std::move(locations_);
    }

private:
    static constexpr auto REGS_COUNT = RegDescr::GPR::Size();
    static constexpr auto SLOTS_COUNT = 255U;
    
private:
    Graph *graph_{};
    decltype(graph_->InstsLiveIntervals()) insts_live_intervals_;
    
    // Use non-const because active interval may be splitted:
    List<LiveInterval *> active_{};

    std::array<bool, REGS_COUNT> busy_regs_{};
    std::array<bool, REGS_COUNT> non_spillable_{};
    std::array<bool, SLOTS_COUNT> busy_slots_{};
    size_t slots_used_{};
    Map<const LiveRange *, Location> locations_{};
};

}
