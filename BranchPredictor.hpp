#ifndef __BRANCH_PREDICTOR_HPP__
#define __BRANCH_PREDICTOR_HPP__

#include <vector>
#include <bitset>
#include <cassert>
struct BranchPredictor {
    virtual bool predict(uint32_t pc) = 0;
    virtual void update(uint32_t pc, bool taken) = 0;
};

struct SaturatingBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> table;
    SaturatingBranchPredictor(int value) : table(1 << 14, value) {}
    
    bool predict(uint32_t pc) {
        // your code here
        std::bitset<2> x = table[pc % (1 << 14)];
        return x.test(1); // the leftmost bit is true
    }

    void update(uint32_t pc, bool taken) {
        // your code here
        if (taken)
        {
            std::bitset<2> x = table[pc % (1<<14)];
            int sum = 2*((int) x.test(1)) + (int) x.test(0);
            int new_sum = sum +1;
            if (new_sum == 4) new_sum = 3;
            x.set(0,(bool) (new_sum %2));
            x.set(1,(bool)(new_sum & 2));
            table[pc %(1<<14)] = x;
        return;
        }
        std::bitset<2> x = table[pc % (1<<14)];
        int new_sum = 0;
        int sum = 2*((int) x.test(1)) + (int) x.test(0);
        if ( sum == 0 ) new_sum = 0;
        else new_sum = sum - 1;
        x.set(0,(bool) (new_sum %2));
        x.set(1,(bool)(new_sum & 2));
        table[pc %(1<<14)] = x;       
    }
};

struct BHRBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> bhrTable;
    std::bitset<2> bhr;
    BHRBranchPredictor(int value) : bhrTable(1 << 2, value),bhr(value) {}
    bool predict(uint32_t pc) {
        // your code here
    int address = static_cast<int> (bhr.to_ulong()); 
    return (bhrTable[address]).test(1);
    }

    void update(uint32_t pc, bool taken) {
        // your code here
        //std::bitset<2> y;
        //y.set(1,taken);
        //y.set(0,bhr.test(1));
        //bhr= y; 
        if (taken) 
        {
            int address = static_cast<int> (bhr.to_ulong()); 
            std::bitset<2> counter = bhrTable[address];
            int sum = static_cast<int> (bhr.to_ulong());
            int new_sum = sum +1;
            if (new_sum == 4) new_sum = 3;
            counter.set(0,(bool) (new_sum %2));
            counter.set(1,(bool)(new_sum & 2));
            bhrTable[address]=counter;
        }
        else {
            int address = static_cast<int> (bhr.to_ulong()); 
            std::bitset<2> counter = bhrTable[address];
            int sum = static_cast<int> (counter.to_ulong());
            int new_sum;
            if (sum == 0) new_sum = 0;
            else new_sum = sum - 1;
            counter.set(0,(bool) (new_sum %2));
            counter.set(1,(bool)(new_sum & 2));
            bhrTable[address]=counter;
        }
        std::bitset<2> y;
        y.set(1, taken);
        y.set(0, bhr.test(1));
        bhr = y;
    } 
};

struct SaturatingBHRBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> bhrTable;
    std::bitset<2> bhr;
    std::vector<std::bitset<2>> table;
    std::vector<std::bitset<2>> combination;
    SaturatingBHRBranchPredictor(int value, int size) : bhrTable(1 << 2, value),bhr(value), table(1 << 14, value), combination(size, value) {
    assert(size <= (1 << 16));
    }
    
    // size == 1 << 16;
    bool predict(uint32_t pc) {
        // your code here
        // if predicted strongly not taken or strongly taken by the local branch predictor then follow that 
        // otherwise look at the global branch predictor if it is strongly taken or not taken then follow it else go to combination table
        std::bitset<2> locpredict = table[pc %(1<<14)];
        int loc_predict = static_cast<int> ((locpredict).to_ulong());
        if (loc_predict==3 || loc_predict==0) return locpredict.test(1);
        int address = static_cast<int> (bhr.to_ulong()); 
        std::bitset<2> bhrpredict = (bhrTable[address]);
        return bhrpredict.test(1);
    }


    void update(uint32_t pc, bool taken) {
        // your code here
        if (taken){
            std::bitset<2> x = table[pc % (1<<14)];
            int sum = 2*((int) x.test(1)) + (int) x.test(0);
            int new_sum = sum +1;
            if (new_sum == 4) new_sum = 3;
            x.set(0,(bool) (new_sum %2));
            x.set(1,(bool)(new_sum & 2));
            table[pc %(1<<14)] = x;

            int address = static_cast<int> (bhr.to_ulong()); 
            std::bitset<2> counter = bhrTable[address];
            sum = static_cast<int> (bhr.to_ulong());
            new_sum = sum +1;
            if (new_sum == 4) new_sum = 3;
            counter.set(0,(bool) (new_sum %2));
            counter.set(1,(bool)(new_sum & 2));
            bhrTable[address]=counter;

        }
        else {
        std::bitset<2> x = table[pc % (1<<14)];
        int new_sum = 0;
        int sum = 2*((int) x.test(1)) + (int) x.test(0);
        if ( sum == 0 ) new_sum = 0;
        else new_sum = sum - 1;
        x.set(0,(bool) (new_sum %2));
        x.set(1,(bool)(new_sum & 2));
        table[pc %(1<<14)] = x;


        int address = static_cast<int> (bhr.to_ulong()); 
            std::bitset<2> counter = bhrTable[address];
            sum = static_cast<int> (counter.to_ulong());
            if (sum == 0) new_sum = 0;
            else new_sum = sum - 1;
            counter.set(0,(bool) (new_sum %2));
            counter.set(1,(bool)(new_sum & 2));
            bhrTable[address]=counter;

        }
    
    std::bitset<2> y;
    y.set(1, taken);
    y.set(0, bhr.test(1));
    bhr = y;
    }

};

#endif