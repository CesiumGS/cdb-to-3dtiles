#define DOCTEST_CONFIG_IMPLEMENT
#include "CDBTo3DTiles.h"
#include <doctest/doctest.h>

using namespace CDBTo3DTiles;

int main(int argc, char** argv) {
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    int testCode = context.run();
    if(context.shouldExit())
        return testCode;
    
    int defaultCode = 0;
    return testCode + defaultCode;
}
