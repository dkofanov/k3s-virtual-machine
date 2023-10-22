
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
