#include "../ir/graph/gen/graph_ctor.h"

using namespace compiler;

namespace hw5_linear_scan {
    GRAPH(g_loop,
        BLOCK(b0,
            CONST(Type::INT64, 1) v0;
            CONST(Type::INT64, 10) v1;
            CONST(Type::INT64, 20) v2;
        );
        BLOCK(b1,
            PHI(2) v3 {b0.v0};
            PHI(2) v4 {b0.v1};
            JMP(If::NE) j {v4, b0.v0};
        );
        BLOCK(b2,
            MUL() v7 {b1.v3, b1.v4};
            SUB() v8 {b1.v4, b0.v0};
        );
        BLOCK(b3, 
            ADD() v9 {b0.v2, b1.v3};
            RETURNVOID() v10 {};
        );

        EDGES(
            b0 --> b1 --> (b2, b3);
            b2 --(b2.v7, b2.v8)--> b1;
        );
    );

    void test1() {
        g_loop.FG();

        g_loop->BuildRPO();
        g_loop->BuildDomTree();
        g_loop->AnalyzeLoops();
        g_loop->BuildLinearOrder();
        const auto &lo = g_loop->GetLinearOrder();
        ASSERT(lo.size() == 5U);
        ASSERT(lo[0].block == g_loop.b0);
        ASSERT(lo[1].block == g_loop.b1);
        ASSERT(lo[2].block == g_loop.b2);
        ASSERT(lo[3].block == g_loop.b3);
        ASSERT(lo[4].block == nullptr);

        g_loop->BuildLiveness();
        g_loop->DumpLiveness();
        g_loop->AllocateRegisters();
        g_loop->DumpRegalloc();
/**
    Dump should produce this:
     
GRAPH(g0,
    BLOCK(b0,
    // Loop: 0
    // Preds: { }
        CONST               (Type::INT64, 1)              v1;  // LN = 2
        CONST               (Type::INT64, 10)              v2;  // LN = 4
        CONST               (Type::INT64, 20)              v3;  // LN = 6
    // Succs: { b1 }
    );
    BLOCK(b1,
    // Loop: 1
    // Preds: { b0 b2 }
        PHI                 ()              v4{ b0.v1, b2.v7 };  // LN = 8
        PHI                 ()              v5{ b0.v2, b2.v8 };  // LN = 8
        JMP                 (If::NE)              v6{ v5, b0.v1 };  // LN = 10
    // Succs: { b2 b3 }
    );
    BLOCK(b2,
    // Loop: 1
    // Preds: { b1 }
        MUL                 ()              v7{ b1.v4, b1.v5 };  // LN = 14
        SUB                 ()              v8{ b1.v5, b0.v1 };  // LN = 16
    // Succs: { b1 }
    );
    BLOCK(b3,
    // Loop: 0
    // Preds: { b1 }
        ADD                 ()              v9{ b0.v3, b1.v4 };  // LN = 20
        RETURNVOID          ()              v10;  // LN = 22
    // Succs: { }
    );
Liveness:
        | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|
        |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|
      v1|   |---|---|---|---|---|---|---|---|>  |   |   | v1
        |   |   |   |   |^  |^  |   |   |^  |   |   |   |
      v2|   |   |---|---|>  |   |   |   |   |   |   |   | v2
        |   |   |   |   |^  |   |   |   |   |   |   |   |
      v3|   |   |   |---|---|---|---|---|---|---|>  |   | v3
        |   |   |   |   |   |   |   |   |   |   |^  |   |
      v4|   |   |   |   |---|---|---|>  |   |---|>  |   | v4
        |   |   |   |   |   |   |   |^  |   |   |^  |   |
      v5|   |   |   |   |---|---|---|---|>  |   |   |   | v5
        |   |   |   |   |   |^  |   |^  |^  |   |   |   |
      v7|   |   |   |   |   |   |   |---|---|>  |   |   | v7
        |   |   |   |   |^  |   |   |   |   |   |   |   |
      v8|   |   |   |   |   |   |   |   |---|>  |   |   | v8
        |   |   |   |   |^  |   |   |   |   |   |   |   |
      v9|   |   |   |   |   |   |   |   |   |   |---|>  | v9
        |   |   |   |   |   |   |   |   |   |   |   |   |
Regalloc:
v1:
- [2, 7): r0;
- [7, 8): r1;
- [8, 18): r0;
v2:
- [4, 7): r1;
- [7, 8): r3;
v3:
- [6, 20): r2;
v4:
- [8, 13): r1;
- [13, 14): s0;
- [18, 19): s0;
- [19, 20): r0;
v5:
- [8, 16): r3;
v7:
- [14, 17): r1;
- [17, 18): r1;
v8:
- [16, 17): r3;
- [17, 18): r3;
v9:
- [20, 22): r1;
        | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|
        |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|
      v1|   |-0-|-0-|-0>1>0-|-0-|-0-|-0-|-0-|>  |   |   | v1
      v2|   |   |-1-|-1>3>  |   |   |   |   |   |   |   | v2
      v3|   |   |   |-2-|-2-|-2-|-2-|-2-|-2-|-2-|>  |   | v3
      v4|   |   |   |   |-1-|-1-|-1>#>  |   |-#>0>  |   | v4
      v5|   |   |   |   |-3-|-3-|-3-|-3-|>  |   |   |   | v5
      v7|   |   |   |   |   |   |   |-1-|-1>1>  |   |   | v7
      v8|   |   |   |   |   |   |   |   |-3>3>  |   |   | v8
      v9|   |   |   |   |   |   |   |   |   |   |-1-|>  | v9
*/
        g_loop.BG();
    }

    GRAPH(g_phiAfterIfElse,
        BLOCK(b0,
            CONST(Type::INT64, 1) v0;
            CONST(Type::INT64, 10) v1;
            CONST(Type::INT64, 20) v2;
        );
        BLOCK(b_if,
            JMP(If::NE) j {b0.v0, b0.v0};
        );
        BLOCK(b_true,
            MUL() v7 {b0.v1, b0.v2};
        );
        BLOCK(b_true_cont,
            MUL() v8 {b0.v1, b0.v1};
            MUL() v9 {b0.v1, b0.v1};
        );
        BLOCK(b_false,
            ADD() v10 {b0.v1, b0.v2};
        );
        BLOCK(b_false_cont,
            ADD() v11 {b0.v1, b0.v1};
            ADD() v12 {b0.v1, b0.v1};
        );
        BLOCK(b_ret, 
            PHI(2) v13 {b_true.v7, b_false.v10};
            RETURN(Type::INT64) r {v13}; 
        );

        EDGES(
            b0 --> b_if --> (b_true, b_false);
            b_true --> b_true_cont;
            b_false --> b_false_cont;
            (b_true_cont, b_false_cont) --> b_ret;
        );
    );

    void test2() {
        g_phiAfterIfElse.FG();

        g_phiAfterIfElse->BuildRPO();
        g_phiAfterIfElse->BuildDomTree();
        g_phiAfterIfElse->AnalyzeLoops();
        g_phiAfterIfElse->BuildLinearOrder();
        const auto &lo = g_phiAfterIfElse->GetLinearOrder();
        ASSERT(lo.size() == 8U);
        ASSERT(lo[0].block == g_phiAfterIfElse.b0);
        ASSERT(lo[1].block == g_phiAfterIfElse.b_if);
        ASSERT(lo[2].block == g_phiAfterIfElse.b_true);
        ASSERT(lo[3].block == g_phiAfterIfElse.b_true_cont);
        ASSERT(lo[4].block == g_phiAfterIfElse.b_false);
        ASSERT(lo[5].block == g_phiAfterIfElse.b_false_cont);
        ASSERT(lo[6].block == g_phiAfterIfElse.b_ret);
        ASSERT(lo[7].block == nullptr);

        g_phiAfterIfElse->BuildLiveness();
        g_phiAfterIfElse->DumpLiveness();
        g_phiAfterIfElse->AllocateRegisters();
        g_phiAfterIfElse->DumpRegalloc();
/**
    Dump should produce this:
     
GRAPH(g0,
    BLOCK(b0,
    // Loop: 0
    // Preds: { }
        CONST               (Type::INT64, 1)              v1;  // LN = 2
        CONST               (Type::INT64, 10)              v2;  // LN = 4
        CONST               (Type::INT64, 20)              v3;  // LN = 6
    // Succs: { b1 }
    );
    BLOCK(b1,
    // Loop: 0
    // Preds: { b0 }
        JMP                 (If::NE)              v4{ b0.v1, b0.v1 };  // LN = 10
    // Succs: { b2 b4 }
    );
    BLOCK(b2,
    // Loop: 0
    // Preds: { b1 }
        MUL                 ()              v5{ b0.v2, b0.v3 };  // LN = 14
    // Succs: { b3 }
    );
    BLOCK(b3,
    // Loop: 0
    // Preds: { b2 }
        MUL                 ()              v6{ b0.v2, b0.v2 };  // LN = 18
        MUL                 ()              v7{ b0.v2, b0.v2 };  // LN = 20
    // Succs: { b6 }
    );
    BLOCK(b4,
    // Loop: 0
    // Preds: { b1 }
        ADD                 ()              v8{ b0.v2, b0.v3 };  // LN = 24
    // Succs: { b5 }
    );
    BLOCK(b5,
    // Loop: 0
    // Preds: { b4 }
        ADD                 ()              v9{ b0.v2, b0.v2 };  // LN = 28
        ADD                 ()              v10{ b0.v2, b0.v2 };  // LN = 30
    // Succs: { b6 }
    );
    BLOCK(b6,
    // Loop: 0
    // Preds: { b3 b5 }
        PHI                 ()              v11{ b2.v5, b4.v8 };  // LN = 32
        RETURN              (Type::INT64)              v12{ v11 };  // LN = 34
    // Succs: { }
    );

Liveness:
        | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|3|3|3|3|
        |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|
      v1|   |---|---|---|---|>  |   |   |   |   |   |   |   |   |   |   |   |   | v1
        |   |   |   |   |   |^  |   |   |   |   |   |   |   |   |   |   |   |   |
      v2|   |   |---|---|---|---|---|---|---|---|>  |---|---|---|---|>  |   |   | v2
        |   |   |   |   |   |   |   |^  |   |^  |^  |   |^  |   |^  |^  |   |   |
      v3|   |   |   |---|---|---|---|>  |   |   |   |---|>  |   |   |   |   |   | v3
        |   |   |   |   |   |   |   |^  |   |   |   |   |^  |   |   |   |   |   |
      v5|   |   |   |   |   |   |   |---|---|---|---|>  |   |   |   |   |   |   | v5
        |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |^  |   |
      v6|   |   |   |   |   |   |   |   |   |---|>  |   |   |   |   |   |   |   | v6
        |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
      v7|   |   |   |   |   |   |   |   |   |   |---|>  |   |   |   |   |   |   | v7
        |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
      v8|   |   |   |   |   |   |   |   |   |   |   |   |---|---|---|---|>  |   | v8
        |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |^  |   |
      v9|   |   |   |   |   |   |   |   |   |   |   |   |   |   |---|>  |   |   | v9
        |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
     v10|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |---|>  |   | v10
        |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
     v11|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |---|>  | v11
        |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |^  |
Regalloc:
v1:
- [2, 10): r0;
v2:
- [4, 20): r1;
- [22, 30): r1;
v3:
- [6, 14): r2;
- [22, 24): r2;
v5:
- [14, 21): r0;
- [21, 22): r0;
v6:
- [18, 20): r3;
v7:
- [20, 22): r3;
v8:
- [24, 31): r0;
- [31, 32): r0;
v9:
- [28, 30): r2;
v10:
- [30, 32): r1;
v11:
- [32, 34): r0;
        | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|3|3|3|3|
        |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|
      v1|   |-0-|-0-|-0-|-0-|>  |   |   |   |   |   |   |   |   |   |   |   |   | v1
      v2|   |   |-1-|-1-|-1-|-1-|-1-|-1-|-1-|-1-|>  |-1-|-1-|-1-|-1-|>  |   |   | v2
      v3|   |   |   |-2-|-2-|-2-|-2-|>  |   |   |   |-2-|>  |   |   |   |   |   | v3
      v5|   |   |   |   |   |   |   |-0-|-0-|-0-|-0>0>  |   |   |   |   |   |   | v5
      v6|   |   |   |   |   |   |   |   |   |-3-|>  |   |   |   |   |   |   |   | v6
      v7|   |   |   |   |   |   |   |   |   |   |-3-|>  |   |   |   |   |   |   | v7
      v8|   |   |   |   |   |   |   |   |   |   |   |   |-0-|-0-|-0-|-0>0>  |   | v8
      v9|   |   |   |   |   |   |   |   |   |   |   |   |   |   |-2-|>  |   |   | v9
     v10|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |-1-|>  |   | v10
     v11|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |-0-|>  | v11
*/
        g_phiAfterIfElse.BG();
    }


GRAPH(g_loopSideExit,
        BLOCK(b0,
            CONST(Type::INT64, 1) v0;
            CONST(Type::INT64, 10) v1;
            CONST(Type::INT64, 20) v2;
        );
        BLOCK(b1,
            PHI(2) v3 {b0.v0};
            PHI(2) v4 {b0.v1};
            JMP(If::NE) j {v4, b0.v0};
        );
        BLOCK(b2,
            MUL() v7 {b1.v3, b1.v4};
            SUB() v8 {b1.v4, b0.v0};
            JMP(If::NE) j {v7, b0.v0};
        );
        BLOCK(b3, 
            ADD() v10 {b0.v2, b1.v3};
            RETURNVOID() v11 {};
        );

        EDGES(
            b0 --> b1 --> (b2, b3);
            b2 --(b2.v7, b2.v8)--> (b1, b3);
        );
    );

    void test3() {
        g_loopSideExit.FG();

        g_loopSideExit->BuildRPO();
        g_loopSideExit->BuildDomTree();
        g_loopSideExit->AnalyzeLoops();
        g_loopSideExit->BuildLinearOrder();
        const auto &lo = g_loopSideExit->GetLinearOrder();
        ASSERT(lo.size() == 5U);
        ASSERT(lo[0].block == g_loopSideExit.b0);
        ASSERT(lo[1].block == g_loopSideExit.b1);
        ASSERT(lo[2].block == g_loopSideExit.b2);
        ASSERT(lo[3].block == g_loopSideExit.b3);
        ASSERT(lo[4].block == nullptr);

        g_loopSideExit->BuildLiveness();
        g_loopSideExit->DumpLiveness();
        g_loopSideExit->AllocateRegisters();
        g_loopSideExit->DumpRegalloc();
/**
    Dump should produce this:

GRAPH(g0,
    BLOCK(b0,
    // Loop: 0
    // Preds: { }
        CONST               (Type::INT64, 1)              v1;  // LN = 2
        CONST               (Type::INT64, 10)              v2;  // LN = 4
        CONST               (Type::INT64, 20)              v3;  // LN = 6
    // Succs: { b1 }
    );
    BLOCK(b1,
    // Loop: 1
    // Preds: { b0 b2 }
        PHI                 ()              v4{ b0.v1, b2.v7 };  // LN = 8
        PHI                 ()              v5{ b0.v2, b2.v8 };  // LN = 8
        JMP                 (If::NE)              v6{ v5, b0.v1 };  // LN = 10
    // Succs: { b2 b3 }
    );
    BLOCK(b2,
    // Loop: 1
    // Preds: { b1 }
        MUL                 ()              v7{ b1.v4, b1.v5 };  // LN = 14
        SUB                 ()              v8{ b1.v5, b0.v1 };  // LN = 16
        JMP                 (If::NE)              v9{ v7, b0.v1 };  // LN = 18
    // Succs: { b1 b3 }
    );
    BLOCK(b3,
    // Loop: 0
    // Preds: { b1 b2 }
        ADD                 ()              v10{ b0.v3, b1.v4 };  // LN = 22
        RETURNVOID          ()              v11;  // LN = 24
    // Succs: { }
    );

Liveness:
        | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|
        |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|
      v1|   |---|---|---|---|---|---|---|---|---|>  |   |   | v1
        |   |   |   |   |^  |^  |   |   |^  |^  |   |   |   |
      v2|   |   |---|---|>  |   |   |   |   |   |   |   |   | v2
        |   |   |   |   |^  |   |   |   |   |   |   |   |   |
      v3|   |   |   |---|---|---|---|---|---|---|---|>  |   | v3
        |   |   |   |   |   |   |   |   |   |   |   |^  |   |
      v4|   |   |   |   |---|---|---|---|---|---|---|>  |   | v4
        |   |   |   |   |   |   |   |^  |   |   |   |^  |   |
      v5|   |   |   |   |---|---|---|---|>  |   |   |   |   | v5
        |   |   |   |   |   |^  |   |^  |^  |   |   |   |   |
      v7|   |   |   |   |   |   |   |---|---|---|>  |   |   | v7
        |   |   |   |   |^  |   |   |   |   |^  |   |   |   |
      v8|   |   |   |   |   |   |   |   |---|---|>  |   |   | v8
        |   |   |   |   |^  |   |   |   |   |   |   |   |   |
     v10|   |   |   |   |   |   |   |   |   |   |   |---|>  | v10
        |   |   |   |   |   |   |   |   |   |   |   |   |   |

Regalloc:
v1:
- [2, 7): r0;
- [7, 8): r1;
- [8, 20): r0;
v2:
- [4, 7): r1;
- [7, 8): r3;
v3:
- [6, 22): r2;
v4:
- [8, 13): r1;
- [13, 21): s0;
- [21, 22): r0;
v5:
- [8, 16): r3;
v7:
- [14, 19): r1;
- [19, 20): r1;
v8:
- [16, 19): r3;
- [19, 20): r3;
v10:
- [22, 24): r1;
        | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|
        |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|
      v1|   |-0-|-0-|-0>1>0-|-0-|-0-|-0-|-0-|-0-|>  |   |   | v1
      v2|   |   |-1-|-1>3>  |   |   |   |   |   |   |   |   | v2
      v3|   |   |   |-2-|-2-|-2-|-2-|-2-|-2-|-2-|-2-|>  |   | v3
      v4|   |   |   |   |-1-|-1-|-1>#-#-|-#-|-#-|-#>0>  |   | v4
      v5|   |   |   |   |-3-|-3-|-3-|-3-|>  |   |   |   |   | v5
      v7|   |   |   |   |   |   |   |-1-|-1-|-1>1>  |   |   | v7
      v8|   |   |   |   |   |   |   |   |-3-|-3>3>  |   |   | v8
     v10|   |   |   |   |   |   |   |   |   |   |   |-1-|>  | v10
*/
        g_loopSideExit.BG();
    }
    void test()
    {
        test1();
        test2();
        test3();
    }
}
