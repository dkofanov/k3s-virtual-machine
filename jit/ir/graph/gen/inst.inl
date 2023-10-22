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
