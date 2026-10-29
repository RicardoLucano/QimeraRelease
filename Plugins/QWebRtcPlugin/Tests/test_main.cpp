#include <iostream>

int main() {
    std::cout << "===========================================" << std::endl;
    std::cout << "     QWebRtcPlugin TDD Standalone Tests     " << std::endl;
    std::cout << "===========================================" << std::endl;
    
    extern int RunWebRtcPluginTests();
    int result = RunWebRtcPluginTests();
    
    std::cout << "===========================================" << std::endl;
    std::cout << "Tests completed with code: " << result << std::endl;
    std::cout << "===========================================" << std::endl;
    return result;
}
