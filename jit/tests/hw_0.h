#include "../ir/graph/gen/graph_ctor.h"

using namespace compiler;

namespace hw0
{
    GRAPH(g,
        BLOCK(entry,
            PARAMETER(Type::FLOAT64) a0;
            PARAMETER(Type::FLOAT64) a1;
        );
        
        BLOCK(b_if,
            ADD() s0 {entry.a0, entry.a1};
            //ADD(s0, a0) s1;
            //CMP(s0, s1) cmp0;
            //BR(GT) br
        );
        
        BLOCK(b_true,
            SUB() sub {b_if.s0, b_if.s0};
        );

        BLOCK(b_false,
            SUB() sub { b_if.s0, b_if.s0};
        );
    
        BLOCK(last,
            SUB() sub { b_false.sub, b_true.sub};
        );

        EDGES(
            entry --> b_if --> (b_true, b_false) --> last --> b_if;
        );
    );


    void test() {
        g->BuildRPO();
        g->BuildDomTree();
        g->AnalyzeLoops();
        g.Dump();
    }
}
