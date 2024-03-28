#include "../ir/graph/gen/graph_ctor.h"

using namespace compiler;

namespace hw1_hw4_fact {
    GRAPH(test_fact,
        BLOCK(start,
            CONST(Type::INT64, 1) v0;
            CONST(Type::INT64, 2) v1;
            PARAMETER(Type::INT64) v2;
        );
        
        BLOCK(head,
            PHI(2) v0 {start.v0};
            PHI(2) v1 {start.v1};
            JMP(If::GT) j {v1, start.v2};
        );
        
        BLOCK(body,
            MUL() v0 {head.v0, head.v1};
            ADD() v1 {head.v1, start.v0};
        );

        BLOCK(exit,
            RETURN(Type::INT64) r {head.v0}; 
        );

        EDGES(
            start --> head --> (exit, body);
            body --(body.v0, body.v1)--> head;
        );
    );

    void test()
    {
        test_fact.FG();
        test_fact->BuildRPO();
        test_fact->BuildDomTree();
        test_fact->AnalyzeLoops();
        test_fact->BuildLinearOrder();
        auto lo = test_fact->GetLinearOrder();
        ASSERT(lo.size() == 6);
        ASSERT(lo[0].block == test_fact.start);
        ASSERT(lo[1].block == test_fact.head);
        ASSERT(lo[2].block == test_fact.body);
        ASSERT(lo[3].block == test_fact.exit);
        ASSERT(lo[4].block == test_fact->GetEndBlock());
        ASSERT(lo[5].block == nullptr);
        test_fact->BuildLiveness();
        test_fact->DumpLiveness();

//        Dump should produce this:

        test_fact->AllocateRegisters();
        test_fact->DumpRegalloc();

        test_fact.BG();
    }
}