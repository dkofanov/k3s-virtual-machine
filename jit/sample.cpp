#include "ir/graph/gen/inst_ctors_gen.h"

using namespace compiler;

namespace test1
{
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
            SUB() sub {b_if.a0, b_if.a1};
        );

        BLOCK(b_false,
            SUB() sub { b_if.a1, b_if.a0};
        );
    
        BLOCK(last,
            SUB() sub { b_false.sub, b_true.sub};
        );

        EDGES(
            b_if --> (b_true, b_false) --> last --> b_if;
        );
    );


    void test1() {
        g.Dump();
    }
}

namespace test_fact {
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
        test_fact.Dump();
    }
}


namespace test_doms {
    GRAPH(g1,
        BLOCK(A, CONST(Type::INT64, 1) dummy; );
        BLOCK(B, CONST(Type::INT64, 1) dummy; );
        BLOCK(C, CONST(Type::INT64, 1) dummy; );
        BLOCK(D, CONST(Type::INT64, 1) dummy; );
        BLOCK(E, CONST(Type::INT64, 1) dummy; );
        BLOCK(F, CONST(Type::INT64, 1) dummy; );
        BLOCK(G, CONST(Type::INT64, 1) dummy; );
        EDGES(
            A --> B --> (C, F);
            F --> (E, G);
            (C, E, G) --> D;
        );
    );
    void test1()
    {
        g1.FG();
        g1->BuildRPO();
        auto rpo = g1->GetRPO();
        ASSERT(rpo.size() == 7U);
        ASSERT(rpo[0] == g1.A);
        ASSERT(rpo[1] == g1.B);
        ASSERT(rpo[2] == g1.F);
        ASSERT(rpo[3] == g1.G);
        ASSERT(rpo[4] == g1.E);
        ASSERT(rpo[5] == g1.C);
        ASSERT(rpo[6] == g1.D);

        g1->BuildDomTree();
        ASSERT(g1->IDomOf(g1.A) == g1.A);
        ASSERT(g1->IDomOf(g1.B) == g1.A);
        ASSERT(g1->IDomOf(g1.C) == g1.B);
        ASSERT(g1->IDomOf(g1.D) == g1.B);
        ASSERT(g1->IDomOf(g1.E) == g1.F);
        ASSERT(g1->IDomOf(g1.F) == g1.B);
        ASSERT(g1->IDomOf(g1.G) == g1.F);
        g1.BG();
    }

    GRAPH(g2,
        BLOCK(A, CONST(Type::INT64, 1) dummy; );
        BLOCK(B, CONST(Type::INT64, 1) dummy; );
        BLOCK(C, CONST(Type::INT64, 1) dummy; );
        BLOCK(D, CONST(Type::INT64, 1) dummy; );
        BLOCK(E, CONST(Type::INT64, 1) dummy; );
        BLOCK(F, CONST(Type::INT64, 1) dummy; );
        BLOCK(G, CONST(Type::INT64, 1) dummy; );
        BLOCK(H, CONST(Type::INT64, 1) dummy; );
        BLOCK(I, CONST(Type::INT64, 1) dummy; );
        BLOCK(J, CONST(Type::INT64, 1) dummy; );
        BLOCK(K, CONST(Type::INT64, 1) dummy; );
        EDGES(
            A --> B --> (C, J);
            J --> C --> D --> (E, C);
            E --> F --> (G, E);
            G --> (H, I);
            H --> B;
            I --> K;
        );
    );
    void test2()
    {
        g2.FG();

        g2->BuildRPO();
        auto rpo = g2->GetRPO();
        ASSERT(rpo.size() == 11U);
        ASSERT(rpo[0] == g2.A);
        ASSERT(rpo[1] == g2.B);
        ASSERT(rpo[2] == g2.J);
        ASSERT(rpo[3] == g2.C);
        ASSERT(rpo[4] == g2.D);
        ASSERT(rpo[5] == g2.E);
        ASSERT(rpo[6] == g2.F);
        ASSERT(rpo[7] == g2.G);
        ASSERT(rpo[8] == g2.I);
        ASSERT(rpo[9] == g2.K);
        ASSERT(rpo[10] == g2.H);

        g2->BuildDomTree();
        ASSERT(g2->IDomOf(g2.A) == g2.A);
        ASSERT(g2->IDomOf(g2.B) == g2.A);
        ASSERT(g2->IDomOf(g2.C) == g2.B);
        ASSERT(g2->IDomOf(g2.D) == g2.C);
        ASSERT(g2->IDomOf(g2.E) == g2.D);
        ASSERT(g2->IDomOf(g2.F) == g2.E);
        ASSERT(g2->IDomOf(g2.G) == g2.F);
        ASSERT(g2->IDomOf(g2.H) == g2.G);
        ASSERT(g2->IDomOf(g2.I) == g2.G);
        ASSERT(g2->IDomOf(g2.J) == g2.B);
        ASSERT(g2->IDomOf(g2.K) == g2.I);

        g2.BG();
    }
    GRAPH(g3,
        BLOCK(A, CONST(Type::INT64, 1) dummy; );
        BLOCK(B, CONST(Type::INT64, 1) dummy; );
        BLOCK(C, CONST(Type::INT64, 1) dummy; );
        BLOCK(D, CONST(Type::INT64, 1) dummy; );
        BLOCK(E, CONST(Type::INT64, 1) dummy; );
        BLOCK(F, CONST(Type::INT64, 1) dummy; );
        BLOCK(G, CONST(Type::INT64, 1) dummy; );
        BLOCK(H, CONST(Type::INT64, 1) dummy; );
        BLOCK(I, CONST(Type::INT64, 1) dummy; );
        EDGES(
            A --> B --> (E, C);
            C --> D --> G --> (I, C);
            E --> (F, D);
            F --> (B, H);
            H --> (G, I);
        );
    );

    void test3()
    {
        g3.FG();
        g3->BuildRPO();
        auto rpo = g3->GetRPO();
        ASSERT(rpo.size() == 9U);
        ASSERT(rpo[0] == g3.A);
        ASSERT(rpo[1] == g3.B);
        ASSERT(rpo[2] == g3.E);
        ASSERT(rpo[3] == g3.F);
        ASSERT(rpo[4] == g3.H);
        ASSERT(rpo[5] == g3.G);
        ASSERT(rpo[6] == g3.C);
        ASSERT(rpo[7] == g3.D);
        ASSERT(rpo[8] == g3.I);

        g3->BuildDomTree();
        ASSERT(g3->IDomOf(g3.A) == g3.A);
        ASSERT(g3->IDomOf(g3.B) == g3.A);
        ASSERT(g3->IDomOf(g3.C) == g3.B);
        ASSERT(g3->IDomOf(g3.D) == g3.B);
        ASSERT(g3->IDomOf(g3.E) == g3.B);
        ASSERT(g3->IDomOf(g3.F) == g3.E);
        ASSERT(g3->IDomOf(g3.G) == g3.B);
        ASSERT(g3->IDomOf(g3.H) == g3.F);
        ASSERT(g3->IDomOf(g3.I) == g3.B);
        g3.BG();
    }

    void test() {
        test1();
        test2();
        test3();
    }
}

int main()
{
    test1::test1();
    test_fact::test();
    test_doms::test();
    return 0;
}
