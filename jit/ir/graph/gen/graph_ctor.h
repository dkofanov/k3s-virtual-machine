#pragma once
#include "../graph.h"
#include "../../../common.h"
#include "../analyses/loop.h"
#include <string_view>
#include <initializer_list>

namespace compiler {

template <bool RESET_IDS>
class GraphHolder
{
public:
    GraphHolder()
    {
        FG();
    }
    void FG()
    {
        ASSERT(GRAPH == nullptr);
        GRAPH = &g_;
    }
    void BG()
    {
        ASSERT(GRAPH == &g_);
        GRAPH = nullptr;
    }
    const auto *GetGraph() const { return &g_; }
    auto *GetGraph() { return &g_; }
private:
    Graph g_ { RESET_IDS };
};

#define GRAPH_CLASS(GR, RESET_IDS, ...) \
class Graph_##GR                                                            \
{                                                                           \
public:                                                                     \
    Graph_##GR() {}                                                         \
    void FG() { GH.FG(); }                                                  \
    void BG() { GH.BG(); }                                                  \
    operator Graph*() { return GH.GetGraph(); }                             \
    Graph *operator->() { return GH.GetGraph(); }                           \
    void Dump() { GH.GetGraph()->Dump(); }                                  \
    void DumpRPO() { GH.GetGraph()->BuildRPO(); GH.GetGraph()->DumpRPO(); } \
private:                                                                    \
    inline static GraphHolder<RESET_IDS> GH {};                             \
public:                                                                     \
    __VA_ARGS__                                                             \
private:                                                                    \
    class GraphBG                                                           \
    {                                                                       \
    public:                                                                 \
        GraphBG() { GH.BG(); }                                              \
    }; inline static GraphBG BG_##GR {};                                    \
}

#define GRAPH(GR, ...) GRAPH_CLASS(GR, true, __VA_ARGS__) GR {};

#define SUBGRAPH(GR, ...) GRAPH_CLASS(GR, false, __VA_ARGS__) GR {};

class BlockCtorBase
{
public:
    BlockCtorBase(std::string_view lbl) : bb_id_(GRAPH->NewBB()), lbl_(lbl) {}

    void Dump() { GRAPH->GetBlockById(bb_id_)->Dump(); }
    auto GetBB() const { return GRAPH->GetBlockById(bb_id_); }
    operator BasicBlock*() { return GetBB(); }
    BasicBlock *operator->() { return GetBB(); }
    class PredSuccDeclarator
    {
    public:
        PredSuccDeclarator(size_t bb_id) { bb_ids_.push_back(bb_id); }
        PredSuccDeclarator(size_t bb_id_0, size_t bb_id_1)
        {
            bb_ids_.push_back(bb_id_0);
            bb_ids_.push_back(bb_id_1);
        }

        auto &operator,(const BlockCtorBase &next_b)
        {
            bb_ids_.push_back(next_b.GetBB()->Id());
            return *this;
        }

        auto &operator>(const BlockCtorBase &succ) const
        {
            // multiple phi fix is not supported:
            ASSERT((missing_phi_inputs_.size() == 0) || (bb_ids_.size() == 1));

            for (auto id : bb_ids_) {
                auto succ_bb = succ.GetBB();
                GRAPH->GetBlockById(id)->SetSucc(succ_bb);
                size_t input_idx = succ_bb->Preds().size() - 1;
                Inst *inst = succ_bb->FirstPhi();
                for (size_t i = 0; i < missing_phi_inputs_.size(); i++) {
                    ASSERT(inst->IsPhi());
                    auto phi = inst->AsPhi();
                    ASSERT(phi->GetInput(input_idx) == nullptr);
                    phi->SetInput(input_idx, missing_phi_inputs_[i]);
                    inst = inst->Next();
                }
            }
            return succ;
        }

        auto &operator>(const PredSuccDeclarator &succs) const
        {
            if (succs.bb_ids_.size() == 1) {
                ASSERT(missing_phi_inputs_.empty());
                auto b_succ = GRAPH->GetBlockById(succs.bb_ids_[0]);
                for (auto id : bb_ids_) {
                    GRAPH->GetBlockById(id)->SetSucc(b_succ);
                }
            } else {
                ASSERT(succs.bb_ids_.size() == 2);
                auto b_true = GRAPH->GetBlockById(succs.bb_ids_[0]);
                auto b_false = GRAPH->GetBlockById(succs.bb_ids_[1]);
                for (auto id : bb_ids_) {
                    GRAPH->GetBlockById(id)->SetTrueFalseSuccs(b_true, b_false);
                }
                // Nasty hack for hw5::g_loopSideExit, need to be fixed.
                if (!missing_phi_inputs_.empty()) {
                    size_t input_idx = b_true->Preds().size() - 1;
                    Inst *inst = b_true->FirstPhi();
                    ASSERT(inst != nullptr);
                    for (size_t i = 0; i < missing_phi_inputs_.size(); i++) {
                        ASSERT(inst->IsPhi());
                        auto phi = inst->AsPhi();
                        ASSERT(phi->GetInput(input_idx) == nullptr);
                        phi->SetInput(input_idx, missing_phi_inputs_[i]);
                        inst = inst->Next();
                    }
                }
            }
            return succs;
        }

        auto &operator--(int) const { return *this; }

        template<typename ... Args>
        auto &operator()(Inst *input, Args &... args)
        {
            missing_phi_inputs_.push_back(input);
            (*this)(args...);
            return *this;     
        }
        auto &operator()(Inst *input)
        {
            missing_phi_inputs_.push_back(input);
            return *this;     
        }

    public:
        std::vector<size_t> bb_ids_ {};
        std::vector<Inst *> missing_phi_inputs_ {};
    };
    
    auto operator--(int) { return PredSuccDeclarator(bb_id_); }
    auto operator,(BlockCtorBase &second_b) { return PredSuccDeclarator(bb_id_, second_b.GetBB()->Id()); }

    operator const BasicBlock*() { return GetBB(); } 
public:
    size_t bb_id_;
    std::string_view lbl_;
};

#define DECL_CLASS_REC(SUFX, BASE, ...)

#define BLOCK(BL, ...) \
class BlockCtor_##BL : public BlockCtorBase {   \
public:                                         \
BlockCtor_##BL() : BlockCtorBase(#BL) {}        \
__VA_ARGS__                                     \
}; static inline BlockCtor_##BL BL 

#define EDGES(...) EDGES_INTERNAL(__LINE__, __VA_ARGS__)
#define EDGES_INTERNAL(LINE, ...) EDGES_PASTE(LINE, __VA_ARGS__)

#define EDGES_PASTE(LINE, ...) \
class Edges_##LINE {                        \
public:                                     \
Edges_##LINE() {                            \
__VA_ARGS__;                                \
};                                          \
}; static inline Edges_##LINE edges_##LINE 



#define CONST(...) CONST_INTERNAL(__LINE__, __VA_ARGS__)

#define CONST_INTERNAL(LINE, ...) CONST_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define CONST_INTERNAL_PASTE(LINE, ...) \
    struct ConstInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        ConstInst_##LINE##_ctor (const ConstInst_##LINE##_ctor &) = delete;       \
        ConstInst_##LINE##_ctor (ConstInst_##LINE##_ctor &&) = delete;            \
        ConstInst_##LINE##_ctor ()                                                             \
        {                                                                                                   \
            inst_ = new ConstInst (__VA_ARGS__);                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::CONST>(GRAPH, GRAPH->GetBlockById());                      \
        }                                                                                                   \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        ConstInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new ConstInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::CONST>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator ConstInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        ConstInst *inst_ {};                                                                   \
    }


#define PARAMETER(...) PARAMETER_INTERNAL(__LINE__, __VA_ARGS__)

#define PARAMETER_INTERNAL(LINE, ...) PARAMETER_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define PARAMETER_INTERNAL_PASTE(LINE, ...) \
    struct ParameterInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        ParameterInst_##LINE##_ctor (const ParameterInst_##LINE##_ctor &) = delete;       \
        ParameterInst_##LINE##_ctor (ParameterInst_##LINE##_ctor &&) = delete;            \
        ParameterInst_##LINE##_ctor ()                                                             \
        {                                                                                                   \
            inst_ = new ParameterInst (__VA_ARGS__);                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::PARAMETER>(GRAPH, GRAPH->GetBlockById());                      \
        }                                                                                                   \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        ParameterInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new ParameterInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::PARAMETER>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator ParameterInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        ParameterInst *inst_ {};                                                                   \
    }


#define CAST(...) CAST_INTERNAL(__LINE__, __VA_ARGS__)

#define CAST_INTERNAL(LINE, ...) CAST_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define CAST_INTERNAL_PASTE(LINE, ...) \
    struct CastInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        CastInst_##LINE##_ctor (const CastInst_##LINE##_ctor &) = delete;       \
        CastInst_##LINE##_ctor (CastInst_##LINE##_ctor &&) = delete;            \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        CastInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new CastInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::CAST>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator CastInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        CastInst *inst_ {};                                                                   \
    }


#define CALL(...) CALL_INTERNAL(__LINE__, __VA_ARGS__)

#define CALL_INTERNAL(LINE, ...) CALL_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define CALL_INTERNAL_PASTE(LINE, ...) \
    struct CallInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        CallInst_##LINE##_ctor (const CallInst_##LINE##_ctor &) = delete;       \
        CallInst_##LINE##_ctor (CallInst_##LINE##_ctor &&) = delete;            \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        CallInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new CallInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::CALL>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator CallInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        CallInst *inst_ {};                                                                   \
    }


#define RETURN(...) RETURN_INTERNAL(__LINE__, __VA_ARGS__)

#define RETURN_INTERNAL(LINE, ...) RETURN_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define RETURN_INTERNAL_PASTE(LINE, ...) \
    struct ReturnInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        ReturnInst_##LINE##_ctor (const ReturnInst_##LINE##_ctor &) = delete;       \
        ReturnInst_##LINE##_ctor (ReturnInst_##LINE##_ctor &&) = delete;            \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        ReturnInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new ReturnInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::RETURN>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator ReturnInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        ReturnInst *inst_ {};                                                                   \
    }


#define RETURNVOID(...) RETURNVOID_INTERNAL(__LINE__, __VA_ARGS__)

#define RETURNVOID_INTERNAL(LINE, ...) RETURNVOID_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define RETURNVOID_INTERNAL_PASTE(LINE, ...) \
    struct ReturnVoidInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        ReturnVoidInst_##LINE##_ctor (const ReturnVoidInst_##LINE##_ctor &) = delete;       \
        ReturnVoidInst_##LINE##_ctor (ReturnVoidInst_##LINE##_ctor &&) = delete;            \
        ReturnVoidInst_##LINE##_ctor ()                                                             \
        {                                                                                                   \
            inst_ = new ReturnVoidInst (__VA_ARGS__);                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::RETURNVOID>(GRAPH, GRAPH->GetBlockById());                      \
        }                                                                                                   \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        ReturnVoidInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new ReturnVoidInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::RETURNVOID>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator ReturnVoidInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        ReturnVoidInst *inst_ {};                                                                   \
    }


#define ADD(...) ADD_INTERNAL(__LINE__, __VA_ARGS__)

#define ADD_INTERNAL(LINE, ...) ADD_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define ADD_INTERNAL_PASTE(LINE, ...) \
    struct AddInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        AddInst_##LINE##_ctor (const AddInst_##LINE##_ctor &) = delete;       \
        AddInst_##LINE##_ctor (AddInst_##LINE##_ctor &&) = delete;            \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        AddInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new AddInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::ADD>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator AddInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        AddInst *inst_ {};                                                                   \
    }


#define SUB(...) SUB_INTERNAL(__LINE__, __VA_ARGS__)

#define SUB_INTERNAL(LINE, ...) SUB_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define SUB_INTERNAL_PASTE(LINE, ...) \
    struct SubInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        SubInst_##LINE##_ctor (const SubInst_##LINE##_ctor &) = delete;       \
        SubInst_##LINE##_ctor (SubInst_##LINE##_ctor &&) = delete;            \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        SubInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new SubInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::SUB>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator SubInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        SubInst *inst_ {};                                                                   \
    }


#define MUL(...) MUL_INTERNAL(__LINE__, __VA_ARGS__)

#define MUL_INTERNAL(LINE, ...) MUL_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define MUL_INTERNAL_PASTE(LINE, ...) \
    struct MulInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        MulInst_##LINE##_ctor (const MulInst_##LINE##_ctor &) = delete;       \
        MulInst_##LINE##_ctor (MulInst_##LINE##_ctor &&) = delete;            \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        MulInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new MulInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::MUL>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator MulInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        MulInst *inst_ {};                                                                   \
    }


#define NEG(...) NEG_INTERNAL(__LINE__, __VA_ARGS__)

#define NEG_INTERNAL(LINE, ...) NEG_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define NEG_INTERNAL_PASTE(LINE, ...) \
    struct NegInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        NegInst_##LINE##_ctor (const NegInst_##LINE##_ctor &) = delete;       \
        NegInst_##LINE##_ctor (NegInst_##LINE##_ctor &&) = delete;            \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        NegInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new NegInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::NEG>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator NegInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        NegInst *inst_ {};                                                                   \
    }


#define NOT(...) NOT_INTERNAL(__LINE__, __VA_ARGS__)

#define NOT_INTERNAL(LINE, ...) NOT_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define NOT_INTERNAL_PASTE(LINE, ...) \
    struct NotInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        NotInst_##LINE##_ctor (const NotInst_##LINE##_ctor &) = delete;       \
        NotInst_##LINE##_ctor (NotInst_##LINE##_ctor &&) = delete;            \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        NotInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new NotInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::NOT>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator NotInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        NotInst *inst_ {};                                                                   \
    }


#define XOR(...) XOR_INTERNAL(__LINE__, __VA_ARGS__)

#define XOR_INTERNAL(LINE, ...) XOR_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define XOR_INTERNAL_PASTE(LINE, ...) \
    struct XorInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        XorInst_##LINE##_ctor (const XorInst_##LINE##_ctor &) = delete;       \
        XorInst_##LINE##_ctor (XorInst_##LINE##_ctor &&) = delete;            \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        XorInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new XorInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::XOR>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator XorInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        XorInst *inst_ {};                                                                   \
    }


#define ASHR(...) ASHR_INTERNAL(__LINE__, __VA_ARGS__)

#define ASHR_INTERNAL(LINE, ...) ASHR_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define ASHR_INTERNAL_PASTE(LINE, ...) \
    struct AshrInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        AshrInst_##LINE##_ctor (const AshrInst_##LINE##_ctor &) = delete;       \
        AshrInst_##LINE##_ctor (AshrInst_##LINE##_ctor &&) = delete;            \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        AshrInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new AshrInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::ASHR>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator AshrInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        AshrInst *inst_ {};                                                                   \
    }


#define SHL(...) SHL_INTERNAL(__LINE__, __VA_ARGS__)

#define SHL_INTERNAL(LINE, ...) SHL_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define SHL_INTERNAL_PASTE(LINE, ...) \
    struct ShlInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        ShlInst_##LINE##_ctor (const ShlInst_##LINE##_ctor &) = delete;       \
        ShlInst_##LINE##_ctor (ShlInst_##LINE##_ctor &&) = delete;            \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        ShlInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new ShlInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::SHL>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator ShlInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        ShlInst *inst_ {};                                                                   \
    }


#define PHI(...) PHI_INTERNAL(__LINE__, __VA_ARGS__)

#define PHI_INTERNAL(LINE, ...) PHI_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define PHI_INTERNAL_PASTE(LINE, ...) \
    struct PhiInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushPhi(inst);                                                           \
        }                                                                                                   \
        PhiInst_##LINE##_ctor (const PhiInst_##LINE##_ctor &) = delete;       \
        PhiInst_##LINE##_ctor (PhiInst_##LINE##_ctor &&) = delete;            \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        PhiInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new PhiInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::PHI>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator PhiInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        PhiInst *inst_ {};                                                                   \
    }


#define JMP(...) JMP_INTERNAL(__LINE__, __VA_ARGS__)

#define JMP_INTERNAL(LINE, ...) JMP_INTERNAL_PASTE(LINE, __VA_ARGS__)   

#define JMP_INTERNAL_PASTE(LINE, ...) \
    struct JmpInst_##LINE##_ctor                                                             \
    {                                                                                                       \
        void AppendInst()                                                                                   \
        {                                                                                                   \
            auto inst = this->inst_;                                                                        \
            GRAPH->GetBlockById()->PushBack(inst);                                                          \
        }                                                                                                   \
        JmpInst_##LINE##_ctor (const JmpInst_##LINE##_ctor &) = delete;       \
        JmpInst_##LINE##_ctor (JmpInst_##LINE##_ctor &&) = delete;            \
        /* init-list currently used only for inputs so `Inst *` should be enough */                         \
        JmpInst_##LINE##_ctor (std::initializer_list<Inst *> l)                              \
        {                                                                                                   \
            size_t i = 0;                                                                                   \
            inst_ = new JmpInst (__VA_ARGS__);                                               \
            for (auto input : l) {                                                                          \
                inst_->SetInput(i, input);                                                                  \
                i++;                                                                                        \
            }                                                                                               \
            AppendInst();                                                                                   \
            GraphInternalsFixup<Inst::JMP>(GRAPH, GRAPH->GetBlockById());             \
        }                                                                                                   \
        operator JmpInst*() { return inst_; }                                                \
        auto *operator->() { return inst_; }                                               \
        JmpInst *inst_ {};                                                                   \
    }


}
