#include "../ir/graph/gen/graph_ctor.h"

using namespace compiler;

namespace hw4_linear_order {
    GRAPH(g1,
        BLOCK(b0, );
        BLOCK(b1, RETURNVOID() r;);
        BLOCK(b2, );
        BLOCK(b3, );
        BLOCK(b4, );
        BLOCK(b5, );
        BLOCK(b6, );
        BLOCK(b7, );
        BLOCK(b8, );
        BLOCK(b9, );
        BLOCK(b10, );
        BLOCK(b11, );
        BLOCK(b12, );
        BLOCK(b13, );
        BLOCK(b14, );
        BLOCK(b15, );
    
        EDGES(
            b0 --> b2 --> (b4, b3);
            // l1:
            b4 --> (b5, b3);
            b5 --> b11 --> (b12, b13);
            b12 --> b4;
            // l1 side exit:
            b13 --> b1;

            // l2:
            b3 --> b6 --> b7 --> b8 --> (b14, b9);
            b14 --> b15 --> b3;

            // l3:
            b9 --> b10 --> b6;
        );
    );

    void test() {
        g1.FG();

        g1->BuildRPO();
        g1->BuildDomTree();
        g1->AnalyzeLoops();
        g1->BuildLinearOrder();
        const auto &lo = g1->GetLinearOrder();
        ASSERT(lo.size() == 18U);
        ASSERT(lo[0].block == g1.b0);
        ASSERT(lo[1].block == g1.b2);
        ASSERT(lo[2].block == g1.b4);
        ASSERT(lo[3].block == g1.b5);
        ASSERT(lo[4].block == g1.b11);
        ASSERT(lo[5].block == g1.b12);
        ASSERT(lo[6].block == g1.b13);
        ASSERT(lo[7].block == g1.b1);
        ASSERT(lo[8].block == g1->GetEndBlock());
        ASSERT(lo[9].block == g1.b3);
        ASSERT(lo[10].block == g1.b6);
        ASSERT(lo[11].block == g1.b7);
        ASSERT(lo[12].block == g1.b8);
        ASSERT(lo[13].block == g1.b9);
        ASSERT(lo[14].block == g1.b10);
        ASSERT(lo[15].block == g1.b14);
        ASSERT(lo[16].block == g1.b15);
        ASSERT(lo[17].block == nullptr);

        g1->BuildLiveness();

        g1->Dump();
        g1.BG();
    }
}
