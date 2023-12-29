#include <stack.h>
#include <cpu.h>
#include <bus.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>
#include <string>
#include <filesystem>

using namespace testing;

namespace gaboem::testing
{
    class StackTest : public Test
    {
    public:
        void SetUp() override
        {
            m_registers = cpu_get_registers();
        }

    protected:
        cpu_registers *m_registers;
    };

    TEST_F(StackTest, stack_push)
    {
        m_registers->sp = 0xFFFE;
        stack_push(0x01);
        ASSERT_THAT(m_registers->sp, Eq(0xFFFD));
        ASSERT_THAT(bus_read(0xFFFD), Eq(0x01));
    }

    TEST_F(StackTest, stack_push_twice)
    {
        m_registers->sp = 0xFFFE;
        stack_push(0x01);
        ASSERT_THAT(m_registers->sp, Eq(0xFFFD));
        ASSERT_THAT(bus_read(0xFFFD), Eq(0x01));
        stack_push(0x02);
        ASSERT_THAT(m_registers->sp, Eq(0xFFFC));
        ASSERT_THAT(bus_read(0xFFFC), Eq(0x02));
    }

    TEST_F(StackTest, stack_pop)
    {
        m_registers->sp = 0xFFFE;
        stack_push(0x01);
        ASSERT_THAT(stack_pop(), Eq(0x01));
        ASSERT_THAT(m_registers->sp, Eq(0xFFFE));
    }

    TEST_F(StackTest, stack_pop_twice)
    {
        m_registers->sp = 0xFFFE;
        stack_push(0x01);
        stack_push(0x02);
        ASSERT_THAT(stack_pop(), Eq(0x02));
        ASSERT_THAT(m_registers->sp, Eq(0xFFFD));
        ASSERT_THAT(stack_pop(), Eq(0x01));
        ASSERT_THAT(m_registers->sp, Eq(0xFFFE));
    }

    TEST_F(StackTest, stack_push16)
    {
        m_registers->sp = 0xFFFE;
        stack_push16(0x0102);
        ASSERT_THAT(m_registers->sp, Eq(0xFFFC));
        ASSERT_THAT(bus_read(0xFFFD), Eq(0x01));
        ASSERT_THAT(bus_read(0xFFFC), Eq(0x02));
    }

    TEST_F(StackTest, stack_pop16)
    {
        m_registers->sp = 0xFFFE;
        stack_push16(0x0102);
        ASSERT_THAT(stack_pop16(), Eq(0x0102));
        ASSERT_THAT(m_registers->sp, Eq(0xFFFE));
    }
}
