#include "../ir/graph/gen/graph_ctor.h"
#include <cstddef>

using namespace compiler;

namespace hw2_hw3_doms_and_loops {
    GRAPH(g1,
        BLOCK(A, CONST(Type::INT64, 1) dummy; );
        BLOCK(B, CONST(Type::INT64, 1) dummy; );
        BLOCK(C, CONST(Type::INT64, 1) dummy; );
        BLOCK(D, RETURNVOID() dummy; );
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
        ASSERT(rpo.size() == 8U);
        ASSERT(rpo[0] == g1.A);
        ASSERT(rpo[1] == g1.B);
        ASSERT(rpo[2] == g1.F);
        ASSERT(rpo[3] == g1.G);
        ASSERT(rpo[4] == g1.E);
        ASSERT(rpo[5] == g1.C);
        ASSERT(rpo[6] == g1.D);
        ASSERT(rpo[7] == g1->GetEndBlock());

        g1->BuildDomTree();
        ASSERT(g1->IDomOf(g1.A) == g1.A);
        ASSERT(g1->IDomOf(g1.B) == g1.A);
        ASSERT(g1->IDomOf(g1.C) == g1.B);
        ASSERT(g1->IDomOf(g1.D) == g1.B);
        ASSERT(g1->IDomOf(g1.E) == g1.F);
        ASSERT(g1->IDomOf(g1.F) == g1.B);
        ASSERT(g1->IDomOf(g1.G) == g1.F);

        g1->AnalyzeLoops();
        g1->Dump();
        auto loop_0 = g1.A->Loop();
        ASSERT(loop_0->Header() == nullptr);
        ASSERT(loop_0->BackEdges().size() == 0);
        ASSERT(loop_0->Reducible());

        ASSERT(g1.A->Loop() == loop_0);
        ASSERT(g1.B->Loop() == loop_0);
        ASSERT(g1.C->Loop() == loop_0);
        ASSERT(g1.D->Loop() == loop_0);
        ASSERT(g1.E->Loop() == loop_0);
        ASSERT(g1.F->Loop() == loop_0);
        ASSERT(g1.G->Loop() == loop_0);

        g1->BuildLinearOrder();
        auto lo = g1->GetLinearOrder();
        ASSERT(lo.size() == 9U);
        ASSERT(lo[0].block == g1.A);
        ASSERT(lo[1].block == g1.B);
        ASSERT(lo[2].block == g1.C);
        ASSERT(lo[3].block == g1.F);
        ASSERT(lo[4].block == g1.E);
        ASSERT(lo[5].block == g1.G);
        ASSERT(lo[6].block == g1.D);
        ASSERT(lo[7].block == g1->GetEndBlock());
        ASSERT(lo[8].block == nullptr);
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
        BLOCK(K, RETURNVOID() dummy; );
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
        std::cout << "test2" << std::endl;
        g2.FG();

        g2->BuildRPO();
        auto rpo = g2->GetRPO();
        ASSERT(rpo.size() == 12U);
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
        ASSERT(rpo[10] == g2->GetEndBlock());
        ASSERT(rpo[11] == g2.H);

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
        g2->AnalyzeLoops();
        g2->Dump();

        auto loop_0 = g2.A->Loop();
        ASSERT(loop_0->Header() == nullptr);
        ASSERT(loop_0->BackEdges().size() == 0);
        ASSERT(loop_0->Reducible());
        auto loop_1 = g2.B->Loop();
        ASSERT(loop_1->Header() == g2.B);
        ASSERT(loop_1->BackEdges().size() == 1);
        ASSERT(loop_1->BackEdges()[0] == g2.H);
        ASSERT(loop_1->Reducible());
        auto loop_2 = g2.C->Loop();
        ASSERT(loop_2->Header() == g2.C);
        ASSERT(loop_2->BackEdges().size() == 1);
        ASSERT(loop_2->BackEdges()[0] == g2.D);
        ASSERT(loop_2->Reducible());
        auto loop_3 = g2.E->Loop();
        ASSERT(loop_3->Header() == g2.E);
        ASSERT(loop_3->BackEdges().size() == 1);
        ASSERT(loop_3->BackEdges()[0] == g2.F);
        ASSERT(loop_3->Reducible());

        ASSERT(g2.A->Loop() == loop_0);
        ASSERT(g2.B->Loop() == loop_1);
        ASSERT(g2.C->Loop() == loop_2);
        ASSERT(g2.D->Loop() == loop_2);
        ASSERT(g2.E->Loop() == loop_3);
        ASSERT(g2.F->Loop() == loop_3);
        ASSERT(g2.G->Loop() == loop_1);
        ASSERT(g2.H->Loop() == loop_1);
        ASSERT(g2.I->Loop() == loop_0);
        ASSERT(g2.J->Loop() == loop_1);
        ASSERT(g2.K->Loop() == loop_0);

        g2->BuildLinearOrder();
        auto lo = g2->GetLinearOrder();
        ASSERT(lo.size() == 13U);
        ASSERT(lo[0].block == g2.A);
        ASSERT(lo[1].block == g2.B);
        ASSERT(lo[2].block == g2.J);
        ASSERT(lo[3].block == g2.C);
        ASSERT(lo[4].block == g2.D);
        ASSERT(lo[5].block == g2.E);
        ASSERT(lo[6].block == g2.F);
        ASSERT(lo[7].block == g2.G);
        ASSERT(lo[8].block == g2.H);
        ASSERT(lo[9].block == g2.I);
        ASSERT(lo[10].block == g2.K);
        ASSERT(lo[11].block == g2->GetEndBlock());
        ASSERT(lo[12].block == nullptr);
        
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
        BLOCK(I, RETURNVOID() dummy; );
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
        ASSERT(rpo.size() == 10U);
        ASSERT(rpo[0] == g3.A);
        ASSERT(rpo[1] == g3.B);
        ASSERT(rpo[2] == g3.E);
        ASSERT(rpo[3] == g3.F);
        ASSERT(rpo[4] == g3.H);
        ASSERT(rpo[5] == g3.G);
        ASSERT(rpo[6] == g3.C);
        ASSERT(rpo[7] == g3.D);
        ASSERT(rpo[8] == g3.I);
        ASSERT(rpo[9] == g3->GetEndBlock());

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

        // Similiar asserts:
        ASSERT(g3->IsDominator(g3.A, g3.A));
        ASSERT(g3->IsDominator(g3.A, g3.B));
        ASSERT(g3->IsDominator(g3.B, g3.C));
        ASSERT(g3->IsDominator(g3.B, g3.D));
        ASSERT(g3->IsDominator(g3.B, g3.E));
        ASSERT(g3->IsDominator(g3.E, g3.F));
        ASSERT(g3->IsDominator(g3.B, g3.G));
        ASSERT(g3->IsDominator(g3.F, g3.H));
        ASSERT(g3->IsDominator(g3.B, g3.I));
        
        g3->AnalyzeLoops();
        g3->Dump();

        auto loop_0 = g3.A->Loop();
        ASSERT(loop_0->Header() == nullptr);
        ASSERT(loop_0->BackEdges().size() == 0);
        ASSERT(loop_0->Reducible());
        auto loop_1 = g3.B->Loop();
        ASSERT(loop_1->Header() == g3.B);
        ASSERT(loop_1->BackEdges().size() == 1);
        ASSERT(loop_1->BackEdges()[0] == g3.F);
        ASSERT(loop_1->Reducible());
        auto loop_2 = g3.G->Loop();
        ASSERT(loop_2->Header() == g3.G);
        ASSERT(loop_2->BackEdges().size() == 1);
        ASSERT(loop_2->BackEdges()[0] == g3.D);
        ASSERT(!loop_2->Reducible());

        ASSERT(g3.A->Loop() == loop_0);
        ASSERT(g3.B->Loop() == loop_1);
        ASSERT(g3.C->Loop() == loop_2);
        ASSERT(g3.D->Loop() == loop_2);
        ASSERT(g3.E->Loop() == loop_1);
        ASSERT(g3.F->Loop() == loop_1);
        ASSERT(g3.G->Loop() == loop_2);
        ASSERT(g3.H->Loop() == loop_0);
        ASSERT(g3.I->Loop() == loop_0);

        
        g3->BuildLinearOrder();
        auto lo = g3->GetLinearOrder();
        ASSERT(lo.size() == 11U);
        ASSERT(lo[0].block == g3.A);
        ASSERT(lo[1].block == g3.B);
        ASSERT(lo[2].block == g3.E);
        ASSERT(lo[3].block == g3.F);
        ASSERT(lo[4].block == g3.H);
        ASSERT(lo[5].block == g3.G);
        ASSERT(lo[6].block == g3.C);
        ASSERT(lo[7].block == g3.D);
        ASSERT(lo[8].block == g3.I);
        ASSERT(lo[9].block == g3->GetEndBlock());
        ASSERT(lo[10].block == nullptr);

        g3.BG();
    }

    void test() {
        test1();
        test2();
        test3();
    }
}