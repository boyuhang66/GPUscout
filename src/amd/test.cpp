#include "parser_metrics.hpp"

#include <unordered_map>
#include <iostream>
#include <vector>
#include <string>

int main(int argc, char **argv)
{
    
    auto mtc_map = parser_metrics(argv[1]);
    
    for (const auto& [krn_name, mtc_obj] : mtc_map) {
        std::cout << "7.2.4 " << mtc_obj.ID_7_2_4 << std::endl;
	std::cout << "10_1_6 " << mtc_obj.ID_10_1_6 << std::endl;
	std::cout << "10_2_14 " << mtc_obj.ID_10_2_14 << std::endl;
        std::cout << "11_2_5 " << mtc_obj.ID_11_2_5 << std::endl;
        std::cout << "12_1_0 " << mtc_obj.ID_12_1_0 << std::endl;
        std::cout << "12_1_1 " << mtc_obj.ID_12_1_1 << std::endl;
        std::cout << "12_1_3 " << mtc_obj.ID_12_1_3 << std::endl;
        std::cout << "12_2_5 " << mtc_obj.ID_12_2_5 << std::endl;
        std::cout << "15_1_9 " << mtc_obj.ID_15_1_9 << std::endl;
        std::cout << "15_1_13 " << mtc_obj.ID_15_1_13 << std::endl;
        std::cout << "16_3_3 " << mtc_obj.ID_16_3_3 << std::endl;
        std::cout << "16_3_5 " << mtc_obj.ID_16_3_5 << std::endl;
        std::cout << "17_2_7 " << mtc_obj.ID_17_2_7 << std::endl;
        std::cout << "17_3_1 " << mtc_obj.ID_17_3_1 << std::endl;
        std::cout << "17_3_1 " << mtc_obj.ID_17_3_4 << std::endl;
        std::cout << "17_5_10 " << mtc_obj.ID_17_5_10 << std::endl; 
    } 
    
    return 0;
}
