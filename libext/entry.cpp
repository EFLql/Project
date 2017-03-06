#include <libext/bootstrap/ServerBootstrap.h>

using namespace libext;
int main()
{
    ServerBootstrap server;
    server.bind(8888); 
}
