#include "../ir/graph/gen/graph_ctor.h"
#include <cstddef>

using namespace compiler;

namespace hw7_inlining {

GRAPH(caller,
    BLOCK(b0,
        CONST(Type::UINT64, 1) c1;
        CONST(Type::UINT64, 5) c5;
    );

    BLOCK(b1, 
        ADD() p0 {b0.c1, b0.c5};
        MUL() p1 {b0.c1, b0.c5};
        CALL(2) c {p0, p1};
        RETURN(Type::UINT64) r {c};
    );
    
    EDGES(
        b0 --> b1;
    );
);

SUBGRAPH(callee,
    BLOCK(b0,
        PARAMETER(Type::UINT64) p0;
        PARAMETER(Type::UINT64) p1;
        CONST(Type::UINT64, 1) c1;
        CONST(Type::UINT64, 10) c10;
    );

    BLOCK(b1,
        ADD() lhs {b0.p0, b0.c1};
        MUL() rhs {b0.p1, b0.c10};
        JMP(If::GT) j {lhs, rhs};
    );

    BLOCK(s0,
        RETURN(Type::UINT64) r {b0.p1};
    );

    BLOCK(s1,
        RETURN(Type::UINT64) r {b0.p0};
    );
    
    EDGES(
        b0 --> b1 --> (s0, s1);
    );
);

void test_inline_0()
{
    caller.FG();

    caller->SetCalleeInfo(callee);
    caller->ApplyInlining();

    caller->BuildRPO();
    caller->BuildDomTree();
    caller->AnalyzeLoops();
    caller->BuildLinearOrder();
    caller->BuildLiveness();
    caller->DumpLiveness();
    caller->AllocateRegisters();
    caller->DumpRegalloc();

    caller.BG();

/**
    Expected output:

GRAPH(g0,
    BLOCK(b1,
    // Loop: 0
    // Preds: { }
        CONST               (Type::UINT64, 1)              v1;  // LN = 2
        CONST               (Type::UINT64, 5)              v2;  // LN = 4
    // Succs: { b2 }
    );
    BLOCK(b2,
    // Loop: 0
    // Preds: { b1 }
        ADD                 ()              v3{ b1.v1, b1.v2 };  // LN = 8
        MUL                 ()              v4{ b1.v1, b1.v2 };  // LN = 10
    // Succs: { b4 }
    );
    BLOCK(b4,
    // Loop: 0
    // Preds: { b2 }
        CONST               (Type::UINT64, 1)              v9;  // LN = 14
        CONST               (Type::UINT64, 10)              v10;  // LN = 16
    // Succs: { b5 }
    );
    BLOCK(b5,
    // Loop: 0
    // Preds: { b4 }
        ADD                 ()              v11{ b2.v3, b4.v9 };  // LN = 20
        MUL                 ()              v12{ b2.v4, b4.v10 };  // LN = 22
        JMP                 (If::GT)              v13{ v11, v12 };  // LN = 24
    // Succs: { b6 b7 }
    );
    BLOCK(b6,
    // Loop: 0
    // Preds: { b5 }
    // Succs: { b3 }
    );
    BLOCK(b7,
    // Loop: 0
    // Preds: { b5 }
    // Succs: { b3 }
    );
    BLOCK(b3,
    // Loop: 0
    // Preds: { b6 b7 }
        PHI                 ()              v26{ b2.v4, b2.v3 };  // LN = 30
    // Succs: { b8 }
    );
    BLOCK(b8,
    // Loop: 0
    // Preds: { b3 }
        RETURN              (Type::UINT64)              v6{ b3.v26 };  // LN = 34
    // Succs: { b0 }
    );
    BLOCK(b0,
    // Loop: 0
    // Preds: { b8 }
    // Succs: { }
    );

Liveness:
        | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|3|3|3|3|3|3|
        |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|
      v1|   |---|---|---|---|>  |   |   |   |   |   |   |   |   |   |   |   |   |   | v1
        |   |   |   |   |^  |^  |   |   |   |   |   |   |   |   |   |   |   |   |   |
      v2|   |   |---|---|---|>  |   |   |   |   |   |   |   |   |   |   |   |   |   | v2
        |   |   |   |   |^  |^  |   |   |   |   |   |   |   |   |   |   |   |   |   |
      v3|   |   |   |   |---|---|---|---|---|---|---|---|---|>  |---|>  |   |   |   | v3
        |   |   |   |   |   |   |   |   |   |   |^  |   |   |   |   |^  |   |   |   |
      v4|   |   |   |   |   |---|---|---|---|---|---|---|---|---|>  |   |   |   |   | v4
        |   |   |   |   |   |   |   |   |   |   |   |^  |   |   |   |^  |   |   |   |
      v9|   |   |   |   |   |   |   |---|---|---|>  |   |   |   |   |   |   |   |   | v9
        |   |   |   |   |   |   |   |   |   |   |^  |   |   |   |   |   |   |   |   |
     v10|   |   |   |   |   |   |   |   |---|---|---|>  |   |   |   |   |   |   |   | v10
        |   |   |   |   |   |   |   |   |   |   |   |^  |   |   |   |   |   |   |   |
     v11|   |   |   |   |   |   |   |   |   |   |---|---|>  |   |   |   |   |   |   | v11
        |   |   |   |   |   |   |   |   |   |   |   |   |^  |   |   |   |   |   |   |
     v12|   |   |   |   |   |   |   |   |   |   |   |---|>  |   |   |   |   |   |   | v12
        |   |   |   |   |   |   |   |   |   |   |   |   |^  |   |   |   |   |   |   |
     v26|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |---|---|>  |   | v26
        |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |^  |   |

Regalloc:
v1:
- [2, 10): r0;
v2:
- [4, 10): r1;
v3:
- [8, 26): r2;
- [28, 29): r2;
- [29, 30): r0;
v4:
- [10, 27): r0;
- [27, 28): r0;
v9:
- [14, 20): r1;
v10:
- [16, 22): r3;
v11:
- [20, 24): r1;
v12:
- [22, 24): r3;
v26:
- [30, 34): r0;
        | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|3|3|3|3|3|3|
        |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|
      v1|   |-0-|-0-|-0-|-0-|>  |   |   |   |   |   |   |   |   |   |   |   |   |   | v1
      v2|   |   |-1-|-1-|-1-|>  |   |   |   |   |   |   |   |   |   |   |   |   |   | v2
      v3|   |   |   |   |-2-|-2-|-2-|-2-|-2-|-2-|-2-|-2-|-2-|>  |-2>0>  |   |   |   | v3
      v4|   |   |   |   |   |-0-|-0-|-0-|-0-|-0-|-0-|-0-|-0-|-0>0>  |   |   |   |   | v4
      v9|   |   |   |   |   |   |   |-1-|-1-|-1-|>  |   |   |   |   |   |   |   |   | v9
     v10|   |   |   |   |   |   |   |   |-3-|-3-|-3-|>  |   |   |   |   |   |   |   | v10
     v11|   |   |   |   |   |   |   |   |   |   |-1-|-1-|>  |   |   |   |   |   |   | v11
     v12|   |   |   |   |   |   |   |   |   |   |   |-3-|>  |   |   |   |   |   |   | v12
     v26|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |-0-|-0-|>  |   | v26

*/   
}

void test()
{
    test_inline_0();
}

}
