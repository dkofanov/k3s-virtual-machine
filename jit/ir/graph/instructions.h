
#pragma once
#include <iostream>
#include <iomanip>

namespace compiler {

class BasicBlock;
class User;

#include "gen/inst.inl"

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
    
    auto GetInputs()
    {
        return Span<Inst *>(inputs_);
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
    std::array<Inst*, INPUTS_COUNT> inputs_ {};
    std::array<User, INPUTS_COUNT + 1> users_ {};
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

    auto GetInputs()
    {
        return Span<Inst *>();
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
};

// Currently, dynamic allocations occurs exactly 1 time (on initialization):
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

    auto GetInputs() {
        return Span<Inst *>(inputs_.begin(), inputs_.size());
    }

private:
    std::vector<Inst *> inputs_ {};
    std::vector<User> users_ {};
};

#include "gen/instructions.inl"

inline size_t Inst::GetInputsCount()
{
    if (IsFixed()) {
        return INPUTS_COUNT_ARRAY[opcode_]; 
    }

    return static_cast<VariadicInputsInst *>(this)->GetInputsCount(); 
}

}
