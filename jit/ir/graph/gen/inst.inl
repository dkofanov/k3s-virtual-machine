class Inst {
public:
    enum Opcode {
        NONE,
        CONST,
        PARAMETER,
        CAST,
        RETURN,
        RETURNVOID,
        ADD,
        SUB,
        MUL,
        NEG,
        NOT,
        XOR,
        ASHR,
        SHL,
        PHI,
        JMP,
    };

    void ReplaceWith(Inst *inst)
    {
        ClearInputs();
        ReplaceInBB(inst);
        ReplaceUsersTo(inst);
    }
    
    bool HasDst()
    {
        switch (opcode_) {
        case RETURN:
        case RETURNVOID:
        case JMP:
            return false;
        default:
            return true;
        }
    }

    bool IsConst() const { return opcode_ == CONST; }
    auto AsConst() const;
    auto AsConst();
    bool IsParameter() const { return opcode_ == PARAMETER; }
    auto AsParameter() const;
    auto AsParameter();
    bool IsCast() const { return opcode_ == CAST; }
    auto AsCast() const;
    auto AsCast();
    bool IsReturn() const { return opcode_ == RETURN; }
    auto AsReturn() const;
    auto AsReturn();
    bool IsReturnVoid() const { return opcode_ == RETURNVOID; }
    auto AsReturnVoid() const;
    auto AsReturnVoid();
    bool IsAdd() const { return opcode_ == ADD; }
    auto AsAdd() const;
    auto AsAdd();
    bool IsSub() const { return opcode_ == SUB; }
    auto AsSub() const;
    auto AsSub();
    bool IsMul() const { return opcode_ == MUL; }
    auto AsMul() const;
    auto AsMul();
    bool IsNeg() const { return opcode_ == NEG; }
    auto AsNeg() const;
    auto AsNeg();
    bool IsNot() const { return opcode_ == NOT; }
    auto AsNot() const;
    auto AsNot();
    bool IsXor() const { return opcode_ == XOR; }
    auto AsXor() const;
    auto AsXor();
    bool IsAshr() const { return opcode_ == ASHR; }
    auto AsAshr() const;
    auto AsAshr();
    bool IsShl() const { return opcode_ == SHL; }
    auto AsShl() const;
    auto AsShl();
    bool IsPhi() const { return opcode_ == PHI; }
    auto AsPhi() const;
    auto AsPhi();
    bool IsJmp() const { return opcode_ == JMP; }
    auto AsJmp() const;
    auto AsJmp();

    Inst(Opcode opcode);
    
    template <bool NEED_UPDATE_USER = true>
    void SetInput(size_t i, Inst *inst);

    Inst *GetInput(size_t i);
    User *GetUserPointee(size_t i);
    Span<Inst *> GetInputs();
    size_t GetInputsCount();

    template <bool DUMP_LIVENESS = false>
    void Dump();

    auto GetOpcode()
    {
        return opcode_;
    }
    bool Equal(Inst *inst) const
    {
        // TODO: implement via GVN
        return this == inst;
    }

    void Prepend(Inst *inst);
    void SetNext(Inst *next)
    {
        next_ = next;
    }
    void SetPrev(Inst *prev)
    {
        prev_ = prev;
    }
    Inst *Next()
    {
        return next_;
    }
    Inst *Prev()
    {
        return prev_;
    }

    void SetLifeNumber(uint32_t ln)
    {
        ln_ = ln;
    }
    auto LN() const
    {
        return ln_;
    }
    
    auto Id() const
    {
        return id_;
    }
    
    auto BB() { return bb_; }
    size_t GetBBId();
    void SetBB(BasicBlock *bb) { bb_ = bb; }

    bool HasUsers()
    {
        return first_user_ != nullptr;
    }

    auto *FirstUser() const
    {
        return first_user_;
    }

private:
    void RemoveUser(User *user);
    void ClearInputs();
    void ReplaceInBB(Inst *inst);
    void ReplaceUsersTo(Inst *inst);

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
 (uint8_t) 0,  (uint8_t) 0,  (uint8_t) 1,  (uint8_t) 1,  (uint8_t) 0,  (uint8_t) 2,  (uint8_t) 2,  (uint8_t) 2,  (uint8_t) 1,  (uint8_t) 1,  (uint8_t) 2,  (uint8_t) 2,  (uint8_t) 2,  (uint8_t) -1,  (uint8_t) 2,     
    };

private:
    Opcode opcode_{};
    uint32_t id_{};
    uint32_t ln_{};
    BasicBlock *bb_{};
    User *first_user_{};
    Inst *prev_{};
    Inst *next_{};

friend class User;
};
