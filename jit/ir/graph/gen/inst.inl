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
    auto AsConst() const;
    auto AsConst();
    bool IsAdd() const { return opcode_ == ADD; }
    auto AsAdd() const;
    auto AsAdd();
    bool IsSub() const { return opcode_ == SUB; }
    auto AsSub() const;
    auto AsSub();
    bool IsMul() const { return opcode_ == MUL; }
    auto AsMul() const;
    auto AsMul();
    bool IsPhi() const { return opcode_ == PHI; }
    auto AsPhi() const;
    auto AsPhi();
    bool IsJmp() const { return opcode_ == JMP; }
    auto AsJmp() const;
    auto AsJmp();
    bool IsParameter() const { return opcode_ == PARAMETER; }
    auto AsParameter() const;
    auto AsParameter();
    bool IsReturn() const { return opcode_ == RETURN; }
    auto AsReturn() const;
    auto AsReturn();

    Inst(Opcode opcode);

    Inst *GetInput(size_t i);
    Span<Inst *> GetInputs();
    size_t GetInputsCount();

    template <bool DUMP_LIVENESS = false>
    void Dump();

    auto GetOpcode()
    {
        return opcode_;
    }

    void SetNextPrev(Inst *next)
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
    uint32_t id_{};
    uint32_t ln_{};
    BasicBlock *bb_{};
    User *first_user_{};
    Inst *prev_{};
    Inst *next_{};

friend class User;
};
