#include "graph.h"
#include "analyses/rpo.h"
#include "analyses/dom_tree.h"
#include "analyses/loop_analyzer.h"

namespace compiler {

Graph *GRAPH;

BasicBlock::BasicBlock() : id_(GRAPH->GetBlocksCount()) {}
Inst::Inst(Opcode opcode) : opcode_(opcode), id_(GRAPH->IncInstId()) {}

Loop *Graph::NewLoop(BasicBlock *header)
{
    if (header == nullptr) {
        return new Loop(GetEntryBlock(), 0);
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

void Graph::SetRootLoop(Loop *l)
{
    ASSERT(l == GetEntryBlock()->Loop());
    root_loop_ = l;
}

void Graph::AnalyzeLoops()
{
    LoopAnalyzer(this);
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
    std::cout << "GRAPH(g0,\n"; 
    for (size_t i = 0; i < blocks_.size(); i++) {
        std::cout << "    BLOCK(b" << i << ",\n";
        blocks_[i].Dump();
        std::cout << "    );\n";
    }
    std::cout << "/**\n";
    std::cout << " Loops:\n";
    root_loop_->Dump();
    std::cout << "*/\n";
    std::cout << ");\n";
}

}
