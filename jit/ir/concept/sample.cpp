#include "inst_ctors_gen.h"
using namespace compiler;

namespace test1 {
    GRAPH(g,
        BLOCK(b_if,
            PARAMETER(Type::FLOAT64) a0 ;
            PARAMETER(Type::FLOAT64) a1 ;
            ADD() s0 {a0, a1};
            //ADD(s0, a0) s1;
            //CMP(s0, s1) cmp0;
            //BR(GT) br
        );
        
        BLOCK(b_true,
            SUB() sub0 {b_if.a0, b_if.a1};
        );

        EDGES(b_if --> b_true);
/*
        BLOCK(b_false,
        {
            MUL(a0, a1) mul0;
        });

        BLOCK(end,
        {
            PHI(b_true.sub0, b_false.mul0) phi0;
        });

        BLOCK(last,
        {
            RET(end.phi0);
        });
        */

        //b_end >> last;
        //b_if >> { b_true, b_false } >> b_end;

    );
    void test1() {
        g.Dump();
    }
}

namespace test2 {
    GRAPH(g0,
        BLOCK(b0,
    // Preds: { }
            PARAMETER           (Type::FLOAT64)              v1;
            PARAMETER           (Type::FLOAT64)              v2;
            ADD                 ()              v3{ v1, v2 };
    // Succs: { 1 }
        );
        BLOCK(b1,
    // Preds: { 0 }
            SUB                 ()              v4{ b0.v1, b0.v2 };
    // Succs: { }
        );
    );
    void test() {
        g0.Dump();
    }
}

int main()
{
    test1::test1();
    test2::test();
    return 0;
}