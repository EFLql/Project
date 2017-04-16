#include <libext/bootstrap/ServerBootstrap.h>

using namespace libext;
int main()
{
    ServerBootstrap<DefaultPipeline> server;
    server.bind(8080); 
    sleep(5000);
}
