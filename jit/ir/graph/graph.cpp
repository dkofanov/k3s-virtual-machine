#include "graph.h"
#include "analyses/rpo.h"
#include "analyses/dom_tree.h"

namespace compiler {

Graph *GRAPH;

BasicBlock::BasicBlock() : id_(GRAPH->GetBlocksCount()) {}
Inst::Inst(Opcode opcode) : opcode_(opcode), id_(GRAPH->IncInstId()) {}

void Graph::BuildDomTree()
{
    idoms_ = DomTree(this).GetBlocks();
}
void Graph::BuildRPO()
{
    rpo_ = RPO(this).GetBlocks();
}
void Graph::DumpRPO() const
{
    std::cout << "GRAPH(g0,\n"; \
    for (const auto &bb : rpo_) {
        std::cout << "    BLOCK(b" << bb->Id() << ",\n";
        bb->Dump();
        std::cout << "    );\n";
    }
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
    std::cout << ");\n";
}

}
