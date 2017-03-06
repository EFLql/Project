//SocketAddr gtest cpp

#include <libext/SocketAddr.h>
#include <gtest/gtest.h>

using namespace libext;

TEST(SocketAddrTest, Basic)
{
    SocketAddr addr;
}

TEST(SocketAddrTest, setFromLocalPortTest)
{
    SocketAddr addr1;
    addr1.setFromLocalPort(8888);
    EXPECT_EQ(addr1.getSocketAddr().sin_port, htons(8888));
}

TEST(SocketAddrTest, Construct)
{
    SocketAddr addr1;
    addr1.setFromLocalPort(8888);

    SocketAddr addr2 = addr1;
    EXPECT_EQ(addr2.getSocketAddr().sin_port, htons(8888));

    SocketAddr addr3(addr1);
    EXPECT_EQ(addr3.getSocketAddr().sin_port, htons(8888));
   
    SocketAddr addr4(std::move(addr1));
    EXPECT_EQ(addr4.getSocketAddr().sin_port, htons(8888));

}

/*int main(int argc, char* argv[])
{
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}*/
