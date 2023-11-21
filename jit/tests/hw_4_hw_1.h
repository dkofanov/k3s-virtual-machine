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
        ASSERT(lo.size() == 4);
        ASSERT(lo[0] == test_fact.start);
        ASSERT(lo[1] == test_fact.head);
        ASSERT(lo[2] == test_fact.body);
        ASSERT(lo[3] == test_fact.exit);
        test_fact->BuildLiveness();
        test_fact->DumpLiveness();

        /**
        Dump should produce this:
        GRAPH(g0,
            BLOCK(b0,
            // Loop: 0
            // Preds: { }
                CONST               (Type::INT64, 1)              v1;  // LN = 2
                CONST               (Type::INT64, 2)              v2;  // LN = 4
                PARAMETER           (Type::INT64)              v3;  // LN = 6
            // Succs: { b1 }
            );
            BLOCK(b1,
            // Loop: 1
            // Preds: { b0 b2 }
                PHI                 ()              v4{ b0.v1, b2.v7 };  // LN = 8
                PHI                 ()              v5{ b0.v2, b2.v8 };  // LN = 8
                JMP                 (If::GT)              v6{ v5, b0.v3 };  // LN = 10
            // Succs: { b3 b2 }
            );
            BLOCK(b2,
            // Loop: 1
            // Preds: { b1 }
                MUL                 ()              v7{ b1.v4, b1.v5 };  // LN = 14
                ADD                 ()              v8{ b1.v5, b0.v1 };  // LN = 16
            // Succs: { b1 }
            );
            BLOCK(b3,
            // Loop: 0
            // Preds: { b1 }
                RETURN              (Type::INT64)              v9{ b1.v4 };  // LN = 20
            // Succs: { }
            );
    
        Liveness:
          | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|
          |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
        v4| | | | | | | | |-|-|-|-|-|-|>| | | |-|-|>| | v4
        v7| | | | | | | | | | | | | | |-|-|-|-|>| | | | v7
        v8| | | | | | | | | | | | | | | | |-|-|>| | | | v8
        v5| | | | | | | | |-|-|-|-|-|-|-|-|>| | | | | | v5
        v1| | |-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|>| | | | v1
        v3| | | | | | |-|-|-|-|-|-|-|-|-|-|-|-|>| | | | v3
        v2| | | | |-|-|-|-|>| | | | | | | | | | | | | | v2
*/
        test_fact.BG();
    }
}