#include "../ir/graph/gen/graph_ctor.h"
#include <cstddef>

using namespace compiler;

namespace hw8_checks_elim {

GRAPH(g0,
    BLOCK(b0,
        PARAMETER(Type::ARRAY) p0;
        CONST(Type::UINT64, 0) c0;
        CONST(Type::UINT64, 1) c1;
        CONST(Type::UINT64, 5) c5;
    );

    BLOCK(b1,
        CHECKNULL() nc {b0.p0};
        CHECKBOUNDS() bc {b0.p0, b0.c0};
        GETELEM(Type::UINT64) el0 {nc, bc};
        JMP(If::NE) j {el0, b0.c5};
    );

    BLOCK(b2,
        CHECKNULL()             nc0 {b0.p0};
        CHECKBOUNDS()           bc0 {b0.p0, b0.c1};
        GETELEM(Type::UINT64)   el1_0 {nc0, bc0};
        CHECKNULL()             nc1 {b0.p0};
        CHECKBOUNDS()           bc1 {b0.p0, b0.c1};
        GETELEM(Type::UINT64)   el1_1 {nc1, bc1};
        CHECKNULL()             nc2 {b0.p0};
        CHECKBOUNDS()           bc2 {b0.p0, b0.c5};
        GETELEM(Type::UINT64)   el2 {nc2, bc2};
    );

    BLOCK(b3,
        CHECKNULL() nc {b0.p0};
        CHECKBOUNDS() bc {b0.p0, b0.c1};
        GETELEM(Type::UINT64) el1 {nc, bc};
    );

    BLOCK(b4, 
        PHI(2) p {b2.el2, b3.el1};
        RETURNVOID() r {};
    );

    
    EDGES(
        b0 --> b1 --> (b2, b3) --> b4;
    );
);

void test_0()
{
    g0.FG();

    g0->BuildRPO();
    g0->BuildDomTree();
    g0->ApplyChecksElimination();

    ASSERT(g0.b1.nc->BB() == g0.b1);
    ASSERT(g0.b1.bc->BB() == g0.b1);
    ASSERT(g0.b1.el0->GetInput(0) == g0.b1.nc);
    ASSERT(g0.b1.el0->GetInput(1) == g0.b1.bc);

    ASSERT(g0.b2.nc0->BB() == nullptr);
    ASSERT(g0.b2.bc0->BB() == g0.b2);
    ASSERT(g0.b2.el1_0->GetInput(0) == g0.b1.nc);
    ASSERT(g0.b2.el1_0->GetInput(1) == g0.b2.bc0);

    ASSERT(g0.b2.nc1->BB() == nullptr);
    ASSERT(g0.b2.bc1->BB() == nullptr);
    ASSERT(g0.b2.el1_1->GetInput(0) == g0.b1.nc);
    ASSERT(g0.b2.el1_1->GetInput(1) == g0.b2.bc0);
    
    ASSERT(g0.b2.nc2->BB() == nullptr);
    ASSERT(g0.b2.bc2->BB() == g0.b2);
    ASSERT(g0.b2.el2->GetInput(0) == g0.b1.nc);
    ASSERT(g0.b2.el2->GetInput(1) == g0.b2.bc2);

    ASSERT(g0.b3.nc->BB() == nullptr);
    ASSERT(g0.b3.bc->BB() == g0.b3);
    ASSERT(g0.b3.el1->GetInput(0) == g0.b1.nc);
    ASSERT(g0.b3.el1->GetInput(1) == g0.b3.bc);

    g0->Dump();

    g0.BG();
}

void test()
{
    test_0();
}

}
