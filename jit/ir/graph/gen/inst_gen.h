
#pragma once
#include <iostream>
#include <iomanip>

namespace compiler {

class BasicBlock;
class User;

class Inst {
public:
    enum Opcode {
        NONE,
        CONST,
        ADD,
        SUB,
        MUL,
        PHI,
        JMP,
        PARAMETER,
        RETURN,
    };

    bool IsConst() const { return opcode_ == CONST; }
    auto AsConst();
    bool IsAdd() const { return opcode_ == ADD; }
    auto AsAdd();
    bool IsSub() const { return opcode_ == SUB; }
    auto AsSub();
    bool IsMul() const { return opcode_ == MUL; }
    auto AsMul();
    bool IsPhi() const { return opcode_ == PHI; }
    auto AsPhi();
    bool IsJmp() const { return opcode_ == JMP; }
    auto AsJmp();
    bool IsParameter() const { return opcode_ == PARAMETER; }
    auto AsParameter();
    bool IsReturn() const { return opcode_ == RETURN; }
    auto AsReturn();

    Inst(Opcode opcode);

    Inst *GetInput(size_t i);
    size_t GetInputsCount();

    void Dump();

    auto GetOpcode()
    {
        return opcode_;
    }

    void SetNext(Inst *next)
    {
        ASSERT(next_ == nullptr);
        next_ = next;
        ASSERT(next->prev_ == nullptr);
        next->prev_ = this;
    }
    Inst *Next()
    {
        return next_;
    }
    
    auto Id() const
    {
        return id_;
    }
    
    auto BB() { return bb_; }
    size_t GetBBId();
    void SetBB(BasicBlock *bb) { bb_ = bb; }

private:
    auto GetFirstUserRef()
    {
        return &first_user_;
    }

    bool IsFixed()
    {
        switch (opcode_) {
        case PHI:
            return false;
        default:
            return true;
        }
    }
    
private:
    static constexpr uint8_t INPUTS_COUNT_ARRAY[] =
    {
        (uint8_t) -1,
 (uint8_t) 0,  (uint8_t) 2,  (uint8_t) 2,  (uint8_t) 2,  (uint8_t) -1,  (uint8_t) 2,  (uint8_t) 0,  (uint8_t) 1,     
    };

private:
    Opcode opcode_{};
    size_t id_{};
    BasicBlock *bb_{};
    User *first_user_{};
    Inst *prev_{};
    Inst *next_{};

friend class User;
};

class TypedMixin {
public:
    enum class Type {
        FLOAT64,
        INT64,
    };

    const auto *ToString() const
    {
        switch (type_) {
        case Type::FLOAT64: return "FLOAT64";
        case Type::INT64: return "INT64";
        default: UNREACHABLE();
        }
    }

    auto &Dump()
    {
        return std::cout << "Type::" << ToString();
    }

    TypedMixin(Type type) : type_{type} {}
private:
    Type type_;
};
using Type = TypedMixin::Type;

class ImmediateMixin {
public:
    auto &Dump()
    {
        return std::cout << imm_;
    }

    ImmediateMixin(uint64_t imm) : imm_{imm} {}
private:
    uint64_t imm_;
};
using Type = TypedMixin::Type;


class ConditionalMixin {
public:
    enum class If {
        EQ,
        NE,
        GT,
        GE,
    }; 
    const auto *ToString() const
    {
        switch (cond_) {
        case If::EQ: return "EQ";
        case If::NE: return "NE";
        case If::GT: return "GT";
        case If::GE: return "GE";
        default: UNREACHABLE();
        }
    }

    auto &Dump()
    {
        return std::cout << "If::" << ToString();
    }

    ConditionalMixin(If cond) : cond_{cond} {}
private:
    If cond_;
};
using If = ConditionalMixin::If;

class User {
public:
    Inst *GetUserInst() const
    {
        const User *user = this;
        size_t i = 0;

        while (!user->IsSpecial()) {
            // Users are always stored in a special-terminated array (whether dynamic or static)
            user++;
        }

        // This idx is also corresponds to the input:
        auto inst = user->ExtractPointerToInst();
        size_t inputs_count = inst->GetInputsCount();
        size_t user_idx = inputs_count - (user - this);

        auto *user_inst = inst->GetInput(user_idx);

        return user_inst;
    }

    User **GetFirstUserRef() const
    {
        return GetUserInst()->GetFirstUserRef();
    }

    void InitAsPointerToInst(Inst *inst)
    {
        next_or_inst_ = reinterpret_cast<uintptr_t>(inst);
        ASSERT(!IsSpecial());
        next_or_inst_ |= MASK;
        ASSERT(IsSpecial());
        ASSERT(inst == ExtractPointerToInst());
    }

    Inst *ExtractPointerToInst() const
    {
        ASSERT(IsSpecial());
        auto inst = next_or_inst_ & (~MASK);
        ASSERT(!IsSpecial(inst));
        return reinterpret_cast<Inst *>(inst);
    }

    bool IsSpecial() const
    {
        return IsSpecial(next_or_inst_); 
    }

    User *GetPrev() const
    {
        return GetPrev(*GetFirstUserRef());
    }

    User *GetNext() const
    {
        ASSERT(!IsSpecial());
        return reinterpret_cast<User *>(next_or_inst_);
    }
    

    void ReplaceUserWith(Inst *inst)
    {
        ASSERT(!IsSpecial());
        auto *next = GetNext();
        // Cache `first_user` as it may be used later:
        auto **first_user_ref = GetFirstUserRef();
        auto *prev = GetPrev(*first_user_ref);

        if (prev == nullptr) {
            *first_user_ref = next;
        }
    
        ASSERT((next == nullptr) || !next->IsSpecial());
        ASSERT(!prev->IsSpecial());
        prev->SetNext(next);

        // Set new user as the head of the list:
        auto **new_first_user_ref = inst->GetFirstUserRef();
        SetNext(*new_first_user_ref);
        *new_first_user_ref = this;
    }

    void AppendUseOf(Inst *inst)
    {
        ASSERT(next_or_inst_ == 0);
        ASSERT(GetUserInst() == nullptr);
        auto first_user_ref = inst->GetFirstUserRef(); 
        next_or_inst_ = reinterpret_cast<uintptr_t>(*first_user_ref); 
        *first_user_ref = this; 
    }

private:
    User *GetPrev(User *first_user) const
    {
        ASSERT(!IsSpecial());
        auto *user1 = first_user;
        ASSERT(!user1->IsSpecial());

        if (user1 == this) {
            return nullptr;
        }
        ASSERT(user1 != nullptr);
        ASSERT(user1->GetNext() != nullptr);
        while (user1->GetNext() != this) {
            ASSERT(!user1->IsSpecial());
            ASSERT(user1->GetNext() != nullptr);
            user1 = user1->GetNext();
        }

        return user1;
    }

    void SetNext(User *next)
    {
        next_or_inst_ = reinterpret_cast<uintptr_t>(next);
    }

    bool IsSpecial(uintptr_t word) const
    {
        return (word & MASK) == MASK; 
    }
private:
    static constexpr uintptr_t MASK = 0xFF00'0000'0000'0000;
    uintptr_t next_or_inst_;
};

template <size_t INPUTS_COUNT>
class FixedInputsInst : public Inst {
public:
    template <typename ... Args>
    FixedInputsInst(Args &&... args) : Inst(std::forward<Args>(args)...)
    {
        users_[INPUTS_COUNT].InitAsPointerToInst(this);
    }

    template <size_t IDX>
    void SetInput(Inst *inst)
    {
        static_assert(IDX < INPUTS_COUNT);

        if (inputs_[IDX] != nullptr) {
            // Firstly, must remove the old user:
            users_[IDX].ReplaceUserWith(inst);
        } else {
            users_[IDX].AppendUseOf(inst);
        }
        inputs_[IDX] = inst;
    }

    void SetInput(size_t idx, Inst *inst)
    {
        ASSERT(idx < INPUTS_COUNT);

        if (inputs_[idx] != nullptr) {
            // Firstly, must remove the old user:
            users_[idx].ReplaceUserWith(inst);
        } else {
            users_[idx].AppendUseOf(inst);
        }
        inputs_[idx] = inst;
    }

    Inst *GetInput(size_t idx)
    {
        return inputs_[idx];
    }
    auto &DumpDF()
    {
        ASSERT(inputs_[0] != nullptr);
        std::cout << "{ " << ((BB() == inputs_[0]->BB()) ? "" : "b" + std::to_string(inputs_[0]->GetBBId()) + ".") << "v" << inputs_[0]->Id();
        for (size_t i = 1; i < INPUTS_COUNT; i++) {
            ASSERT(inputs_[i] != nullptr);
            std::cout << ", " << ((BB() == inputs_[i]->BB()) ? "" : "b" + std::to_string(inputs_[i]->GetBBId()) + ".") << "v" << inputs_[i]->Id();
        }
        return std::cout << " }";
    }
private:
    Inst* inputs_[INPUTS_COUNT] {};
    User users_[INPUTS_COUNT + 1] {};
};

template <>
class FixedInputsInst<0> : public Inst {
public:
    template <typename ... Args>
    FixedInputsInst(Args &&... args) : Inst(std::forward<Args>(args)...) {}

    template <size_t IDX>
    void SetInput(Inst *inst)
    {
        UNREACHABLE();
    }

    void SetInput(size_t idx, Inst *inst)
    {
        UNREACHABLE();
    }

    Inst *GetInput(size_t idx)
    {
        UNREACHABLE();
    }
    auto &DumpDF()
    {
        // dump users only
        return std::cout;
    }
private:
};

// In fact, dynamic allocations occurs exactly 1 time (on initialization):
class VariadicInputsInst : public Inst {
public:
    template <typename ... Args>
    VariadicInputsInst(size_t inputs_count, Args &&... args)
    : Inst(std::forward<Args>(args)...), inputs_(inputs_count), users_(inputs_count + 1) 
    {
        users_[inputs_count].InitAsPointerToInst(this);
    }

    template <size_t IDX>
    void SetInput(Inst *inst)
    {
        ASSERT(IDX < inputs_.size());

        if (inputs_[IDX] != nullptr) {
            // Firstly, must remove the old user:
            users_[IDX].ReplaceUserWith(inst);
        } else {
            users_[IDX].AppendUseOf(inst);
        }
        inputs_[IDX] = inst;
    }
    void SetInput(size_t idx, Inst *inst)
    {
        ASSERT(idx < inputs_.size());

        if (inputs_[idx] != nullptr) {
            // Firstly, must remove the old user:
            users_[idx].ReplaceUserWith(inst);
        } else {
            users_[idx].AppendUseOf(inst);
        }
        inputs_[idx] = inst;
    }

    Inst *GetInput(size_t idx)
    {
        ASSERT(idx < inputs_.size());
        return inputs_[idx];
    }

    size_t GetInputsCount() const
    {
        ASSERT(inputs_.size() == (users_.size() - 1));
        return inputs_.size();
    }
    auto &DumpDF()
    {
        ASSERT(inputs_[0] != nullptr);
        std::cout << "{ " << ((BB() == inputs_[0]->BB()) ? "" : "b" + std::to_string(inputs_[0]->GetBBId()) + ".") << "v" << inputs_[0]->Id();
        for (size_t i = 1; i < inputs_.size(); i++) {
            ASSERT(inputs_[i] != nullptr);
            std::cout << ", " << ((BB() == inputs_[i]->BB()) ? "" : "b" + std::to_string(inputs_[i]->GetBBId()) + ".") << "v" << inputs_[i]->Id();
        }
        return std::cout << " }";
    }

private:
    std::vector<Inst *> inputs_ {};
    std::vector<User> users_ {};
};


class ConstInst : public FixedInputsInst<0>, public TypedMixin, public ImmediateMixin
{
public:
    template <typename Arg0, typename Arg1, typename unused = void>
    ConstInst( Arg0 &&arg0, Arg1 &&arg1)
    : FixedInputsInst<0>(CONST)
    , TypedMixin(arg0)
    , ImmediateMixin(arg1)
    {}

    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "CONST" << std::setw(0) << "(" ;
        TypedMixin::Dump()<< ", "; ImmediateMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        return std::cout << ";\n";
    } 
};
inline auto Inst::AsConst()
{
    return static_cast<ConstInst *>(this);
}
class AddInst : public FixedInputsInst<2>
{
public:
    template < typename unused = void>
    AddInst( )
    : FixedInputsInst<2>(ADD)
    {}

    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "ADD" << std::setw(0) << "(" ;
        ; 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        return std::cout << ";\n";
    } 
};
inline auto Inst::AsAdd()
{
    return static_cast<AddInst *>(this);
}
class SubInst : public FixedInputsInst<2>
{
public:
    template < typename unused = void>
    SubInst( )
    : FixedInputsInst<2>(SUB)
    {}

    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "SUB" << std::setw(0) << "(" ;
        ; 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        return std::cout << ";\n";
    } 
};
inline auto Inst::AsSub()
{
    return static_cast<SubInst *>(this);
}
class MulInst : public FixedInputsInst<2>
{
public:
    template < typename unused = void>
    MulInst( )
    : FixedInputsInst<2>(MUL)
    {}

    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "MUL" << std::setw(0) << "(" ;
        ; 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        return std::cout << ";\n";
    } 
};
inline auto Inst::AsMul()
{
    return static_cast<MulInst *>(this);
}
class PhiInst : public VariadicInputsInst
{
public:
    template < typename unused = void>
    PhiInst(size_t inputs_count )
    : VariadicInputsInst( inputs_count, PHI)
    {}

    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "PHI" << std::setw(0) << "(" ;
        ; 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        return std::cout << ";\n";
    } 
};
inline auto Inst::AsPhi()
{
    return static_cast<PhiInst *>(this);
}
class JmpInst : public FixedInputsInst<2>, public ConditionalMixin
{
public:
    template <typename Arg0, typename unused = void>
    JmpInst( Arg0 &&arg0)
    : FixedInputsInst<2>(JMP)
    , ConditionalMixin(arg0)
    {}

    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "JMP" << std::setw(0) << "(" ;
        ConditionalMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        return std::cout << ";\n";
    } 
};
inline auto Inst::AsJmp()
{
    return static_cast<JmpInst *>(this);
}
class ParameterInst : public FixedInputsInst<0>, public TypedMixin
{
public:
    template <typename Arg0, typename unused = void>
    ParameterInst( Arg0 &&arg0)
    : FixedInputsInst<0>(PARAMETER)
    , TypedMixin(arg0)
    {}

    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "PARAMETER" << std::setw(0) << "(" ;
        TypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        return std::cout << ";\n";
    } 
};
inline auto Inst::AsParameter()
{
    return static_cast<ParameterInst *>(this);
}
class ReturnInst : public FixedInputsInst<1>, public TypedMixin
{
public:
    template <typename Arg0, typename unused = void>
    ReturnInst( Arg0 &&arg0)
    : FixedInputsInst<1>(RETURN)
    , TypedMixin(arg0)
    {}

    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "RETURN" << std::setw(0) << "(" ;
        TypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        return std::cout << ";\n";
    } 
};
inline auto Inst::AsReturn()
{
    return static_cast<ReturnInst *>(this);
}

inline void Inst::Dump()
{
    switch (opcode_) {
    case CONST: AsConst()->Dump(); break;
    case ADD: AsAdd()->Dump(); break;
    case SUB: AsSub()->Dump(); break;
    case MUL: AsMul()->Dump(); break;
    case PHI: AsPhi()->Dump(); break;
    case JMP: AsJmp()->Dump(); break;
    case PARAMETER: AsParameter()->Dump(); break;
    case RETURN: AsReturn()->Dump(); break;
    default: UNREACHABLE();
    }
}


inline size_t Inst::GetInputsCount()
{
    if (IsFixed()) {
        return INPUTS_COUNT_ARRAY[opcode_]; 
    }

    return static_cast<VariadicInputsInst *>(this)->GetInputsCount(); 
}

inline Inst *Inst::GetInput(size_t i)
{
    switch (opcode_) {
    case CONST: return AsConst()->GetInput(i);
    case ADD: return AsAdd()->GetInput(i);
    case SUB: return AsSub()->GetInput(i);
    case MUL: return AsMul()->GetInput(i);
    case PHI: return AsPhi()->GetInput(i);
    case JMP: return AsJmp()->GetInput(i);
    case PARAMETER: return AsParameter()->GetInput(i);
    case RETURN: return AsReturn()->GetInput(i);
    default:
        UNREACHABLE();
    }
}
}
