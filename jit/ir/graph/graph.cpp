#include "graph.h"
#include "analyses/rpo.h"
#include "analyses/dom_tree.h"
#include "analyses/loop.h"
#include "analyses/linear_order.h"
#include "analyses/liveness.h"

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
    DomTreeCheck(this);
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
    linear_order_ = lo.GetBlocks();
    max_life_number_ = lo.GetLifeNumber();
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
    for (const auto &bb : linear_order_) {
        std::cout << "    BLOCK(b" << bb->Id() << ",\n";
        bb->Dump<true>();
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
        std::cout << "          |";
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

    for (auto &[inst, intervals] : insts_live_intervals_) {
        std::cout << std::setw(9) << "v" << inst->Id() << "|";
        auto interval_it = intervals.Intervals();
        auto interval_it_end = intervals.IntervalsEnd();
        for (size_t i = 0; i < last; i++) {
            if ((interval_it == interval_it_end) || (i < (*interval_it).Begin())) {
                std::cout << " |";
            } else if (i < (*interval_it).End()) {
                std::cout << "-|";
            } else if (i == (*interval_it).End()) {
                std::cout << ">|";
                ++interval_it;
            }
        }
        std::cout << " v" << inst->Id() << "\n";

    }

    std::cout << "*/\n";
    std::cout << ");\n";
}

}
