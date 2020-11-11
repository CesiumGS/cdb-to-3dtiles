#define CATCH_CONFIG_RUNNER
#include "CDBTo3DTiles.h"
#include "catch2/catch.hpp"

using namespace CDBTo3DTiles;

int main(int argc, char *argv[])
{
    GlobalInitializer initializer;

    int result = Catch::Session().run(argc, argv);

    return result;
}
