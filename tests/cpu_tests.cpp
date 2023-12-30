#include <stack.h>
#include <cpu.h>
#include <bus.h>
#include <emu.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>
#include <string>
#include <filesystem>

extern cpu_context ctx;

using namespace testing;

namespace gaboem::testing
{
    class CpuTest : public Test
    {
    public:
        void SetUp() override
        {
            emu_init();
            m_cpu->regs.pc = 0xC000;
        }

    protected:
        cpu_context *m_cpu = &ctx;
        emu_context *m_emu = emu_get_context();
    };

    TEST_F(CpuTest, execute_0x00) // NOP
    {
        u16 pc = m_cpu->regs.pc;
        bus_write(pc++, 0x00);
        cpu_step();
        ASSERT_THAT(m_cpu->regs.pc, Eq(pc));
        ASSERT_THAT(m_cpu->current_opcode, Eq(0x00));
        ASSERT_THAT(m_cpu->current_instruction->type, Eq(IN_NOP));
        ASSERT_THAT(m_emu->ticks, Eq(4));
    }

    TEST_F(CpuTest, execute_0x01) // LD BC, d16
    {
        u16 af = cpu_read_reg(RT_AF);
        u16 bc = cpu_read_reg(RT_BC);
        u16 de = cpu_read_reg(RT_DE);
        u16 hl = cpu_read_reg(RT_HL);
        u16 sp = cpu_read_reg(RT_SP);
        u16 pc = cpu_read_reg(RT_PC);
        bus_write(pc++, 0x01);
        bus_write(pc++, 0x34);
        bus_write(pc++, 0x12);
        cpu_step();
        ASSERT_THAT(cpu_read_reg(RT_PC), Eq(pc));
        ASSERT_THAT(m_cpu->current_opcode, Eq(0x01));
        ASSERT_THAT(m_cpu->current_instruction->type, Eq(IN_LD));
        ASSERT_THAT(m_cpu->current_instruction->mode, Eq(AM_R_D16));
        ASSERT_THAT(m_cpu->current_instruction->reg_1, Eq(RT_BC));
        ASSERT_THAT(m_cpu->current_instruction->reg_2, Eq(RT_NONE));
        ASSERT_THAT(m_cpu->current_instruction->cond, Eq(CT_NONE));
        ASSERT_THAT(m_cpu->current_instruction->param, Eq(0x00));
        ASSERT_THAT(m_cpu->fetched_data, Eq(0x1234));
        ASSERT_THAT(m_cpu->dest_is_mem, Eq(false));
        ASSERT_THAT(m_cpu->mem_dest, Eq(0x0000));
        ASSERT_THAT(m_cpu->regs.b, Eq(0x12));
        ASSERT_THAT(m_cpu->regs.c, Eq(0x34));
        ASSERT_THAT(cpu_read_reg(RT_BC), Eq(0x1234));
        ASSERT_THAT(cpu_read_reg(RT_AF), Eq(af));
        ASSERT_THAT(cpu_read_reg(RT_BC), Ne(bc));
        ASSERT_THAT(cpu_read_reg(RT_DE), Eq(de));
        ASSERT_THAT(cpu_read_reg(RT_HL), Eq(hl));
        ASSERT_THAT(cpu_read_reg(RT_SP), Eq(sp));
        ASSERT_THAT(m_emu->ticks, Eq(12));
    }
}
