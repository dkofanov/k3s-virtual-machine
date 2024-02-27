#include "graph.h"
#include "analyses/reg_descr.h"
#include "analyses/rpo.h"
#include "analyses/dom_tree.h"
#include "analyses/loop.h"
#include "analyses/linear_order.h"
#include "analyses/liveness.h"
#include "analyses/reg_alloc.h"

namespace compiler {

Graph *GRAPH;

BasicBlock::BasicBlock() : id_(GRAPH->GetBlocksMaxId()) {}
Inst::Inst(Opcode opcode) : opcode_(opcode), id_(GRAPH->IncInstId()) {}

Loop *Graph::NewLoop(BasicBlock *header)
{
    if (header == nullptr) {
        return new Loop(nullptr, 0);
    }
    return new Loop(header, ++loop_id_);
}

void Graph::BuildDomTree()
{
    idoms_ = DomTree(this).GetBlocks();
#ifndef NDEBUG
    DomTreeCheck(this);
#endif
}

bool Graph::IsDomTreeValid()
{
#ifndef NDEBUG
    volatile DomTreeCheck check(this);
#endif
    return true;
}

void Graph::BuildRPO()
{
    rpo_ = RPO(this).GetBlocks();
}

bool Graph::IsLoopAnalysisValid()
{
    // TODO: Implement properly.
    return root_loop_ != nullptr;
}

void Graph::SetRootLoop(Loop *l)
{
    ASSERT(l == GetEntryBlock()->Loop());
    root_loop_ = l;
}

void Graph::AnalyzeLoops()
{
    LoopAnalyzer(this);
}

void Graph::BuildLinearOrder()
{
    auto lo = LinearOrder(this);
    max_life_number_ = lo.GetLifeNumber();
    linear_order_ = std::move(lo).GetLinearOrder();
}

bool Graph::IsLinearOrderValid() const
{
    // TODO: Implement properly.
    return !linear_order_.empty();
}

void Graph::BuildLiveness()
{
    insts_live_intervals_ = LivenessAnalysis(this).InstsLiveIntervals();
}

void Graph::AllocateRegisters()
{
    RegAlloc<RegDescSample> ra(this);
    slots_used_ = ra.SlotsUsed();
    locations_ = std::move(ra).Locations();
}

void Graph::DumpRPO() const
{
    std::cout << "GRAPH(g0,\n"; \
    for (const auto &bb : rpo_) {
        std::cout << "    BLOCK(b" << bb->Id() << ",\n";
        bb->Dump();
        std::cout << "    );\n";
    }
    std::cout << "/**\n";
    std::cout << " Loops:\n";
    root_loop_->Dump();
    std::cout << "*/\n";
    std::cout << ");\n";
}

void Graph::Dump() const
{
    std::cout << "GRAPH(g0,\n"; \
    for (size_t i = 0; i < blocks_.size(); i++) {
        std::cout << "    BLOCK(b" << blocks_[i].Id() << ",\n";
        blocks_[i].Dump();
        std::cout << "    );\n";
    }
    std::cout << "/**\n";
    std::cout << " Loops:\n";
    root_loop_->Dump();
    std::cout << "*/\n";
    std::cout << ");\n";
}

void Graph::DumpLiveness() const
{
    ASSERT(IsLinearOrderValid());
    std::cout << "GRAPH(g0,\n"; 
    for (auto lo_elem = linear_order_.begin(); lo_elem->block != nullptr; lo_elem++) {
        
        std::cout << "    BLOCK(b" << lo_elem->block->Id() << ",\n";
        lo_elem->block->Dump<true>();
        std::cout << "    );\n";
    }

    std::cout << "/**\nLiveness:\n";

    auto last = max_life_number_;
    auto dig_sel = 10;
    while (dig_sel < last) {
        dig_sel *= 10;
    }
    dig_sel /= 10;
    for (; dig_sel >= 1; dig_sel /= 10) {
        std::cout << std::setw(9        ) << "|";
        for (size_t i = 0; i < last; i++) {
            if ((i / dig_sel != 0) || dig_sel == 1) {
                std::cout << i / dig_sel % 10;
            } else {
                std::cout << " ";
            }
            std::cout << "|";
        }
        std::cout << '\n';
    }

    Vector<const Inst *> sorted_insts;
    sorted_insts.reserve(insts_live_intervals_.size());
    for (auto &[inst, intervals] : insts_live_intervals_) {
        sorted_insts.push_back(inst);
    }
    std::sort(sorted_insts.begin(), sorted_insts.end(), [](const Inst *a, const Inst *b) 
                                                                    {
                                                                        return a->LN() < b->LN();
                                                                    });
    for (auto inst : sorted_insts) {
        const auto &intervals = insts_live_intervals_.at(inst);
        auto interval_it = intervals.Intervals();
        auto interval_it_end = intervals.IntervalsEnd();

        std::cout << std::setw(9) << (std::stringstream() << "v" << inst->Id() << "|").str();
        for (size_t i = 0; i < last; i++) {
            if ((interval_it == interval_it_end) || (i < (*interval_it).Begin())) {
                if (i % 2 == 0) {
                    std::cout << "  ";
                } else {
                    std::cout << " |";
                }
            } else if (i < (*interval_it).End()) {
                if (i % 2 == 0) {
                    std::cout << "--";
                } else {
                    std::cout << "-|";
                }
            } else if (i == (*interval_it).End()) {
                std::cout << "> ";
                ++interval_it;
            }
        }
        std::cout << " v" << inst->Id() << "\n";

        // Dump use-points:
        size_t up_idx = 0;
        std::cout << std::setw(9        ) << "|";
        for (size_t i = 0; i < last; i++) {
            if ((up_idx < intervals.Usepoints().size()) && (intervals.Usepoint(up_idx) == i)) {
                if (i % 2 == 0) {
                    std::cout << "^ ";
                } else {
                    std::cout << "^|";
                }
                up_idx++;
            } else {
                if (i % 2 == 0) {
                    std::cout << "  ";
                } else {
                    std::cout << " |";
                }
            }
        }
        std::cout << "\n";

    }

    std::cout << "*/\n";
    std::cout << ");\n";
}

// Refactor this.
static char LocToChar(Location l)
{
    if (l.type_ == Location::SLOT) {
        return '#';
    }
    ASSERT(l.type_ == Location::REG);
    if (l.idx_ < 10) {
        return '0' + l.idx_;
    }
    ASSERT(l.idx_ < 36);
    return 'A' + l.idx_ - 10;
}

void Graph::DumpRegalloc() const
{
    ASSERT(IsLinearOrderValid());
    std::cout << "/**\nRegalloc:\n";

    Vector<const Inst *> sorted_insts;
    sorted_insts.reserve(insts_live_intervals_.size());
    for (auto &[inst, intervals] : insts_live_intervals_) {
        sorted_insts.push_back(inst);
    }
    std::sort(sorted_insts.begin(), sorted_insts.end(), [](const Inst *a, const Inst *b) 
                                                                    {
                                                                        return a->LN() < b->LN();
                                                                    });
    for (auto inst : sorted_insts) {
        const auto &intervals = insts_live_intervals_.at(inst);
        auto interval_it_end = intervals.IntervalsEnd();
        std::cout << 'v' << inst->Id() << ":\n";
        for (auto it = intervals.Intervals(); it != interval_it_end; it++) {
            auto &range = *it;
            std::cout << "- [" << range.Begin() << ", " << range.End() << "): ";
            const auto &loc = locations_.at(&range);
            if (loc.type_ == Location::REG) { 
                std::cout << 'r' << loc.idx_ << ";\n";
            } else {
                ASSERT(loc.type_ == Location::SLOT);
                std::cout << 's' << loc.idx_ << ";\n";
            }
        }
    }

    auto last = max_life_number_;
    auto dig_sel = 10;
    while (dig_sel < last) {
        dig_sel *= 10;
    }
    dig_sel /= 10;
    for (; dig_sel >= 1; dig_sel /= 10) {
        std::cout << std::setw(9        ) << "|";
        for (size_t i = 0; i < last; i++) {
            if ((i / dig_sel != 0) || dig_sel == 1) {
                std::cout << i / dig_sel % 10;
            } else {
                std::cout << " ";
            }
            std::cout << "|";
        }
        std::cout << '\n';
    }

    for (auto inst : sorted_insts) {
        const auto &intervals = insts_live_intervals_.at(inst);
        auto interval_it = intervals.Intervals();
        auto interval_it_end = intervals.IntervalsEnd();

        std::cout << std::setw(9) << (std::stringstream() << "v" << inst->Id() << "|").str();
        for (size_t i = 0; i < last; i++) {
            if ((interval_it == interval_it_end) || (i < (*interval_it).Begin())) {
                if (i % 2 == 0) {
                    std::cout << "  ";
                } else {
                    std::cout << " |";
                }
            } else if (i < (*interval_it).End()) {
                if (i % 2 == 0) {
                    std::cout << '-' << LocToChar(locations_.at(&(*interval_it)));
                } else {
                    std::cout << "-|";
                }
            } else if (i == (*interval_it).End()) {
                std::cout << '>';
                ++interval_it;
                if ((interval_it != interval_it_end) && ((*interval_it).Begin() == i)) {
                    std::cout << LocToChar(locations_.at(&(*interval_it)));
                } else {
                    std::cout << ' ';
                }
            }
        }
        std::cout << " v" << inst->Id() << "\n";
    }
    std::cout << "*/\n";
}

}
