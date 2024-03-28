
class ConstInst : public FixedInputsInst<0>, public TypedMixin, public ImmediateMixin
{
public:
    template <typename Arg_Typed, typename Arg_Immediate>
    ConstInst(Arg_Typed &&arg_Typed, Arg_Immediate &&arg_Immediate)
    : FixedInputsInst<0>(Opcode::CONST), TypedMixin(arg_Typed), ImmediateMixin(arg_Immediate)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "CONST" << std::setw(0) << "(" ;
        TypedMixin::Dump()<< ", "; ImmediateMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsConst()
{
    return static_cast<ConstInst *>(this);
}

inline auto Inst::AsConst() const
{
    return static_cast<const ConstInst *>(this);
}
class ParameterInst : public FixedInputsInst<0>, public TypedMixin
{
public:
    template <typename Arg_Typed>
    ParameterInst(Arg_Typed &&arg_Typed)
    : FixedInputsInst<0>(Opcode::PARAMETER), TypedMixin(arg_Typed)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "PARAMETER" << std::setw(0) << "(" ;
        TypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsParameter()
{
    return static_cast<ParameterInst *>(this);
}

inline auto Inst::AsParameter() const
{
    return static_cast<const ParameterInst *>(this);
}
class CastInst : public FixedInputsInst<1>, public TypedMixin
{
public:
    template <typename Arg_Typed>
    CastInst(Arg_Typed &&arg_Typed)
    : FixedInputsInst<1>(Opcode::CAST), TypedMixin(arg_Typed)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "CAST" << std::setw(0) << "(" ;
        TypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsCast()
{
    return static_cast<CastInst *>(this);
}

inline auto Inst::AsCast() const
{
    return static_cast<const CastInst *>(this);
}
class CallInst : public VariadicInputsInst, public ImplicitlyTypedMixin
{
public:
    
    CallInst(size_t inputs_count)
    : VariadicInputsInst(inputs_count, Opcode::CALL)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "CALL" << std::setw(0) << "(" ;
        ImplicitlyTypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsCall()
{
    return static_cast<CallInst *>(this);
}

inline auto Inst::AsCall() const
{
    return static_cast<const CallInst *>(this);
}
class ReturnInst : public FixedInputsInst<1>, public TypedMixin
{
public:
    template <typename Arg_Typed>
    ReturnInst(Arg_Typed &&arg_Typed)
    : FixedInputsInst<1>(Opcode::RETURN), TypedMixin(arg_Typed)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "RETURN" << std::setw(0) << "(" ;
        TypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsReturn()
{
    return static_cast<ReturnInst *>(this);
}

inline auto Inst::AsReturn() const
{
    return static_cast<const ReturnInst *>(this);
}
class ReturnVoidInst : public FixedInputsInst<0>
{
public:
    
    ReturnVoidInst()
    : FixedInputsInst<0>(Opcode::RETURNVOID)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "RETURNVOID" << std::setw(0) << "(" ;
        ; 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsReturnVoid()
{
    return static_cast<ReturnVoidInst *>(this);
}

inline auto Inst::AsReturnVoid() const
{
    return static_cast<const ReturnVoidInst *>(this);
}
class AddInst : public FixedInputsInst<2>, public ImplicitlyTypedMixin
{
public:
    
    AddInst()
    : FixedInputsInst<2>(Opcode::ADD)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "ADD" << std::setw(0) << "(" ;
        ImplicitlyTypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsAdd()
{
    return static_cast<AddInst *>(this);
}

inline auto Inst::AsAdd() const
{
    return static_cast<const AddInst *>(this);
}
class SubInst : public FixedInputsInst<2>, public ImplicitlyTypedMixin
{
public:
    
    SubInst()
    : FixedInputsInst<2>(Opcode::SUB)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "SUB" << std::setw(0) << "(" ;
        ImplicitlyTypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsSub()
{
    return static_cast<SubInst *>(this);
}

inline auto Inst::AsSub() const
{
    return static_cast<const SubInst *>(this);
}
class MulInst : public FixedInputsInst<2>, public ImplicitlyTypedMixin
{
public:
    
    MulInst()
    : FixedInputsInst<2>(Opcode::MUL)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "MUL" << std::setw(0) << "(" ;
        ImplicitlyTypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsMul()
{
    return static_cast<MulInst *>(this);
}

inline auto Inst::AsMul() const
{
    return static_cast<const MulInst *>(this);
}
class NegInst : public FixedInputsInst<1>, public ImplicitlyTypedMixin
{
public:
    
    NegInst()
    : FixedInputsInst<1>(Opcode::NEG)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "NEG" << std::setw(0) << "(" ;
        ImplicitlyTypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsNeg()
{
    return static_cast<NegInst *>(this);
}

inline auto Inst::AsNeg() const
{
    return static_cast<const NegInst *>(this);
}
class NotInst : public FixedInputsInst<1>, public ImplicitlyTypedMixin
{
public:
    
    NotInst()
    : FixedInputsInst<1>(Opcode::NOT)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "NOT" << std::setw(0) << "(" ;
        ImplicitlyTypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsNot()
{
    return static_cast<NotInst *>(this);
}

inline auto Inst::AsNot() const
{
    return static_cast<const NotInst *>(this);
}
class XorInst : public FixedInputsInst<2>, public ImplicitlyTypedMixin
{
public:
    
    XorInst()
    : FixedInputsInst<2>(Opcode::XOR)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "XOR" << std::setw(0) << "(" ;
        ImplicitlyTypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsXor()
{
    return static_cast<XorInst *>(this);
}

inline auto Inst::AsXor() const
{
    return static_cast<const XorInst *>(this);
}
class AshrInst : public FixedInputsInst<2>, public ImplicitlyTypedMixin
{
public:
    
    AshrInst()
    : FixedInputsInst<2>(Opcode::ASHR)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "ASHR" << std::setw(0) << "(" ;
        ImplicitlyTypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsAshr()
{
    return static_cast<AshrInst *>(this);
}

inline auto Inst::AsAshr() const
{
    return static_cast<const AshrInst *>(this);
}
class ShlInst : public FixedInputsInst<2>, public ImplicitlyTypedMixin
{
public:
    
    ShlInst()
    : FixedInputsInst<2>(Opcode::SHL)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "SHL" << std::setw(0) << "(" ;
        ImplicitlyTypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsShl()
{
    return static_cast<ShlInst *>(this);
}

inline auto Inst::AsShl() const
{
    return static_cast<const ShlInst *>(this);
}
class PhiInst : public VariadicInputsInst, public ImplicitlyTypedMixin
{
public:
    
    PhiInst(size_t inputs_count)
    : VariadicInputsInst(inputs_count, Opcode::PHI)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "PHI" << std::setw(0) << "(" ;
        ImplicitlyTypedMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsPhi()
{
    return static_cast<PhiInst *>(this);
}

inline auto Inst::AsPhi() const
{
    return static_cast<const PhiInst *>(this);
}
class JmpInst : public FixedInputsInst<2>, public ConditionalMixin
{
public:
    template <typename Arg_Conditional>
    JmpInst(Arg_Conditional &&arg_Conditional)
    : FixedInputsInst<2>(Opcode::JMP), ConditionalMixin(arg_Conditional)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "JMP" << std::setw(0) << "(" ;
        ConditionalMixin::Dump(); 
        std::cout << ")" << std::setw(15) << std::right << " v" << Id() << std::setw(0);
        DumpDF();
        std::cout.copyfmt(state);
        std::cout << ";  // LN = " << LN() << '\n';

        return std::cout;
    } 
};

inline auto Inst::AsJmp()
{
    return static_cast<JmpInst *>(this);
}

inline auto Inst::AsJmp() const
{
    return static_cast<const JmpInst *>(this);
}

template <bool DUMP_LIVENESS>
inline void Inst::Dump()
{
    switch (opcode_) {
    case CONST: AsConst()->Dump<DUMP_LIVENESS>(); break;
    case PARAMETER: AsParameter()->Dump<DUMP_LIVENESS>(); break;
    case CAST: AsCast()->Dump<DUMP_LIVENESS>(); break;
    case CALL: AsCall()->Dump<DUMP_LIVENESS>(); break;
    case RETURN: AsReturn()->Dump<DUMP_LIVENESS>(); break;
    case RETURNVOID: AsReturnVoid()->Dump<DUMP_LIVENESS>(); break;
    case ADD: AsAdd()->Dump<DUMP_LIVENESS>(); break;
    case SUB: AsSub()->Dump<DUMP_LIVENESS>(); break;
    case MUL: AsMul()->Dump<DUMP_LIVENESS>(); break;
    case NEG: AsNeg()->Dump<DUMP_LIVENESS>(); break;
    case NOT: AsNot()->Dump<DUMP_LIVENESS>(); break;
    case XOR: AsXor()->Dump<DUMP_LIVENESS>(); break;
    case ASHR: AsAshr()->Dump<DUMP_LIVENESS>(); break;
    case SHL: AsShl()->Dump<DUMP_LIVENESS>(); break;
    case PHI: AsPhi()->Dump<DUMP_LIVENESS>(); break;
    case JMP: AsJmp()->Dump<DUMP_LIVENESS>(); break;
    default: UNREACHABLE();
    }
}

template <bool NEED_UPDATE_USER>
inline void Inst::SetInput(size_t i, Inst *inst)
{
    switch (opcode_) {
    case CONST:
    {
        AsConst()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case PARAMETER:
    {
        AsParameter()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case CAST:
    {
        AsCast()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case CALL:
    {
        AsCall()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case RETURN:
    {
        AsReturn()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case RETURNVOID:
    {
        AsReturnVoid()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case ADD:
    {
        AsAdd()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case SUB:
    {
        AsSub()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case MUL:
    {
        AsMul()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case NEG:
    {
        AsNeg()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case NOT:
    {
        AsNot()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case XOR:
    {
        AsXor()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case ASHR:
    {
        AsAshr()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case SHL:
    {
        AsShl()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case PHI:
    {
        AsPhi()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    case JMP:
    {
        AsJmp()->SetInput<NEED_UPDATE_USER>(i, inst);
        return;
    }
    default:
        UNREACHABLE();
    }
}

inline Inst *Inst::GetInput(size_t i)
{
    switch (opcode_) {
    case CONST: return AsConst()->GetInput(i);
    case PARAMETER: return AsParameter()->GetInput(i);
    case CAST: return AsCast()->GetInput(i);
    case CALL: return AsCall()->GetInput(i);
    case RETURN: return AsReturn()->GetInput(i);
    case RETURNVOID: return AsReturnVoid()->GetInput(i);
    case ADD: return AsAdd()->GetInput(i);
    case SUB: return AsSub()->GetInput(i);
    case MUL: return AsMul()->GetInput(i);
    case NEG: return AsNeg()->GetInput(i);
    case NOT: return AsNot()->GetInput(i);
    case XOR: return AsXor()->GetInput(i);
    case ASHR: return AsAshr()->GetInput(i);
    case SHL: return AsShl()->GetInput(i);
    case PHI: return AsPhi()->GetInput(i);
    case JMP: return AsJmp()->GetInput(i);
    default:
        UNREACHABLE();
    }
}
inline User *Inst::GetUserPointee(size_t i)
{
    switch (opcode_) {
    case CONST: return AsConst()->GetUserPointee(i);
    case PARAMETER: return AsParameter()->GetUserPointee(i);
    case CAST: return AsCast()->GetUserPointee(i);
    case CALL: return AsCall()->GetUserPointee(i);
    case RETURN: return AsReturn()->GetUserPointee(i);
    case RETURNVOID: return AsReturnVoid()->GetUserPointee(i);
    case ADD: return AsAdd()->GetUserPointee(i);
    case SUB: return AsSub()->GetUserPointee(i);
    case MUL: return AsMul()->GetUserPointee(i);
    case NEG: return AsNeg()->GetUserPointee(i);
    case NOT: return AsNot()->GetUserPointee(i);
    case XOR: return AsXor()->GetUserPointee(i);
    case ASHR: return AsAshr()->GetUserPointee(i);
    case SHL: return AsShl()->GetUserPointee(i);
    case PHI: return AsPhi()->GetUserPointee(i);
    case JMP: return AsJmp()->GetUserPointee(i);
    default:
        UNREACHABLE();
    }
}
inline Span<Inst *> Inst::GetInputs()
{
    switch (opcode_) {
    case CONST: return AsConst()->GetInputs();
    case PARAMETER: return AsParameter()->GetInputs();
    case CAST: return AsCast()->GetInputs();
    case CALL: return AsCall()->GetInputs();
    case RETURN: return AsReturn()->GetInputs();
    case RETURNVOID: return AsReturnVoid()->GetInputs();
    case ADD: return AsAdd()->GetInputs();
    case SUB: return AsSub()->GetInputs();
    case MUL: return AsMul()->GetInputs();
    case NEG: return AsNeg()->GetInputs();
    case NOT: return AsNot()->GetInputs();
    case XOR: return AsXor()->GetInputs();
    case ASHR: return AsAshr()->GetInputs();
    case SHL: return AsShl()->GetInputs();
    case PHI: return AsPhi()->GetInputs();
    case JMP: return AsJmp()->GetInputs();
    default:
        UNREACHABLE();
    }
}
