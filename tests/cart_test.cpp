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
} // namespace gaboem::testing
