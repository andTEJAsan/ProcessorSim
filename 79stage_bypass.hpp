/**
 * @file MIPS_Processor.hpp
 * @author Mallika Prabhakar and Sayam Sethi
 *
 */

#ifndef __MIPS_PROCESSOR_HPP__
#define __MIPS_PROCESSOR_HPP__
using namespace std;
#include <unordered_map>
#include <string>
#include <functional>
#include <vector>
#include <fstream>
#include <exception>
#include <iostream>
#include <boost/tokenizer.hpp>
using namespace std ; 

struct MIPS_Architecture
{
	struct WBctr{
		bool RegWrite;
		bool MemtoReg;

		void print(struct WBctr* ptr){
			if (ptr == NULL){return ; }
			cout << "RegWrite" << ptr->RegWrite  << "\n" ; 
			cout << "MemtoReg" << ptr->MemtoReg  << "\n" ; 
		}
	};
	
	struct MEMctr2{
		bool MemWrite;
		void print(struct MEMctr2* ptr){
			if (ptr == NULL){return ; }
			cout << "MemWrite" << ptr->MemWrite  << "\n" ; 
		}

	};

    struct MEMctr1{
		bool MemRead;
		void print(struct MEMctr1* ptr){
			if (ptr == NULL){return ; }
			cout << "MemRead" << ptr->MemRead  << "\n" ; 
		}

	};
	struct EXctr{
		bool branch;
		bool branch_not ; 
		bool RegDst;
		int OP; // 0 denotes add , 1 denotes subtraction, 2 denotes multiplication, 3 denotes slt
		bool ALUSrc ; // true if alu src has to be immediate field

		void print(struct EXctr* ptr){
			if (ptr == NULL){return ; }
			cout << "branch" << ptr->branch  << "\n" ; 
			cout << "branch_not" << ptr->branch_not << "\n" ; 
			cout << "RegDst" << ptr->RegDst  << "\n" ; 
			cout << "OP" << ptr->OP  << "\n" ; 
			cout << "ALUSrc" << ptr->ALUSrc  << "\n" ; 
		}

	};
	typedef struct WBctr WBctr;
	typedef struct MEMctr1 MEMctr1;
    typedef struct MEMctr2 MEMctr2 ; 
	typedef struct EXctr EXctr;
	struct PipelineReg
    {
        std::vector<string> currcomand;
        int at_PC = -1;
        int PC_new = -1;
    /*control signals */
        WBctr* WB = NULL;
        MEMctr1* MEM1 = NULL;
        MEMctr2* MEM2 = NULL ; 
        EXctr* EX = NULL;
    /*saved results */
        int ALURESULT;
        int Read_data;
        int immediate;
        bool zero = (ALURESULT == 0) ;
        int RegReadData1;
        int RegReadData2;

        int RS,RT,RD;
        bool is_9_stage ; 

        void print(struct  PipelineReg* ptr)  {
            if (ptr == NULL){return ; }
            cout << "at_PC" << ptr->at_PC << "\n" ; 
            cout << "PC_new" << ptr->PC_new << "\n" ;
            cout << "ALURESULT" << ptr->ALURESULT << "\n" ;
            cout << "Read_data" << ptr->Read_data << "\n" ;
            cout << "immediate" << ptr->immediate << "\n" ;
            cout << "zero" << ptr->zero << "\n" ;
            cout << "RegReadData1" << ptr->RegReadData1 << "\n" ;
            cout << "RegReadData2" << ptr->RegReadData2 << "\n" ;
            cout << "RS" << ptr->RS << "\n" ;
            cout << "RT" << ptr->RT << "\n" ;
            cout << "RD" << ptr->RD << "\n" ;
            cout << "is_9_stage " << ptr->is_9_stage << "\n" ;
            cout << "printing control signals" << '\n' ;
            
            if (ptr->EX)  (ptr->EX)->print(ptr->EX) ;
            if (ptr->MEM1) (ptr->MEM1)->print(ptr->MEM1) ; 
            if (ptr->MEM2) (ptr->MEM2)->print(ptr->MEM2) ; 
            if (ptr->WB) (ptr->WB)->print(ptr->WB) ; 
            
        }
    };
	typedef struct PipelineReg pr;

    struct  seven_nine_stage_pr : public pr {
        bool is_9_stage ; 
        void print(struct   seven_nine_stage_pr* ptr)  {
            if (ptr == NULL){return ; }
            cout << "at_PC " << ptr->at_PC << "\n" ; 
            cout << "PC_new " << ptr->PC_new << "\n" ;
            cout << "ALURESULT " << ptr->ALURESULT << "\n" ;
            cout << "Read_data " << ptr->Read_data << "\n" ;
            cout << "immediate " << ptr->immediate << "\n" ;
            cout << "zero " << ptr->zero << "\n" ;
            cout << "RegReadData1 " << ptr->RegReadData1 << "\n" ;
            cout << "RegReadData2 " << ptr->RegReadData2 << "\n" ;
            cout << "RS " << ptr->RS << "\n" ;
            cout << "RT " << ptr->RT << "\n" ;
            cout << "RD " << ptr->RD << "\n" ;
            cout << "is_9_stage " << ptr->is_9_stage << "\n" ; 
            cout << "printing control signals " << '\n' ;
            
            if (ptr->EX)  (ptr->EX)->print(ptr->EX) ;
            if (ptr->MEM1) (ptr->MEM1)->print(ptr->MEM1) ; 
            if (ptr->MEM2) (ptr->MEM2)->print(ptr->MEM2) ; 
            if (ptr->WB) (ptr->WB)->print(ptr->WB) ; 
            
        }
    } ; 

	// struct check {
	// 	int rd ; 
	// 	int at_pc ; 
	// 	bool regwrite ; 
	// } ;
	// typedef struct check check ; 

    typedef struct  seven_nine_stage_pr   seven_nine_stage_pr ; 

	int registers[32] = {0}, PCcurr = 0, PCnext;
	std::unordered_map<std::string, std::function<int(MIPS_Architecture &, std::string, std::string, std::string)>> instructions;
	std::unordered_map<std::string, int> registerMap, address;
	std::unordered_map<int, int> memoryDelta;
	static const int MAX = (1 << 20);
	int data[MAX >> 2] = {0};
	std::vector<std::vector<std::string>> commands;
	std::vector<int> commandCount;
	

	// constructor to initialise the instruction set
	MIPS_Architecture(std::ifstream &file)
	{

		for (int i = 0; i < 32; ++i)
			registerMap["$" + std::to_string(i)] = i;
		registerMap["$zero"] = 0;
		registerMap["$at"] = 1;
		registerMap["$v0"] = 2;
		registerMap["$v1"] = 3;
		for (int i = 0; i < 4; ++i)
			registerMap["$a" + std::to_string(i)] = i + 4;
		for (int i = 0; i < 8; ++i)
			registerMap["$t" + std::to_string(i)] = i + 8, registerMap["$s" + std::to_string(i)] = i + 16;
		registerMap["$t8"] = 24;
		registerMap["$t9"] = 25;
		registerMap["$k0"] = 26;
		registerMap["$k1"] = 27;
		registerMap["$gp"] = 28;
		registerMap["$sp"] = 29;
		registerMap["$s8"] = 30;
		registerMap["$ra"] = 31;

		constructCommands(file);
		commandCount.assign(commands.size(), 0);
	}

	// parse the command assuming correctly formatted MIPS instruction (or label)
	void parseCommand(std::string line)
	{
		// strip until before the comment begins
		line = line.substr(0, line.find('#'));
		std::vector<std::string> command;
		boost::tokenizer<boost::char_separator<char>> tokens(line, boost::char_separator<char>(", \t"));
		for (auto &s : tokens)
			command.push_back(s);
		// empty line or a comment only line
		if (command.empty())
			return;
		else if (command.size() == 1)
		{
			std::string label = command[0].back() == ':' ? command[0].substr(0, command[0].size() - 1) : "?";
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command.clear();
		}
		else if (command[0].back() == ':')
		{
			std::string label = command[0].substr(0, command[0].size() - 1);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command = std::vector<std::string>(command.begin() + 1, command.end());
		}
		else if (command[0].find(':') != std::string::npos)
		{
			int idx = command[0].find(':');
			std::string label = command[0].substr(0, idx);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command[0] = command[0].substr(idx + 1);
		}
		else if (command[1][0] == ':')
		{
			if (address.find(command[0]) == address.end())
				address[command[0]] = commands.size();
			else
				address[command[0]] = -1;
			command[1] = command[1].substr(1);
			if (command[1] == "")
				command.erase(command.begin(), command.begin() + 2);
			else
				command.erase(command.begin(), command.begin() + 1);
		}
		if (command.empty())
			return;
		if (command.size() > 4)
			for (int i = 4; i < (int)command.size(); ++i)
				command[3] += " " + command[i];
		command.resize(4);
		commands.push_back(command);
	}

	// construct the commands vector from the input file
	void constructCommands(std::ifstream &file)
	{
		std::string line;
		while (getline(file, line))
			parseCommand(line);
		file.close();
		// for (auto i = commands.begin() ; i != commands.end() ; i ++){
		// 	print_vector(*i)  ; 
		// }
	}

	

	void executePipelined(){
		int clockCycles=0; WBctr wbarr[8] ; EXctr exarr[8] ; MEMctr1 mem1arr[8] ; MEMctr2 mem2arr[8] ; 
		
        WBctr wbctr1 = {.RegWrite = false , .MemtoReg = false} ; 
        EXctr exctr1 = {.branch = false, .branch_not = false,  .RegDst = false , .OP = 0, .ALUSrc = false  } ; 
		MEMctr2 memctr2 = {   .MemWrite = false } ; 
        MEMctr1 memctr1 = {  .MemRead = false } ; 
        for (int i = 0 ; i < 8 ; i ++){
            wbarr[i] = wbctr1 ; exarr[i] = exctr1 ; mem1arr[i] = memctr1 ; mem2arr[i] = memctr2 ; 
        }
		vector<string> str = {"0"}  ; 
		pr IF1_IF2 = {.currcomand = str, .at_PC = -1, .PC_new = -1, .WB = &(wbarr[0]), .MEM1 = &(mem1arr[0]), .MEM2 = &(mem2arr[0]) , .EX = &(exarr[0]) , 
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false } ; 

		pr IF2_ID1 = {.currcomand = str, .at_PC = -1, .PC_new = -1,  .WB = &(wbarr[1]), .MEM1 = &(mem1arr[1]), .MEM2 = &(mem2arr[1]) , .EX = &(exarr[1]) , 
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 

		pr ID1_ID2 = {.currcomand = str, .at_PC = -1, .PC_new = -1,  .WB = &(wbarr[2]), .MEM1 = &(mem1arr[2]), .MEM2 = &(mem2arr[2]) , .EX = &(exarr[2]) , 
					 .ALURESULT = 0, .Read_data =  0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 

		pr ID2_RR = {.currcomand = str, .at_PC = -1, .PC_new = -1,  .WB = &(wbarr[3]), .MEM1 = &(mem1arr[3]), .MEM2 = &(mem2arr[3]) , .EX = &(exarr[3]) , 
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 
        pr RR_EX = {.currcomand = str, .at_PC = -1, .PC_new = -1,  .WB = &(wbarr[4]), .MEM1 = &(mem1arr[4]), .MEM2 = &(mem2arr[4]) , .EX = &(exarr[4]) , 
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 

		pr EX_DM1 = {.currcomand = str, .at_PC = -1, .PC_new = -1,  .WB = &(wbarr[5]), .MEM1 = &(mem1arr[5]), .MEM2 = &(mem2arr[5]) , .EX = &(exarr[5]) , 
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 

		pr DM1_DM2 = {.currcomand = str, .at_PC = -1, .PC_new = -1, .WB = &(wbarr[6]), .MEM1 = &(mem1arr[6]), .MEM2 = &(mem2arr[6]) , .EX = &(exarr[6]) , 
					 .ALURESULT = 0, .Read_data =  0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 

		pr DM2_WB = {.currcomand = str, .at_PC = -1, .PC_new = -1,  .WB = &(wbarr[7]), .MEM1 = &(mem1arr[7]), .MEM2 = &(mem2arr[7]) , .EX = &(exarr[7]) , 
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 
     pr* dummy ; // pointer to a pipeline register
		

		bool is_stall = false ; bool is_beq_stall = false ; bool is_j_stall = false ; bool is_fifo_stall = false ; 
		int prev_PC = -1 ; bool prev_is_9_stage = false ; // saving values of RR stage for fifo stall detection in ID1 stage
		bool check_rs = false ; bool check_rt = false ; // setting rs and rt for checking

		bool if1_if2_write_enable = true ; bool if2_id1_write_enable = true ;
		bool id1_id2_write_enable = true ; bool id2_rr_write_enable = true ;
		string rs, rt, rd ; 

        int RW_pc = -1 ; int RW_rd = -1 ; bool RW_regwrite = false ; // saved values of RW
		int DM1_pc = -1 ; int DM1_rd = -1 ; bool DM1_regwrite = false ; // saved values of RW
        int op1, op2 ; // chooses  between forwarded value and og value in EX stage
		int write_data ; // chooses between forwarded value and og value in DM2 stage
		int lparen, offset ; 
		
		printRegistersAndMemoryDelta(0) ; 
		while ( !(PCcurr == -1 && IF1_IF2.at_PC == -1 && IF2_ID1.at_PC == -1 && ID1_ID2.at_PC == -1 && 
						 		ID2_RR.at_PC == -1 && RR_EX.at_PC == -1 && EX_DM1.at_PC == -1 && 
                                DM1_DM2.at_PC == -1 && DM2_WB.at_PC == -1)) // if pc < 0 then a stall will occur
		{		
			// cout << "current pc is " << PCcurr << '\n' ; 
			
			
			++clockCycles;
			// cout << "clock cycle - " << clockCycles << "\n" ; 
			if (PCcurr == -1){PCnext = -1 ; }
			else PCnext = PCcurr + 1 ;  

			  

			 
			// write back (during first half )	// rest stages are in second half cycle 
             
			if(DM2_WB.at_PC >= 0 && (DM2_WB.WB)->RegWrite && DM2_WB.RD != 0){
				
				
				if ((DM2_WB.WB)->MemtoReg)  registers[DM2_WB.RD] = DM2_WB.Read_data;
				else registers[DM2_WB.RD] = DM2_WB.ALURESULT;

				 
			};
            
            RW_pc = DM2_WB.at_PC ; 
            RW_rd = DM2_WB.RD ; 
            RW_regwrite = (DM2_WB.WB)->RegWrite ; 
			
			// DM2 stage (memwrite)
			
			if (DM1_DM2.at_PC >= 0 && DM1_DM2.is_9_stage){
                
				// sw after lw forwarding 
                if ( (DM1_DM2.MEM2)->MemWrite && DM2_WB.at_PC >= 0 && (DM2_WB.WB)->RegWrite && DM2_WB.RD != 0 && 
                        DM2_WB.RD == DM1_DM2.RT ){ 
						
                    	write_data = registers[DM2_WB.RD] ; 
						// cout << "forwarding from RW to DM2 " << "registers[DM2_WB.RD] " << registers[DM2_WB.RD] << "(DM1_DM2.MEM2)->MemWrite " << (DM1_DM2.MEM2)->MemWrite 
						// <<  "DM2_WB.at_PC " << DM2_WB.at_PC  << " " << "(DM2_WB.WB)->RegWrite " << (DM2_WB.WB)->RegWrite << " " << "DM2_WB.RD "<<DM2_WB.RD << "\n" ; 
                }
                else{
                    write_data = DM1_DM2.RegReadData2 ; 
                }



				if ((DM1_DM2.MEM2)->MemWrite) {
					if ( data[(DM1_DM2.ALURESULT)/4] != write_data){
						memoryDelta[(DM1_DM2.ALURESULT)/4] = write_data ;
					   }
					data[DM1_DM2.ALURESULT/4] = write_data ;
				} 
				
				(DM2_WB.WB)->MemtoReg = (DM1_DM2.WB)->MemtoReg;
				(DM2_WB.WB)->RegWrite = (DM1_DM2.WB)->RegWrite; 
				
                DM2_WB.ALURESULT = DM1_DM2.ALURESULT;
				DM2_WB.RD = DM1_DM2.RD;	
				DM2_WB.Read_data = DM1_DM2.Read_data ; 
			}

			DM2_WB.at_PC = DM1_DM2.at_PC;

			



            // DM1 stage (mem read stage) 
            if (EX_DM1.at_PC >= 0 && EX_DM1.is_9_stage){
				
				// forwarding logic ( 9 after 9 only,  possible here)
				if (RW_pc >= 0 && RW_regwrite && (EX_DM1.MEM2)->MemWrite && RW_rd != 0 && RW_rd == EX_DM1.RT){
					EX_DM1.RegReadData2 = registers[RW_rd] ; 
					// cout << "forwarding from RW stage to DM1 stage " << "\n" ; 
                    // cout << "RW_pc" << RW_pc << " " << "RW_regwrite" << RW_regwrite << " " << "RW_rd" << RW_rd << " " << "register[rd]" << registers[RW_rd] ; 
				}


                if ( (EX_DM1.MEM1)->MemRead) {
                        DM1_DM2.Read_data = data[EX_DM1.ALURESULT/4];
                        // cout << "read " <<DM1_DM2.Read_data<< " from address " << EX_DM1.ALURESULT/4 << "\n" ;
                    }

                (DM1_DM2.WB)->MemtoReg = (EX_DM1.WB)->MemtoReg;
				(DM1_DM2.WB)->RegWrite = (EX_DM1.WB)->RegWrite; 

                (DM1_DM2.MEM2)->MemWrite = (EX_DM1.MEM2)->MemWrite ; 

                DM1_DM2.ALURESULT = EX_DM1.ALURESULT;
				DM1_DM2.RD = EX_DM1.RD;	
				DM1_DM2.RT = EX_DM1.RT ; 
                DM1_DM2.RegReadData2 = EX_DM1.RegReadData2 ;    
                DM1_DM2.is_9_stage = EX_DM1.is_9_stage ; 
                (DM1_DM2.MEM1)->MemRead = (EX_DM1.MEM1)->MemRead ; 
                (DM1_DM2.MEM2)->MemWrite = (EX_DM1.MEM2)->MemWrite ; 
                
            }   
			DM1_rd = EX_DM1.RD ;// saving the value of ex_dm1 rd for stall check
			DM1_regwrite = (EX_DM1.WB)->RegWrite ; // saving the value of ex_dm1 regwrite for stall check
			DM1_pc = EX_DM1.at_PC ; // saving the value of ex_dm1 pc for stall check
            DM1_DM2.at_PC = EX_DM1.at_PC  ; 

			// EX Stage
			if (RR_EX.at_PC >= 0){
		
                dummy = RR_EX.is_9_stage ? &EX_DM1 : &DM2_WB ; 
                
                if (!RR_EX.is_9_stage){
					// cout << "jumping to WB in next cycle" << "\n" ; 
				}

				
                // forwarding logic 
                // first operand 
                if (RW_pc >= 0 && RW_regwrite && RW_rd != 0 && RW_rd == RR_EX.RS){
                    op1 =  registers[RW_rd] ; 
                    // cout << "forwarding from RW stage to EX stage " << "\n" ; 
                    // cout << "RW_pc" << RW_pc << " " << "RW_regwrite" << RW_regwrite << " " << "RW_rd" << RW_rd << " " << "register[rd]" << registers[RW_rd] ; 
                } 
                else {
                    op1 = RR_EX.RegReadData1 ; 
                }

                // second operand 
				// even if RT not defined for the instruction, implying it uses immediate field
				// and even if forwarding occurs, then it will get updated to correct value below coz of alusrc
                if (RW_pc >= 0 && RW_regwrite && RW_rd != 0 && RW_rd == RR_EX.RT){
                    op2 =  registers[RW_rd] ; 
					// sw uses regreaddata2 as write_data, so setting it also (case covered - add $1 .. ; sw $1 ..)
					RR_EX.RegReadData2 = op2 ; 
                    // cout << "forwarding from RW stage to EX stage " << "\n" ; 
                    // cout << "RW_pc" << RW_pc << " " << "RW_regwrite" << RW_regwrite << " " << "RW_rd" << RW_rd << " " << "register[rd]" << registers[RW_rd] ; 
                }
                else{
                    op2 = RR_EX.RegReadData2 ; 
                }
                
                if ( (RR_EX.EX)->ALUSrc ){
                    op2 = RR_EX.immediate ; 
                }




				if ((RR_EX.EX)->OP == 0){
					    dummy->ALURESULT = op1 + op2 ; 
				}
				else if ((RR_EX.EX)->OP == 1){	
						dummy->ALURESULT = op1 - op2 ;
				}

				else if ((RR_EX.EX)->OP == 2) {
						dummy->ALURESULT = op1*op2;
				}
				else if ((RR_EX.EX)->OP == 3){
					dummy->ALURESULT = ( op1 < op2 ) ? 1 : 0 ;  
				}
				 
				
				
				
				// handling beq instr
				if ((RR_EX.EX)->branch){
					is_beq_stall = true ; 
					// cout << "five stalls inserted for beq instruction" << "\n" ;
					dummy->zero = (dummy->ALURESULT == 0);
					
					PCnext = ( (RR_EX.EX)->branch && dummy->zero ) ? RR_EX.PC_new : ( RR_EX.at_PC + 1) ; 
					
					// cout << "branching to new PC " << PCnext << "\n" ;
				}

				// handling bne instr
				if ((RR_EX.EX)->branch_not){
					is_beq_stall = true ; 
					// cout << "five stalls inserted for beq instruction" << "\n" ;
					dummy->zero = (dummy->ALURESULT == 0);
					
					PCnext = ( (RR_EX.EX)->branch && (!dummy->zero) ) ? RR_EX.PC_new : ( RR_EX.at_PC + 1) ;

					
					// cout << "branching to new PC " << PCnext << "\n" ;
				}
				 
				
				(dummy->MEM1)->MemRead = (RR_EX.MEM1)->MemRead ;
				(dummy->MEM2)->MemWrite = (RR_EX.MEM2)->MemWrite ;
				(dummy->WB)->MemtoReg = (RR_EX.WB)->MemtoReg ;
				(dummy->WB)->RegWrite = (RR_EX.WB)->RegWrite ; 

				 
				
                dummy->RT = RR_EX.RT ; dummy->RS = RR_EX.RS ; 
				dummy->RD = RR_EX.RD; 

				dummy->RegReadData2 = RR_EX.RegReadData2;
                dummy->is_9_stage = RR_EX.is_9_stage ; 
                dummy->at_PC = RR_EX.at_PC ; 
				if (!RR_EX.is_9_stage) {
					EX_DM1.at_PC = -1 ; 
				}
            	dummy->PC_new = RR_EX.PC_new ; 
			} 
			
			else {
                EX_DM1.at_PC = RR_EX.at_PC;
			    EX_DM1.PC_new = RR_EX.PC_new;
            }


            // RR stage 
            if (ID2_RR.at_PC >= 0 ){
				string instruction_name = (ID2_RR.currcomand)[0] ; 
				int RegReadData1 , RegReadData2 ; 
                //forwarding and stalling logic for RR stage 

				if (instruction_name == "add" || instruction_name == "sub" || instruction_name == "mul" || instruction_name == "slt"){
					  rs = (ID2_RR.currcomand)[2]; check_rs = true ; 
					  rt = (ID2_RR.currcomand)[3] ; check_rt = true; 
				}
				else if (instruction_name == "addi"){
					rs = (ID2_RR.currcomand)[2]; check_rs= true ;check_rt = false ; 
				}

				else if (instruction_name == "lw" || instruction_name == "sw"){
					if ((ID2_RR.currcomand)[2].back() == ')'){
					lparen = (ID2_RR.currcomand)[2].find('(');
					rs = (ID2_RR.currcomand)[2].substr(lparen + 1); rs.pop_back();
					offset = stoi(lparen == 0 ? "0" : (ID2_RR.currcomand)[2].substr(0, lparen));
					check_rs = true  ; check_rt = false ; 
					}
					else{
						offset = stoi((ID2_RR.currcomand)[2]) ;
						check_rs = false ; check_rt = false ; 
					}
					
					rt = (ID2_RR.currcomand)[1] ; 
					
				}// not sw  first operand as it needs value for mem stage 

				else if (instruction_name == "beq" || instruction_name == "bne"){
					  rs = (ID2_RR.currcomand)[1] ; check_rs = true ; 
					  rt = (ID2_RR.currcomand)[2] ; check_rt = true ; 
				}

				


				// stalling logic (checking in ALU and DM1 stages)
				if (RR_EX.at_PC >= 0 && RR_EX.is_9_stage && (RR_EX.WB)->RegWrite && RR_EX.RD != 0 && 
					( ( check_rs && RR_EX.RD ==  registerMap[rs] ) || ( check_rt && RR_EX.RD == registerMap[rt]) ) ){
					is_stall = true ; if1_if2_write_enable = if2_id1_write_enable = id1_id2_write_enable = id2_rr_write_enable = false ;
					// cout << "type 1 stalling" << '\n' ; 
				}
				else if (DM1_pc >= 0 && DM1_regwrite && DM1_rd != 0 && 
					( ( check_rs && DM1_rd ==  registerMap[rs]) || ( check_rt && DM1_rd ==  registerMap[rt]) ) ){
					is_stall = true ; if1_if2_write_enable = if2_id1_write_enable = id1_id2_write_enable = id2_rr_write_enable = false ;
					// cout << "type 2 stalling" << '\n' ; 
				}
				else {
					// forwarding logic 
					// regreaddata1 
					if (RW_pc >= 0 && RW_regwrite && RW_rd != 0 && 
						check_rs && RW_rd == registerMap[rs] ) {
						RegReadData1 = registers[RW_rd]  ; 
						// cout << "forwarding from RW stage to RR stage " << "\n" ; 
						// cout << "rw_pc" << RW_pc << " " << "RW_rd " << RW_rd  << " " << "RW_regwrite" << RW_regwrite << " " << "RegReadData1" << RegReadData1 << "\n" ; 
					}
					else{
						RegReadData1 = registers[registerMap[rs]] ;  
					}

					// regreaddata2
					if (RW_pc >= 0 && RW_regwrite && RW_rd != 0 && 
						check_rt && RW_rd == registerMap[rt] ) {
						RegReadData2 = registers[RW_rd]  ; 
						// cout << "forwarding from RW stage to RR stage " << "\n" ; 
						// cout << "RW_pc" << RW_pc << " " << "RW_regwrite" << RW_regwrite << " " << "RW_rd" << RW_rd << " " << "RegReadData2" << RegReadData2 << "\n" ; 
					}
					else{
						RegReadData2 = registers[registerMap[rt]] ; 
					}


					if (instruction_name == "add" || instruction_name == "sub" || instruction_name == "mul" || instruction_name == "slt"){
						
						rd = (ID2_RR.currcomand)[1];
						
						
						RR_EX.RegReadData1 = RegReadData1 ; 
						RR_EX.RegReadData2 = RegReadData2 ; 
						

						RR_EX.RD = registerMap[rd] ; 
						RR_EX.RS = registerMap[rs] ; 
						RR_EX.RT = registerMap[rt] ; 
						RR_EX.PC_new = ID2_RR.at_PC + 1;
						
						}


					else if (instruction_name == "lw" || instruction_name == "sw"){ 
						
						if ((ID2_RR.currcomand)[2].back() == ')'){
							RR_EX.RegReadData1 = RegReadData1; 
							RR_EX.immediate = offset ; 
						}
						else {
							RR_EX.immediate = offset ; 
							RR_EX.RegReadData1 = 0 ; 
						}
						RR_EX.RegReadData2 = RegReadData2 ; 
						RR_EX.RS = registerMap[rs] ; 
						RR_EX.RT = registerMap[rt] ;  
						RR_EX.PC_new = ID2_RR.at_PC + 1;

						RR_EX.RD = RR_EX.RT ; 
						
					}

					else if (instruction_name == "addi") { 
						RR_EX.immediate = stoi(ID2_RR.currcomand[3]) ;
						rd = (ID2_RR.currcomand)[1];

						RR_EX.RS = registerMap[rs] ; 
						RR_EX.RegReadData1 = RegReadData1; 
						RR_EX.RD = registerMap[rd] ; 
						RR_EX.PC_new = ID2_RR.at_PC + 1 ; 
					}

					else if (instruction_name == "beq" || instruction_name == "bne"){
						RR_EX.RS = registerMap[rs] ; 
						RR_EX.RT = registerMap[rt] ; 
						RR_EX.RegReadData1 = RegReadData1 ; 
						RR_EX.RegReadData2 = RegReadData2 ; 
						RR_EX.PC_new = address[ID2_RR.currcomand[3]] ; 
					}
					// no jump case as its already handled in id2 (still propagates to RW stage)
					RR_EX.is_9_stage = ID2_RR.is_9_stage ; 

					(RR_EX.WB)->RegWrite = (ID2_RR.WB)->RegWrite;
					(RR_EX.WB)->MemtoReg = (ID2_RR.WB)->MemtoReg;

					(RR_EX.MEM1)->MemRead = (ID2_RR.MEM1)->MemRead ; 

					(RR_EX.MEM2)->MemWrite = (ID2_RR.MEM2)->MemWrite ;

					(RR_EX.EX)->ALUSrc = (ID2_RR.EX)->ALUSrc ;
					(RR_EX.EX)->branch = (ID2_RR.EX)->branch ;
					(RR_EX.EX)->branch_not = (ID2_RR.EX)->branch_not ;
					(RR_EX.EX)->OP = (ID2_RR.EX)->OP;
					(RR_EX.EX)->RegDst = (ID2_RR.EX)->RegDst;
				}
			}

                    
                
            RR_EX.at_PC = ID2_RR.at_PC ; 
			prev_is_9_stage = ID2_RR.is_9_stage ; 
			prev_PC = ID2_RR.at_PC ; 


			// ID2 phase  (decoding control signals)
			
			if(ID1_ID2.at_PC >= 0 && id2_rr_write_enable) {
				string instruction_name = (ID1_ID2.currcomand)[0];
				// cout << "decoding instruction- "   ; 
				// print_vector(ID1_ID2.currcomand) ; 
				
                    // setting the control signals
				
					(ID2_RR.EX)->RegDst = true ; (ID2_RR.EX)->OP = 0 ; (ID2_RR.EX)->ALUSrc = false ;  

					if (instruction_name == "add"){(ID2_RR.EX)->OP = 0 ; }
					if (instruction_name == "sub"){(ID2_RR.EX)->OP= 1 ; }
					if (instruction_name == "mul") { (ID2_RR.EX)->OP = 2 ; } 
					if (instruction_name == "slt") { (ID2_RR.EX)->OP = 3 ; } 

					(ID2_RR.EX)->branch = false; (ID2_RR.MEM1)->MemRead = false; (ID2_RR.MEM2)->MemWrite = false;
                    (ID2_RR.EX)->branch_not = false; 
					
					(ID2_RR.WB)->RegWrite = true ; (ID2_RR.WB)->MemtoReg = false ; 
					
					if (instruction_name == "lw"){ 
						
                        (ID2_RR.EX)->RegDst =0;
						(ID2_RR.EX)->ALUSrc =1;
						(ID2_RR.MEM1)->MemRead =1;
						(ID2_RR.WB)->MemtoReg =1;					
					}
					else if (instruction_name == "sw"){
                        
						(ID2_RR.EX)->ALUSrc =1;
						(ID2_RR.MEM2)->MemWrite=1;
						(ID2_RR.WB)->RegWrite=0;					
					}
					else if (instruction_name == "beq"){ // inserting a stall whenver encountering a beq instruction as per piazza instruction 
						(ID2_RR.EX)->branch=1;
						(ID2_RR.EX)->branch_not=0;
						(ID2_RR.EX)->OP =1;  
						(ID2_RR.WB)->RegWrite=0;					
					}
					else if (instruction_name == "bne"){ // inserting a stall whenver encountering a beq instruction as per piazza instruction 
						(ID2_RR.EX)->branch=0;
						(ID2_RR.EX)->branch_not=1;
						(ID2_RR.EX)->OP =1;  
						(ID2_RR.WB)->RegWrite=0;					
					}
					else if (instruction_name =="j"){
                        is_j_stall = true ;
                        PCnext = address[ ID1_ID2.currcomand[1] ] ; 
                        ID2_RR.PC_new = PCnext  ; 
                        (ID2_RR.WB)->RegWrite =0;
                        // cout << "jumping to new PC " << PCnext << "\n"  ; 
                        // cout << "inserting three stalls " << "\n" ; 
						
					}
					else if (instruction_name == "addi"){
						(ID2_RR.EX)->ALUSrc=1;	 
					}
                ID2_RR.currcomand = ID1_ID2.currcomand ;
				ID2_RR.is_9_stage = ID1_ID2.is_9_stage ; 
				ID2_RR.at_PC = ID1_ID2.at_PC ;
			}
			else if (!id2_rr_write_enable){
				// cout << "stalling" << "\n" ; 
			}
			else{
				ID2_RR.at_PC = ID1_ID2.at_PC ;
			}


            // ID1 STAGE (detect hazards in this stage )
            if (IF2_ID1.at_PC >= 0 && id1_id2_write_enable){
                // write stall detection logic 
                bool temp = false ;  string instruction_name = (IF2_ID1.currcomand)[0]  ; 
                temp = (instruction_name == "lw"  || instruction_name == "sw" ) ; 
                if (ID1_ID2.at_PC >= 0 && ID1_ID2.is_9_stage && !temp){ // 9 stage followed by 7 stage 
                    is_fifo_stall = true ; if1_if2_write_enable =  if2_id1_write_enable = false ; 
                    // cout << "stalling due to fifo type 1" << "\n" ; 
                }
				
                else if (prev_PC >= 0 && prev_is_9_stage && !temp){ // 9 stage followed by 7 stage 
                    is_fifo_stall = true ; if1_if2_write_enable =  if2_id1_write_enable = false ;
                    // cout << "stalling due to fifo type 2" << "\n" ; 
                }

                ID1_ID2.is_9_stage = temp ; 
                ID1_ID2.currcomand = IF2_ID1.currcomand ;
				ID1_ID2.at_PC = IF2_ID1.at_PC ; 
            }
			else if (!id1_id2_write_enable){
                // cout << "stalling " << "\n" ; 
            }
			else {
				ID1_ID2.at_PC = IF2_ID1.at_PC ; 
			}
            
			

			// IF2 stage 			
			if ( IF1_IF2.at_PC >= 0 && if2_id1_write_enable) {
				 
                IF2_ID1.at_PC = IF1_IF2.at_PC ;
                IF2_ID1.currcomand = commands[IF2_ID1.at_PC] ; // fetching the instruction
                // cout << "fetched instruction is " << "\n" ; 
                // print_vector(IF2_ID1.currcomand) ;
				
                } 
            else if ( !if2_id1_write_enable){
                // cout << "stalling" << "\n" ; 
            }
			else {
				// cout << "no fetched instruction" << "\n" ; 
				IF2_ID1.at_PC = IF1_IF2.at_PC ;
			}


            // IF1 stage 
			if ( (PCcurr >= 0 && PCcurr < commands.size()) && if1_if2_write_enable ){
				 
                IF1_IF2.at_PC = PCcurr ;
                // cout << "fetched pc is " << PCcurr << "\n" ; 
			} 
            else if (!if1_if2_write_enable){
                // cout << "stalling" << "\n" ; 
            }
			else {
				// cout << "no fetched instruction" << "\n" ; 
				IF1_IF2.at_PC = -1 ;
			}

            // fifo stall 
            if (is_fifo_stall && !is_j_stall && !is_beq_stall ){
				// cout << "doing fifo stalls " << "\n" ; 
                ID1_ID2.at_PC = -1 ; 
                PCnext = PCcurr ; 
            }
			
			
			// doing j stalls
			if (is_j_stall && !is_beq_stall && !is_stall){
				IF1_IF2.at_PC = -1 ; 
                IF2_ID1.at_PC = -1 ; 
                ID1_ID2.at_PC = -1 ;  
				// cout << "performing j stalls " << "\n" ; 
			}

			// doing data dependency stalls
			if (is_stall && !is_beq_stall){
				RR_EX.at_PC = -1 ; 
				PCnext = PCcurr ;  
				// cout << "rr stalling" << '\n' ; 
			}

			

			// doing beq stalls
			if (is_beq_stall){
				// cout << "doing beq stalls" << "\n" ; 
				IF1_IF2.at_PC = -1 ; 
                IF2_ID1.at_PC = -1 ; 
                ID1_ID2.at_PC = -1 ; 
                ID2_RR.at_PC = -1 ; 
                RR_EX.at_PC = -1; 
			}

			

			// updating program counter
			if (PCnext >= commands.size()) { PCnext = -1 ; }  
			PCcurr = PCnext ;  
			// setting counters back
			is_stall = false ; is_beq_stall = false ; is_j_stall = false ; check_rs = check_rt = false ; 
            is_fifo_stall = false ; if1_if2_write_enable = if2_id1_write_enable = id1_id2_write_enable = id2_rr_write_enable =  true ; 
			
			printRegistersAndMemoryDelta(clockCycles);
		}
		
		
	}

	// print the register data in decimal
	void printRegisters()
	{
		// std::cout << "Cycle number: " << clockCycle << '\n'
		// 		  << std::dec; 
		for (int i = 0; i < 32; ++i)
			std::cout << registers[i] << ' ';
		std::cout << std::dec << '\n';
	}

	// print the register data in hexadecimal
	void printRegistersAndMemoryDelta(int clockCycle)
	{
		for (int i = 0; i < 32; ++i)
			std::cout << registers[i] << ' ';
		std::cout << '\n';
		std::cout << memoryDelta.size() << ' ';
		if ( memoryDelta.size() == 0){
			cout << "\n" ; 
		}
		for (auto &p : memoryDelta)
			std::cout << p.first << ' ' << p.second << '\n';
		memoryDelta.clear();
	}
};

#endif