
class ConstInst : public FixedInputsInst<0>, public TypedMixin, public ImmediateMixin
{
public:
    template <typename Arg0, typename Arg1, typename unused = void>
    ConstInst( Arg0 &&arg0, Arg1 &&arg1)
    : FixedInputsInst<0>(CONST)
    , TypedMixin(arg0)
    , ImmediateMixin(arg1)
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
class AddInst : public FixedInputsInst<2>
{
public:
    template < typename unused = void>
    AddInst( )
    : FixedInputsInst<2>(ADD)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "ADD" << std::setw(0) << "(" ;
        ; 
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
class SubInst : public FixedInputsInst<2>
{
public:
    template < typename unused = void>
    SubInst( )
    : FixedInputsInst<2>(SUB)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "SUB" << std::setw(0) << "(" ;
        ; 
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
class MulInst : public FixedInputsInst<2>
{
public:
    template < typename unused = void>
    MulInst( )
    : FixedInputsInst<2>(MUL)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "MUL" << std::setw(0) << "(" ;
        ; 
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
class PhiInst : public VariadicInputsInst
{
public:
    template < typename unused = void>
    PhiInst(size_t inputs_count )
    : VariadicInputsInst( inputs_count, PHI)
    {}

    template <bool DUMP_LIVENESS = false>
    auto &Dump() 
    {
        std::ios state(nullptr);
        state.copyfmt(std::cout);
        
        std::cout <<  "        " << std::setw(20) << std::left <<  "PHI" << std::setw(0) << "(" ;
        ; 
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
    template <typename Arg0, typename unused = void>
    JmpInst( Arg0 &&arg0)
    : FixedInputsInst<2>(JMP)
    , ConditionalMixin(arg0)
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
class ParameterInst : public FixedInputsInst<0>, public TypedMixin
{
public:
    template <typename Arg0, typename unused = void>
    ParameterInst( Arg0 &&arg0)
    : FixedInputsInst<0>(PARAMETER)
    , TypedMixin(arg0)
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
class ReturnInst : public FixedInputsInst<1>, public TypedMixin
{
public:
    template <typename Arg0, typename unused = void>
    ReturnInst( Arg0 &&arg0)
    : FixedInputsInst<1>(RETURN)
    , TypedMixin(arg0)
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

template <bool DUMP_LIVENESS>
inline void Inst::Dump()
{
    switch (opcode_) {
    case CONST: AsConst()->Dump<DUMP_LIVENESS>(); break;
    case ADD: AsAdd()->Dump<DUMP_LIVENESS>(); break;
    case SUB: AsSub()->Dump<DUMP_LIVENESS>(); break;
    case MUL: AsMul()->Dump<DUMP_LIVENESS>(); break;
    case PHI: AsPhi()->Dump<DUMP_LIVENESS>(); break;
    case JMP: AsJmp()->Dump<DUMP_LIVENESS>(); break;
    case PARAMETER: AsParameter()->Dump<DUMP_LIVENESS>(); break;
    case RETURN: AsReturn()->Dump<DUMP_LIVENESS>(); break;
    default: UNREACHABLE();
    }
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

inline Span<Inst *> Inst::GetInputs()
{
    switch (opcode_) {
    case CONST: return AsConst()->GetInputs();
    case ADD: return AsAdd()->GetInputs();
    case SUB: return AsSub()->GetInputs();
    case MUL: return AsMul()->GetInputs();
    case PHI: return AsPhi()->GetInputs();
    case JMP: return AsJmp()->GetInputs();
    case PARAMETER: return AsParameter()->GetInputs();
    case RETURN: return AsReturn()->GetInputs();
    default:
        UNREACHABLE();
    }
}
