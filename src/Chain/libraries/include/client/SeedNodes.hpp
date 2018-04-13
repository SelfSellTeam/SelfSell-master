#pragma once 
namespace thinkyoung { 
    namespace client { 
#ifndef ALP_TEST_NETWORK 
    static const std::vector<std::string> SeedNodes = {
"18.197.85.175:61896","54.177.186.21:61896","23.97.71.16:61896"
 }; 
#else 
 static const std::vector<std::string> SeedNodes { }; 
#endif
} 
} // thinkyoung::client 
