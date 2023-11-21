#include "../ir/graph/gen/graph_ctor.h"

using namespace compiler;

namespace hw4_linear_order {
    GRAPH(g1,
        BLOCK(b0, CONST(Type::INT64, 1) dummy; );
        BLOCK(b1, CONST(Type::INT64, 1) dummy; );
        BLOCK(b2, CONST(Type::INT64, 1) dummy; );
        BLOCK(b3, CONST(Type::INT64, 1) dummy; );
        BLOCK(b4, CONST(Type::INT64, 1) dummy; );
        BLOCK(b5, CONST(Type::INT64, 1) dummy; );
        BLOCK(b6, CONST(Type::INT64, 1) dummy; );
        BLOCK(b7, CONST(Type::INT64, 1) dummy; );
        BLOCK(b8, CONST(Type::INT64, 1) dummy; );
        BLOCK(b9, CONST(Type::INT64, 1) dummy; );
        BLOCK(b10, CONST(Type::INT64, 1) dummy; );
        BLOCK(b11, CONST(Type::INT64, 1) dummy; );
        BLOCK(b12, CONST(Type::INT64, 1) dummy; );
        BLOCK(b13, CONST(Type::INT64, 1) dummy; );
        BLOCK(b14, CONST(Type::INT64, 1) dummy; );
        BLOCK(b15, CONST(Type::INT64, 1) dummy; );
    
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
        ASSERT(lo.size() == 16U);
        ASSERT(lo[0]  == g1.b0);
        ASSERT(lo[1]  == g1.b2);
        ASSERT(lo[2]  == g1.b4);
        ASSERT(lo[3]  == g1.b5);
        ASSERT(lo[4]  == g1.b11);
        ASSERT(lo[5]  == g1.b12);
        ASSERT(lo[6]  == g1.b13); 
        ASSERT(lo[7]  == g1.b1);
        ASSERT(lo[8]  == g1.b3);
        ASSERT(lo[9]  == g1.b6);
        ASSERT(lo[10] == g1.b7);
        ASSERT(lo[11] == g1.b8);
        ASSERT(lo[12] == g1.b9);
        ASSERT(lo[13] == g1.b10);
        ASSERT(lo[14] == g1.b14);
        ASSERT(lo[15] == g1.b15);

        g1->BuildLiveness();

        g1->Dump();
        g1.BG();
    }
}
