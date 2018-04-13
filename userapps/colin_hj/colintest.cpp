
#include <colin/ColinMain.h>

//
// A standard COLIN main utility, which links in the solvers that have
// been registered.
//
int main(int argc, char* argv[] )
{
return colin::colin_main(argc,argv,"colintest");
}
