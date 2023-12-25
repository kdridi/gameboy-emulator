#include <cart.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>
#include <string>
#include <filesystem>

using namespace testing;

namespace gaboem::testing
{
    class CartTest : public Test
    {
    public:
        static std::string GetPath(const std::string &path)
        {
            std::string fullPath(__FILE__);
            std::filesystem::path pathObj(fullPath);
            std::filesystem::path dirPath = pathObj.parent_path().parent_path().append(path);
            return dirPath.string();
        }
    };

    TEST(gaboem, cart_load_failure)
    {
        auto path = CartTest::GetPath("roms/00.gb");

        bool success = cart_load(path.c_str());
        ASSERT_THAT(success, Eq(false));
    }

    TEST(gaboem, cart_load_success)
    {
        auto path = CartTest::GetPath("roms/01.gb");

        bool success = cart_load(path.c_str());
        ASSERT_THAT(success, Eq(true));
    }

    TEST(gaboem, cart_read_nintendo_logo)
    {
        u8 logo[] = NINTENDO_LOGO;

        auto path = CartTest::GetPath("roms/01.gb");

        bool success = cart_load(path.c_str());
        ASSERT_THAT(success, Eq(true));

        ASSERT_THAT(sizeof(logo), Eq(0x30));

        for (u8 i = 0; i < sizeof(logo); i++)
            ASSERT_THAT(cart_read(0x104 + i), Eq(logo[i]));
    }
} // namespace gaboem::testing
