#ifndef INTERPRETER_INTERPRETER_H
#define INTERPRETER_INTERPRETER_H

#include "bytecode_instruction.h"
#include "register.h"
#include "allocator/allocator.h"
#include "classfile/class_file.h"
#include "types/coretypes.h"

namespace k3s {
class Stack
{
public:
    struct Frame
    {
    public:
        static constexpr size_t MAX_REGS_COUNT = 256;

        Frame(BytecodeInstruction *caller_pc, coretypes::Function *callee_obj)
        {
            caller_pc_ = caller_pc;
            callee_ = callee_obj;
        }

        static constexpr size_t SizeOf(size_t slots_n = MAX_REGS_COUNT)
        {
            return sizeof(Frame) + slots_n * sizeof(Register);
        }

        auto &Acc()
        {
            return acc_;
        }
        
        const auto &Acc() const
        {
            return acc_;
        }

        auto &Reg(size_t idx)
        {
            return regs_[idx];
        }

        const auto &Reg(size_t idx) const
        {
            return regs_[idx];
        }
        
        auto *CallerPc()
        {
            return caller_pc_;
        }

        auto *Callee()
        {
            return callee_;
        }
        
    private:
        BytecodeInstruction *caller_pc_ {};
        coretypes::Function *callee_ = nullptr;
        Register acc_ {};
        Register regs_[];
        // This is used for implicit this inside functions:
    };
    Stack()
    {
        auto ptr = Allocator().StackRegion().AllocBytes(Allocator().StackRegion().MAX_ALLOC_SIZE);
        fp_ = reinterpret_cast<class Frame *>(ptr);
        free_frames_count_ = Allocator().StackRegion().MAX_ALLOC_SIZE / Frame::SizeOf();
    }


    template <bool IS_FIRST_FRAME = false>
    void CreateFrame(BytecodeInstruction *caller_pc, coretypes::Function *callee_obj)
    {
        free_frames_count_--;
        ASSERT(free_frames_count_ != 0);
        char *target_fp = nullptr;   
        if constexpr(!IS_FIRST_FRAME) {
            ASSERT(caller_pc->GetOpcode() == Opcode::CALL);
            size_t slots_n = caller_pc->GetOperands();
            auto *target_fp = reinterpret_cast<char *>(fp_) + Frame::SizeOf(slots_n); 
        } else {
            ASSERT(caller_pc == nullptr);
            auto *target_fp = fp_; 
        }
        fp_ = new (target_fp) class Frame(caller_pc, callee_obj);
    }

    BytecodeInstruction *DestroyFrame()
    {
        free_frames_count_++;
        auto *ret_pc = fp_->CallerPc();
        ASSERT(ret_pc->GetOpcode() == Opcode::CALL);
        size_t slots_n = ret_pc->GetOperands();
        auto *prev_fp = reinterpret_cast<char *>(fp_) - Frame::SizeOf(slots_n);
        fp_->~Frame();
        fp_ = reinterpret_cast<class Frame *>(prev_fp);
        ASSERT(fp_->Callee()->GetTargetPc() < ret_pc);
        return ret_pc;
    }
    
    auto *Frame()
    {
        return fp_;
    }
    
    const auto *Frame() const
    {
        return fp_;
    }

private:
    class Frame *fp_{};
    size_t free_frames_count_{};
};

class Interpreter {
public:
    // Returns after execution of Opcode::RET with empty call stack
    int Invoke();

    void SetProgram(BytecodeInstruction *program)
    {
        program_ = program;
    }

    void SetPc(BytecodeInstruction *pc)
    {
        pc_ = pc;
    }

    const auto &Fetch() const
    {
        return *pc_;
    }

    using Type = Register::Type;

    template <Type REG_TYPE, Type... REG_TYPES, typename... RegsIds>
    bool CheckRegsType(size_t reg_id, RegsIds... regs_ids) const
    {
        static_assert(sizeof...(REG_TYPES) == sizeof...(RegsIds));

        const auto &reg = GetReg(reg_id);
        if constexpr (sizeof...(REG_TYPES) != 0) {
            if ((REG_TYPE == Type::ANY) || (reg.GetType() == REG_TYPE)){
                return CheckRegsType<REG_TYPES...>(regs_ids...);
            }
        } else {
            return true;
        }
        return false;
    }

    template <Type ACC_TYPE, Type... REG_TYPES, typename... RegsIds>
    bool CheckRegsTypeWithAcc(RegsIds... regs_ids) const
    {
        static_assert(sizeof...(REG_TYPES) == (sizeof...(RegsIds)));

        auto &acc = GetAcc();
        if ((ACC_TYPE == Type::ANY) || (acc.GetType() == ACC_TYPE)) {
            if constexpr (sizeof...(REG_TYPES) != 0) {
                return CheckRegsType<REG_TYPES...>(regs_ids...);
            }
            return true;
        }
        return false;
    }

    const Register &GetReg(size_t id) const
    {
        return state_stack_.Frame()->Reg(id);
    }
    Register &GetReg(size_t id)
    {
        return state_stack_.Frame()->Reg(id);
    }

    const Register &GetAcc() const
    {
        return const_cast<const Register &>(GetAcc());
    }
    Register &GetAcc()
    {
        return state_stack_.Frame()->Acc();
    }
    coretypes::Function *GetCallee()
    {
        return state_stack_.Frame()->Callee();
    }
    auto *GetStateStack()
    {
        return &state_stack_;
    }

private:
    BytecodeInstruction *pc_ {};
    Stack state_stack_;
    BytecodeInstruction *program_ {};
};

}  // namespace k3s 

#endif  // INTERPRETER_INTERPRETER_H

