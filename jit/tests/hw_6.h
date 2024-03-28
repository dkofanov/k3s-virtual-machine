#include "../ir/graph/gen/graph_ctor.h"
#include <cstddef>

using namespace compiler;

namespace hw6_peepholes {
    GRAPH(g_xor_1,
        BLOCK(b0, 
            PARAMETER(Type::UINT64) v0;
            XOR() xor_i {v0, v0};
            RETURN(Type::UINT64) r {xor_i};
        );
    );

    void test_xor_1() {
        g_xor_1.FG();
        g_xor_1->BuildRPO();
        g_xor_1->ApplyPeepholes();

        ASSERT(g_xor_1->GetBlocksCount() == 2U);

        auto inst_it = g_xor_1.b0->GetAllInsts();
        ASSERT(inst_it->IsParameter());
        ASSERT(!inst_it->HasUsers());

        ++inst_it;
        ASSERT(inst_it->IsConst());
        ASSERT(inst_it->HasUsers());
        auto const_inst_0 = inst_it->AsConst();
        ASSERT(const_inst_0->GetImm() == 0);

        ++inst_it;
        ASSERT(inst_it->IsReturn());
        auto ret_inst = inst_it->AsReturn();
        ASSERT(ret_inst->GetInput(0) == const_inst_0);

        ++inst_it;
        ASSERT(*inst_it == nullptr);

        g_xor_1.BG();
    }

    GRAPH(g_xor_2,
        BLOCK(b0, 
            PARAMETER(Type::UINT64) v0;
            NOT() not_i {v0};
            XOR() xor_i {v0, not_i};
            RETURN(Type::UINT64) r {xor_i};
        );
    );

    void test_xor_2() {
        g_xor_2.FG();
        g_xor_2->BuildRPO();
        g_xor_2->ApplyPeepholes();

        ASSERT(g_xor_2->GetBlocksCount() == 2U);

        auto inst_it = g_xor_2.b0->GetAllInsts();
        ASSERT(inst_it->IsParameter());
        ASSERT(inst_it->HasUsers());
        auto param_inst = inst_it->AsParameter();

        ++inst_it;
        ASSERT(inst_it->IsNot());
        ASSERT(!inst_it->HasUsers());
        auto not_i = inst_it->AsNot();
        ASSERT(not_i->GetInput(0) == param_inst);

        ++inst_it;
        ASSERT(inst_it->IsConst());
        ASSERT(inst_it->HasUsers());
        auto const_inst_1 = inst_it->AsConst();
        ASSERT(const_inst_1->GetImm() == 0xFFFF'FFFF'FFFF'FFFFU);

        ++inst_it;
        ASSERT(inst_it->IsReturn());
        auto ret_inst = inst_it->AsReturn();
        ASSERT(ret_inst->GetInput(0) == const_inst_1);

        ++inst_it;
        ASSERT(*inst_it == nullptr);

        g_xor_2.BG();
    }

    GRAPH(g_xor_3,
        BLOCK(b0, 
            CONST(Type::UINT64, 0) v0;
            PARAMETER(Type::UINT64) v1;
            XOR() xor_i {v0, v1};
            RETURN(Type::UINT64) r {xor_i};
        );
    );

    void test_xor_3() {
        g_xor_3.FG();
        g_xor_3->BuildRPO();
        g_xor_3->ApplyPeepholes();

        ASSERT(g_xor_3->GetBlocksCount() == 2U);

        auto inst_it = g_xor_3.b0->GetAllInsts();
        ASSERT(inst_it->IsConst());
        ASSERT(!inst_it->HasUsers());
        auto const_inst_0 = inst_it->AsConst();
        ASSERT(const_inst_0->GetImm() == 0);

        ++inst_it;
        ASSERT(inst_it->IsParameter());
        ASSERT(inst_it->HasUsers());
        auto param_inst = inst_it->AsParameter();

        ++inst_it;
        ASSERT(inst_it->IsReturn());
        auto ret_inst = inst_it->AsReturn();
        ASSERT(ret_inst->GetInput(0) == param_inst);

        ++inst_it;
        ASSERT(*inst_it == nullptr);

        g_xor_3.BG();
    }

    GRAPH(g_xor_4,
        BLOCK(b0, 
            CONST(Type::UINT64, 0b1010) v0;
            CONST(Type::UINT64, 0b1100) v1;
            XOR() xor_i {v0, v1};
            RETURN(Type::UINT64) r {xor_i};
        );
    );

    void test_xor_4() {
        g_xor_4.FG();
        g_xor_4->BuildRPO();
        g_xor_4->ApplyPeepholes();

        ASSERT(g_xor_4->GetBlocksCount() == 2U);

        auto inst_it = g_xor_4.b0->GetAllInsts();
        ASSERT(inst_it->IsConst());
        ASSERT(!inst_it->HasUsers());
        auto const_inst_0 = inst_it->AsConst();
        ASSERT(const_inst_0->GetImm() == 0b1010);

        ++inst_it;
        ASSERT(inst_it->IsConst());
        ASSERT(!inst_it->HasUsers());
        auto const_inst1 = inst_it->AsConst();
        ASSERT(const_inst1->GetImm() == 0b1100);

        ++inst_it;
        ASSERT(inst_it->IsConst());
        ASSERT(inst_it->HasUsers());
        auto const_inst = inst_it->AsConst();
        ASSERT(const_inst->GetImm() == 0b0110);

        ++inst_it;
        ASSERT(inst_it->IsReturn());
        auto ret_inst = inst_it->AsReturn();
        ASSERT(ret_inst->GetInput(0) == const_inst);

        ++inst_it;
        ASSERT(*inst_it == nullptr);

        g_xor_4.BG();
    }

    void test_xor()
    {
        test_xor_1();
        test_xor_2();
        test_xor_3();
        test_xor_4();
    }

    GRAPH(g_sub_1,
        BLOCK(b0, 
            CONST(Type::INT64, 0x1234) v0;
            CONST(Type::INT64, 0xABCD) v1;
            SUB() sub_i {v0, v1};
            RETURN(Type::UINT64) r {sub_i};
        );
    );

    void test_sub_1() {
        g_sub_1.FG();
        g_sub_1->BuildRPO();
        g_sub_1->ApplyPeepholes();

        ASSERT(g_sub_1->GetBlocksCount() == 2U);

        auto inst_it = g_sub_1.b0->GetAllInsts();
        ASSERT(inst_it->IsConst());
        ASSERT(!inst_it->HasUsers());
        auto const_inst_0 = inst_it->AsConst();
        ASSERT(const_inst_0->GetImm() == 0x1234);

        ++inst_it;
        ASSERT(inst_it->IsConst());
        ASSERT(!inst_it->HasUsers());
        auto const_inst1 = inst_it->AsConst();
        ASSERT(const_inst1->GetImm() == 0xABCD);

        ++inst_it;
        ASSERT(inst_it->IsConst());
        ASSERT(inst_it->HasUsers());
        auto const_inst = inst_it->AsConst();
        ASSERT(const_inst->GetImm() == -39321);

        ++inst_it;
        ASSERT(inst_it->IsReturn());
        auto ret_inst = inst_it->AsReturn();
        ASSERT(ret_inst->GetInput(0) == const_inst);

        ++inst_it;
        ASSERT(*inst_it == nullptr);

        g_sub_1.BG();
    }

    GRAPH(g_sub_2,
        BLOCK(b0, 
            PARAMETER(Type::UINT64) v0;
            PARAMETER(Type::UINT64) v1;
            NEG() neg_i {v1};
            SUB() sub_i {v0, neg_i};
            RETURN(Type::UINT64) r {sub_i};
        );
    );

    void test_sub_2() {
        g_sub_2.FG();
        g_sub_2->BuildRPO();
        g_sub_2->ApplyPeepholes();

        ASSERT(g_sub_2->GetBlocksCount() == 2U);

        auto inst_it = g_sub_2.b0->GetAllInsts();
        ASSERT(inst_it->IsParameter());
        ASSERT(inst_it->HasUsers());
        auto i0 = *inst_it;

        ++inst_it;
        ASSERT(inst_it->IsParameter());
        ASSERT(inst_it->HasUsers());
        auto i1 = *inst_it;

        ++inst_it;
        ASSERT(inst_it->IsNeg());
        ASSERT(!inst_it->HasUsers());
    
        ++inst_it;
        ASSERT(inst_it->IsAdd());
        ASSERT(inst_it->HasUsers());
        auto add_i = inst_it->AsAdd();
        ASSERT(add_i->GetInput(0) == i0);
        ASSERT(add_i->GetInput(1) == i1);
    
        ++inst_it;
        ASSERT(inst_it->IsReturn());
        auto ret_inst = inst_it->AsReturn();
        ASSERT(ret_inst->GetInput(0) == add_i);

        ++inst_it;
        ASSERT(*inst_it == nullptr);

        g_sub_2.BG();
    }
    
    GRAPH(g_sub_3,
        BLOCK(b0, 
            PARAMETER(Type::UINT64) v0;
            PARAMETER(Type::UINT64) v1;
            NEG() neg_i {v1};
            SUB() sub_i {neg_i, v0};
            RETURN(Type::UINT64) r {sub_i};
        );
    );

    void test_sub_3() {
        g_sub_3.FG();
        g_sub_3->BuildRPO();
        g_sub_3->ApplyPeepholes();

        ASSERT(g_sub_3->GetBlocksCount() == 2U);

        auto inst_it = g_sub_3.b0->GetAllInsts();
        ASSERT(inst_it->IsParameter());
        ASSERT(inst_it->HasUsers());
        auto i0 = *inst_it;

        ++inst_it;
        ASSERT(inst_it->IsParameter());
        ASSERT(inst_it->HasUsers());
        auto i1 = *inst_it;

        ++inst_it;
        ASSERT(inst_it->IsNeg());
        ASSERT(!inst_it->HasUsers());

        ++inst_it;
        ASSERT(inst_it->IsAdd());
        ASSERT(inst_it->HasUsers());
        auto add_i = inst_it->AsAdd();
        ASSERT(add_i->GetInput(0) == i1);
        ASSERT(add_i->GetInput(1) == i0);

        ++inst_it;
        ASSERT(inst_it->IsNeg());
        ASSERT(inst_it->HasUsers());
        auto neg_2 = inst_it->AsNeg();
        ASSERT(neg_2->GetInput(0) == add_i);


        ++inst_it;
        ASSERT(inst_it->IsReturn());
        auto ret_inst = inst_it->AsReturn();
        ASSERT(ret_inst->GetInput(0) == neg_2);

        ++inst_it;
        ASSERT(*inst_it == nullptr);

        g_sub_3.BG();
    }

    GRAPH(g_sub_4,
        BLOCK(b0, 
            PARAMETER(Type::UINT64) v0;
            CONST(Type::UINT64, 0) v1;
            SUB() sub_i {v1, v0};
            RETURN(Type::UINT64) r {sub_i};
        );
    );

    void test_sub_4() {
        g_sub_4.FG();
        g_sub_4->BuildRPO();
        g_sub_4->ApplyPeepholes();

        ASSERT(g_sub_4->GetBlocksCount() == 2U);

        auto inst_it = g_sub_4.b0->GetAllInsts();
        ASSERT(inst_it->IsParameter());
        ASSERT(inst_it->HasUsers());
        auto param = *inst_it;

        ++inst_it;
        ASSERT(inst_it->IsConst());
        ASSERT(!inst_it->HasUsers());
        ASSERT(inst_it->AsConst()->GetImm() == 0);

        ++inst_it;
        ASSERT(inst_it->IsNeg());
        ASSERT(inst_it->HasUsers());
        auto neg_i = inst_it->AsNeg();
        ASSERT(neg_i->GetInput(0) == param);
    
        ++inst_it;
        ASSERT(inst_it->IsReturn());
        auto ret_inst = inst_it->AsReturn();
        ASSERT(ret_inst->GetInput(0) == neg_i);

        ++inst_it;
        ASSERT(*inst_it == nullptr);

        g_sub_4.BG();
    }

    GRAPH(g_sub_5,
        BLOCK(b0, 
            PARAMETER(Type::UINT64) v0;
            CONST(Type::UINT64, 0) v1;
            SUB() sub_i {v0, v1};
            RETURN(Type::UINT64) r {sub_i};
        );
    );

    void test_sub_5() {
        g_sub_5.FG();
        g_sub_5->BuildRPO();
        g_sub_5->ApplyPeepholes();

        ASSERT(g_sub_5->GetBlocksCount() == 2U);

        auto inst_it = g_sub_5.b0->GetAllInsts();
        ASSERT(inst_it->IsParameter());
        ASSERT(inst_it->HasUsers());
        auto param = *inst_it;

        ++inst_it;
        ASSERT(inst_it->IsConst());
        ASSERT(!inst_it->HasUsers());
        ASSERT(inst_it->AsConst()->GetImm() == 0);

        ++inst_it;
        ASSERT(inst_it->IsSub());
        ASSERT(!inst_it->HasUsers());

        ++inst_it;
        ASSERT(inst_it->IsReturn());
        auto ret_inst = inst_it->AsReturn();
        ASSERT(ret_inst->GetInput(0) == param);

        ++inst_it;
        ASSERT(*inst_it == nullptr);

        g_sub_5.BG();
    }

    void test_sub()
    {
        test_sub_1();
        test_sub_2();
        test_sub_3();
        test_sub_4();
    }

    GRAPH(g_ashr_1,
        BLOCK(b0, 
            CONST(Type::INT64, -0x1234) v0;
            CONST(Type::INT64, 0x5) v1;
            ASHR() ashr_i {v0, v1};
            RETURN(Type::UINT64) r {ashr_i};
        );
    );

    void test_ashr_1() {
        g_ashr_1.FG();
        g_ashr_1.b0.ashr_i->SetType(g_ashr_1.b0.v0->GetType());
        g_ashr_1->BuildRPO();
        g_ashr_1->ApplyPeepholes();

        ASSERT(g_ashr_1->GetBlocksCount() == 2U);

        auto inst_it = g_ashr_1.b0->GetAllInsts();
        ASSERT(inst_it->IsConst());
        ASSERT(!inst_it->HasUsers());
        auto const_inst_0 = inst_it->AsConst();
        ASSERT(const_inst_0->GetImm() == -0x1234);

        ++inst_it;
        ASSERT(inst_it->IsConst());
        ASSERT(!inst_it->HasUsers());
        auto const_inst1 = inst_it->AsConst();
        ASSERT(const_inst1->GetImm() == 0x5);

        ++inst_it;
        ASSERT(inst_it->IsConst());
        ASSERT(inst_it->HasUsers());
        auto const_inst = inst_it->AsConst();
        ASSERT(const_inst->GetImm() == -146);

        ++inst_it;
        ASSERT(inst_it->IsReturn());
        auto ret_inst = inst_it->AsReturn();
        ASSERT(ret_inst->GetInput(0) == const_inst);

        ++inst_it;
        ASSERT(*inst_it == nullptr);

        g_ashr_1.BG();
    }

    GRAPH(g_ashr_2,
        BLOCK(b0, 
            PARAMETER(Type::INT64) v0;
            CONST(Type::UINT64, 32) v1;
            SHL() shl_i {v0, v1};
            ASHR() ashr_i {shl_i, v1};
            RETURN(Type::UINT64) r {ashr_i};
        );
    );

    void test_ashr_2() {
        g_ashr_2.FG();
        g_ashr_2.b0.ashr_i->SetType(g_ashr_2.b0.v0->GetType());
        g_ashr_2->BuildRPO();
        g_ashr_2->ApplyPeepholes();

        ASSERT(g_ashr_2->GetBlocksCount() == 2U);

        auto inst_it = g_ashr_2.b0->GetAllInsts();
        ASSERT(inst_it->IsParameter());
        ASSERT(inst_it->HasUsers());
        auto param = *inst_it;

        ++inst_it;
        ASSERT(inst_it->IsConst());
        ASSERT(inst_it->HasUsers());
        auto const_i = *inst_it;
        ASSERT(const_i->AsConst()->GetImm() == 32U);

        ++inst_it;
        ASSERT(inst_it->IsShl());
        ASSERT(!inst_it->HasUsers());
        auto shl_i = inst_it->AsShl();
        ASSERT(shl_i->GetInput(0) == param);
        ASSERT(shl_i->GetInput(1) == const_i);
    
        ++inst_it;
        ASSERT(inst_it->IsCast());
        ASSERT(inst_it->HasUsers());
        auto cast_i = inst_it->AsCast();
        ASSERT(cast_i->GetInput(0) == param);
        ASSERT(cast_i->GetType() == Type::INT32);
    
        ++inst_it;
        ASSERT(inst_it->IsReturn());
        auto ret_inst = inst_it->AsReturn();
        ASSERT(ret_inst->GetInput(0) == cast_i);

        ++inst_it;
        ASSERT(*inst_it == nullptr);

        g_ashr_2.BG();
    }
    
    GRAPH(g_ashr_3,
        BLOCK(b0, 
            PARAMETER(Type::INT64) v0;
            CONST(Type::UINT64, 0) v1;
            ASHR() ashr_i {v0, v1};
            RETURN(Type::UINT64) r {ashr_i};
        );
    );

    void test_ashr_3()
    {
        g_ashr_3.FG();
        g_ashr_3->BuildRPO();
        g_ashr_3->ApplyPeepholes();

        ASSERT(g_ashr_3->GetBlocksCount() == 2U);

        auto inst_it = g_ashr_3.b0->GetAllInsts();
        ASSERT(inst_it->IsParameter());
        ASSERT(inst_it->HasUsers());
        auto param = *inst_it;

        ++inst_it;
        ASSERT(inst_it->IsConst());
        ASSERT(!inst_it->HasUsers());
        ASSERT(inst_it->AsConst()->GetImm() == 0);
    
        ++inst_it;
        ASSERT(inst_it->IsReturn());
        auto ret_inst = inst_it->AsReturn();
        ASSERT(ret_inst->GetInput(0) == param);

        ++inst_it;
        ASSERT(*inst_it == nullptr);

        g_ashr_3.BG();
    }

    void test_ashr()
    {
        test_ashr_1();
        test_ashr_2();
        test_ashr_3();
    }

    void test()
    {
        test_xor();
        test_sub();
        test_ashr();
    }
}
