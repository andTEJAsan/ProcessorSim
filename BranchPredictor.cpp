#include "BranchPredictor.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;
void tester(int x) {

    std::ifstream file("branchtrace.txt"); // open the file
    std::string line;
    struct SaturatingBranchPredictor sbp(x);
    struct BHRBranchPredictor bbp(x);
    struct SaturatingBHRBranchPredictor sbbp(x,1 << 16);
    int sbp_count=0;
    int bbp_count =0;
    int sbbp_count=0;
    int count=0;
    while (std::getline(file, line)) { // read the file line by line
        //std::cout << line[9]<<std::endl; // do something with each line
        uint32_t pc =static_cast<uint32_t>( std::stoul(line.substr(0,8),nullptr,16));
        bool taken;
        //std::cout <<std::bitset<32>(pc) << "\n";
        if (line[9]=='1') { taken = true; }
        else {taken =false;}
        if (taken == sbp.predict(pc)) {sbp_count++; }
        sbp.update(pc,taken);
        if (taken == bbp.predict(pc)) {bbp_count++; }
        bbp.update(pc,taken);
        if (taken == sbbp.predict(pc)) {sbbp_count++; }
        sbbp.update(pc,taken);
        count++;


    }
    float sbp_accuracy =(float)(sbp_count) / count;
    float bbp_accuracy = (float)(bbp_count) / count;
    float sbbp_accuracy = (float)(sbbp_count) / count;
    cout << "Initialization :- " << x << "\n";
    cout << "SBP Accuracy = " << sbp_accuracy << "\n";
    cout << "BBP Accuracy = " << bbp_accuracy << "\n";
    cout << "SBBP Accuracy = " << sbbp_accuracy << "\n";
    cout << "---------------------------------"<<"\n";
    file.close(); // close the file
 
}
int main() {
   tester(0);
   tester(1);
   tester(2);
   tester(3);
   return 0;
}
