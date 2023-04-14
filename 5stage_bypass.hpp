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
	
	struct MEMctr{
		
		bool MemRead;
		bool MemWrite;
		

		void print(struct MEMctr* ptr){
			if (ptr == NULL){return ; }
			
			cout << "MemRead" << ptr->MemRead  << "\n" ; 
			cout << "MemWrite" << ptr->MemWrite  << "\n" ; 
			
		}

	};
	struct EXctr{
		bool branch;
		bool branch_not;
		bool RegDst;
		int OP; // 0 denotes add , 1 denotes subtraction, 2 denotes multiplication, 3 denotes slt 
		bool ALUSrc ; // true if alu src has to be immediate field

		void print(struct EXctr* ptr){
			if (ptr == NULL){return ; }
			cout << "branch" << ptr->branch  << "\n" ; 
			cout << "branch not" << ptr->branch_not  << "\n" ; 
			cout << "RegDst" << ptr->RegDst  << "\n" ; 
			cout << "OP" << ptr->OP  << "\n" ; 
			cout << "ALUSrc" << ptr->ALUSrc  << "\n" ; 
		}

	};
	typedef struct WBctr WBctr;
	typedef struct MEMctr MEMctr;
	typedef struct EXctr EXctr;
	struct PipelineReg
{
	std::vector<string> currcomand;
	int at_PC = -1;
	int PC_new = -1;
/*control signals */
	WBctr* WB = NULL;
	MEMctr* MEM = NULL;
	EXctr* EX = NULL;
/*saved results */
	int ALURESULT;
	int Read_data;
	int immediate;
	bool zero = (ALURESULT == 0) ;
	int RegReadData1;
	int RegReadData2;

	int RS,RT,RD;

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
		cout << "printing control signals" << '\n' ;
		
		
		if (ptr->EX)  (ptr->EX)->print(ptr->EX) ;
		if (ptr->MEM) (ptr->MEM)->print(ptr->MEM) ; 
		if (ptr->WB) (ptr->WB)->print(ptr->WB) ; 
		
		  

	}
};
	typedef struct PipelineReg pr;
	


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
		int clockCycles=0;
		WBctr wbctr1,wbctr2, wbctr3, wbctr4 = {.RegWrite = false , .MemtoReg = false} ; 
		EXctr exctr1, exctr2, exctr3, exctr4 = {.branch = false, .branch_not = false, .RegDst = false , .OP = 0, .ALUSrc = false  } ; 
		MEMctr memctr1, memctr2, memctr3, memctr4 = {  .MemRead = false, .MemWrite = false } ; 
		vector<string> str = {"0"}  ; 
		pr IF_ID = {.currcomand = str, .at_PC = -1, .PC_new = -1, .WB = &wbctr1, .MEM = &memctr1, .EX = &exctr1,
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0} ; 

		pr ID_ALU = {.currcomand = str, .at_PC = -1, .PC_new = -1, .WB = &wbctr2, .MEM = &memctr2, .EX = &exctr2,
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0} ; 

		pr ALU_MEM = {.currcomand = str, .at_PC = -1, .PC_new = -1, .WB = &wbctr3, .MEM = &memctr3, .EX = &exctr3,
					 .ALURESULT = 0, .Read_data =  0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0} ; 

		pr MEM_WB = {.currcomand = str, .at_PC = -1, .PC_new = -1, .WB = &wbctr4, .MEM = &memctr4, .EX = &exctr4,
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0} ; 

		
		 bool is_beq_stall = false ; bool is_j_stall = false ; bool check = false;  bool is_stall = false ; // data dependency stall
         int prev_RD = 0 ; bool prev_RegWrite = false ; int prev_PC = -1 ; int prev_ALURESULT = 0 ;
        int lparen = 0 ; int offset = 0 ;    
		
        bool check_rs = false ; bool check_rt = false ; // setting rs and rt for checking

        int op1 = 0 , op2 = 0 ;  // op1, op2 chooses between og value and forwarded value 
		int write_data = 0 ; // wte data chooses between forwarded 
		string rs, rt, rd ; 
		printRegistersAndMemoryDelta(0) ; 
		while ( !(PCcurr == -1 && IF_ID.at_PC == -1 && ID_ALU.at_PC == -1 && 
						 		ALU_MEM.at_PC == -1 && MEM_WB.at_PC == -1 )) // if pc < 0 then a stall will occur
		{	
	
			
			++clockCycles;
			// cout << "clock cycle - " << clockCycles << "\n" ; 
			if (PCcurr == -1){PCnext = -1 ; }
			else PCnext = PCcurr + 1 ;  

			  
			// write back (during first half )	// rest stages are in second half cycle 
			if(MEM_WB.at_PC >= 0 && (MEM_WB.WB)->RegWrite && MEM_WB.RD != 0){
				 
				if ((MEM_WB.WB)->MemtoReg)  registers[MEM_WB.RD] = MEM_WB.Read_data;
				else registers[MEM_WB.RD] = MEM_WB.ALURESULT;
			};

            prev_RD = MEM_WB.RD ;// saving the value of mem_wb rd for forward check
			prev_RegWrite = (MEM_WB.WB)->RegWrite ; // saving the value of mem_wb regwrite for forward check
			prev_PC = MEM_WB.at_PC ; // saving the value of mem_wb pc for forward check
            prev_ALURESULT = MEM_WB.ALURESULT ; // saving the value of mem_wb aluresult for forwarding
			
			// mem stage 
			if (ALU_MEM.at_PC >= 0 ){

                // forwarding logic for mem stage
                if ( MEM_WB.at_PC >= 0 && (MEM_WB.WB)->RegWrite && (ALU_MEM.MEM)->MemWrite && MEM_WB.RD != 0 
                    && MEM_WB.RD == ALU_MEM.RT ){
                       
                        write_data = registers[MEM_WB.RD] ; 
                        // cout << "forwarding in mem stage " << "ALU_MEM.RT " << ALU_MEM.RT << " write_data " << write_data << "\n" ; 
                    }
                else {
                    write_data = ALU_MEM.RegReadData2 ; 
                }
				  

                
				if ((ALU_MEM.MEM)->MemWrite) {
					if ( data[(ALU_MEM.ALURESULT)/4] != write_data){
						memoryDelta[(ALU_MEM.ALURESULT)/4] = write_data ;
					   }
					data[(ALU_MEM.ALURESULT)/4] = write_data ; 
				} 
				if ( (ALU_MEM.MEM)->MemRead) {
					MEM_WB.Read_data = data[(ALU_MEM.ALURESULT)/4];
				}
				(MEM_WB.WB)->MemtoReg = (ALU_MEM.WB)->MemtoReg;
				(MEM_WB.WB)->RegWrite = (ALU_MEM.WB)->RegWrite; 
				
                MEM_WB.ALURESULT = ALU_MEM.ALURESULT;
				MEM_WB.RD = ALU_MEM.RD;	
                MEM_WB.RT= ALU_MEM.RT;
                MEM_WB.RS = ALU_MEM.RS;
			}

			MEM_WB.at_PC = ALU_MEM.at_PC;

			


			// EX Stage
			if (ID_ALU.at_PC >= 0){

                // EX stage forwarding logic 
                
                // first operand
                 if (prev_PC >= 0 && prev_RegWrite && prev_RD != 0 &&
                    ID_ALU.RS == prev_RD){
                    op1 = registers[prev_RD]  ; 
                    // cout << "forwarding from wb stage for op1" << "\n" ; 
                    // cout << "passed values " << "prev_PC " << prev_PC << " " << " prev_RD" << prev_RD << " " << "ID_ALU.RS " << ID_ALU.RS << "op1" << op1 << "\n" ; 
                    
                    }
               

                else if (ALU_MEM.at_PC >= 0 && (ALU_MEM.WB)->RegWrite && ALU_MEM.RD != 0 &&
                    ID_ALU.RS == ALU_MEM.RD){
                    op1 = ALU_MEM.ALURESULT ; 
                    // cout << "forwarding from mem stage for op1" << "\n" ; 
                    }
                else op1 = ID_ALU.RegReadData1 ; 
                
               

                // second operand 
                 if (prev_PC >= 0 && prev_RegWrite && prev_RD != 0 &&
                    ID_ALU.RT == prev_RD){
                    op2 = registers[prev_RD] ; 
					ID_ALU.RegReadData2 = op2 ; 
                    // cout << "forwarding from wb stage for op2" << "\n" ; 
                    }
                 

                else if (ALU_MEM.at_PC >= 0 && (ALU_MEM.WB)->RegWrite && ALU_MEM.RD != 0 &&
                    ID_ALU.RT == ALU_MEM.RD){ // even if RT not defined for the instruction, implying it uses immediate field
											 // and even if forwarding occurs, then it will get updated to correct value below coz of alusrc
                    op2 = ALU_MEM.ALURESULT ; 
					ID_ALU.RegReadData2 = op2 ; 
                    // cout << "forwarding from mem stage for op2" << "\n" ; 
                    // cout << "passed values " << "ALU_MEM.at_PC " << ALU_MEM.at_PC  << " " <<  "ALU_MEM.RD " << ALU_MEM.RD  << " " << "ID_ALU.RT " << ID_ALU.RT << "op2 " << op2 << "\n" ;                   
					 }
                else op2 = ID_ALU.RegReadData2 ;

                if ( (ID_ALU.EX)->ALUSrc ){
                    op2 = ID_ALU.immediate ; 
                } 


				
				if ((ID_ALU.EX)->OP == 0){
					    ALU_MEM.ALURESULT = op1 + op2 ; 
				}
				else if ((ID_ALU.EX)->OP == 1){	
					ALU_MEM.ALURESULT = op1 - op2 ;
				}

				else if ((ID_ALU.EX)->OP == 2) {
					ALU_MEM.ALURESULT = op1 * op2;  
				}

				else if ((ID_ALU.EX)->OP == 3){
					ALU_MEM.ALURESULT = ( op1 < op2 ) ? 1 : 0 ;  
				}
				 
				
				
				
				// handling beq instr
				if ((ID_ALU.EX)->branch){
					is_beq_stall = true ; 
					// cout << "two stalls inserted for beq instruction" << "\n" ;
					ALU_MEM.zero = (ALU_MEM.ALURESULT == 0);
					check = (ID_ALU.EX)->branch && ALU_MEM.zero ; 
					PCnext = ( check ? ID_ALU.PC_new : (ID_ALU.at_PC + 1) )  ; 
					
					// cout << "branching to new PC " << PCnext << "\n" ;
				}

				// handling beq instr
				if ((ID_ALU.EX)->branch_not){
					is_beq_stall = true ; 
					// cout << "two stalls inserted for bne instruction" << "\n" ;
					ALU_MEM.zero = (ALU_MEM.ALURESULT == 0);
					check = (ID_ALU.EX)->branch && (!ALU_MEM.zero) ; 
					PCnext = ( check ? ID_ALU.PC_new : (ID_ALU.at_PC + 1) )  ; 
					// cout << "branching to new PC " << PCnext << "\n" ;
				}
				 
				(ALU_MEM.EX)->branch = (ID_ALU.EX)->branch ;
				(ALU_MEM.EX)->branch_not = (ID_ALU.EX)->branch_not ;
				
				(ALU_MEM.MEM)->MemRead = (ID_ALU.MEM)->MemRead ;
				(ALU_MEM.MEM)->MemWrite = (ID_ALU.MEM)->MemWrite ;

				(ALU_MEM.WB)->MemtoReg = (ID_ALU.WB)->MemtoReg ;
				(ALU_MEM.WB)->RegWrite = (ID_ALU.WB)->RegWrite ; 

				 
				if((ID_ALU.EX)->RegDst) ALU_MEM.RD = ID_ALU.RD;
				else ALU_MEM.RD = ID_ALU.RT;
				
				ALU_MEM.RegReadData2 = ID_ALU.RegReadData2;

                ALU_MEM.RT = ID_ALU.RT ; 
                ALU_MEM.RS = ID_ALU.RS ; 

			} 
			
			ALU_MEM.at_PC = ID_ALU.at_PC;
			ALU_MEM.PC_new = ID_ALU.PC_new;



			// Decode phase  
			
			if(IF_ID.at_PC >= 0) {
				string instruction_name = (IF_ID.currcomand)[0];
				// cout << "decoding instruction- "   ; 
				// print_vector(IF_ID.currcomand) ; 
				
                // hazard detection logic for ID stage 

               if (instruction_name == "add" || instruction_name == "sub" || instruction_name == "mul" || instruction_name == "slt"){
					  rs = (IF_ID.currcomand)[2]; check_rs = true ; 
					  rt = (IF_ID.currcomand)[3] ; check_rt = true; 
				}
				else if (instruction_name == "addi"){
					
					  rs = (IF_ID.currcomand)[2]; check_rs= true ; check_rt = false ; 
				}

				else if (instruction_name == "lw" && (IF_ID.currcomand)[2].back() == ')'){
					int lparen = (IF_ID.currcomand)[2].find('(');
					  rs = (IF_ID.currcomand)[2].substr(lparen + 1); rs.pop_back();
					check_rs = true  ; check_rt = false ; 
				}

				else if (instruction_name == "sw" ){
					if ((IF_ID.currcomand)[2].back() == ')'){
					int lparen = (IF_ID.currcomand)[2].find('(');
					  rs = (IF_ID.currcomand)[2].substr(lparen + 1); rs.pop_back();
					//   cout << "decoded rs is " << rs << "\n" ; 
					check_rs = true ; 
					}
					else  check_rs = false ; 
					  rt = (IF_ID.currcomand)[1]; check_rt = false ;

				} 
                // not sw  first operand as it needs value for mem stage 
				else if (instruction_name == "beq" || instruction_name == "bne"){
					  rs = (IF_ID.currcomand)[1] ; check_rs = true ; 
					  rt = (IF_ID.currcomand)[2] ; check_rt = true ; 
				}


                if (ID_ALU.at_PC >= 0 && (ID_ALU.MEM)->MemRead && ( (check_rs && registerMap[rs] == ID_ALU.RT) || (check_rt && registerMap[rt] == ID_ALU.RT)) ){
                    is_stall = true ; 
                    // cout << "stalling " << '\n' ; 
                }

                else {
				
				

					// setting the control signals
				
					(ID_ALU.EX)->RegDst = true ; (ID_ALU.EX)->OP = 0 ; (ID_ALU.EX)->ALUSrc = false ;  

					if (instruction_name == "add"){(ID_ALU.EX)->OP = 0 ; }
					if (instruction_name == "sub"){(ID_ALU.EX)->OP= 1 ; }
					if (instruction_name == "mul") { (ID_ALU.EX)->OP = 2 ; } 
					if (instruction_name == "slt") { (ID_ALU.EX)->OP = 3 ;}

					(ID_ALU.EX)->branch = false; (ID_ALU.MEM)->MemRead = false; (ID_ALU.MEM)->MemWrite = false;
					(ID_ALU.EX)->branch_not = false;
					
					(ID_ALU.WB)->RegWrite = true ; (ID_ALU.WB)->MemtoReg = false ; 
					
					if (instruction_name == "lw"){
						(ID_ALU.EX)->RegDst =0;
						(ID_ALU.EX)->ALUSrc =true;
						(ID_ALU.MEM)->MemRead =1;
						(ID_ALU.WB)->MemtoReg =1;					
					}
					else if (instruction_name == "sw"){
						(ID_ALU.EX)->ALUSrc =true;
						(ID_ALU.MEM)->MemWrite=1;
						(ID_ALU.WB)->RegWrite=0;					
					}
					else if (instruction_name == "beq"){ // inserting a stall whenver encountering a beq instruction as per piazza instruction 
						(ID_ALU.EX)->branch=1;
						(ID_ALU.EX)->branch_not=0;
						(ID_ALU.EX)->OP =1;  
						(ID_ALU.WB)->RegWrite=0;					
					}
					else if (instruction_name == "bne"){ // inserting a stall whenver encountering a beq instruction as per piazza instruction 
						(ID_ALU.EX)->branch=0;
						(ID_ALU.EX)->branch_not=1;
						(ID_ALU.EX)->OP =1;  
						(ID_ALU.WB)->RegWrite=0;					
					}
					else if (instruction_name =="j"){
					
						(ID_ALU.WB)->RegWrite =0;
					}
					else if (instruction_name == "addi"){
						(ID_ALU.EX)->ALUSrc=true;	 
					}
				



					// reading registers
					if (instruction_name == "add" || instruction_name == "sub" || instruction_name == "mul" || instruction_name == "slt"){
					
					  rd = (IF_ID.currcomand)[1];
					  rs = (IF_ID.currcomand)[2];
					  rt = (IF_ID.currcomand)[3] ;
					
					ID_ALU.RegReadData1 = registers[registerMap[rs]] ; 
					ID_ALU.RegReadData2 = registers[registerMap[rt]] ; 
					
                    ID_ALU.RS = registerMap[rs] ; 
					ID_ALU.RD = registerMap[rd] ; 
					ID_ALU.RT = registerMap[rt] ; 
					ID_ALU.PC_new = IF_ID.at_PC + 1;
					
					}


					else if (instruction_name == "lw" || instruction_name == "sw"){ 

                        if ((IF_ID.currcomand)[2].back() == ')'){
                            lparen = (IF_ID.currcomand)[2].find('('), offset = stoi(lparen == 0 ? "0" : (IF_ID.currcomand)[2].substr(0, lparen));
                            rs = (IF_ID.currcomand)[2].substr(lparen + 1);
                            rs.pop_back();

                            ID_ALU.RegReadData1 = registers[registerMap[rs]] ; 
                            ID_ALU.immediate = offset ; 

                        }
                        else {
                            ID_ALU.immediate = stoi((IF_ID.currcomand)[2]) ; 
                            ID_ALU.RegReadData1 = 0 ; 
                        }
						

						rt = (IF_ID.currcomand)[1];
						ID_ALU.RegReadData2 = registers[registerMap[rt]] ; 
                        ID_ALU.RS = registerMap[rs] ; 
						ID_ALU.RT = registerMap[rt] ;  
						ID_ALU.PC_new = IF_ID.at_PC + 1;
					}

					else if (instruction_name == "addi") { 
						  rs = (IF_ID.currcomand)[2]; 
						ID_ALU.immediate = stoi(IF_ID.currcomand[3]) ;
						  rd = (IF_ID.currcomand)[1];

                        ID_ALU.RS = registerMap[rs] ;
						ID_ALU.RegReadData1 = registers[registerMap[rs]] ; 
						ID_ALU.RD = registerMap[rd] ; 
						ID_ALU.PC_new = IF_ID.at_PC + 1 ; 
					}

					else if (instruction_name == "j"){
						ID_ALU.PC_new = address[(IF_ID.currcomand)[1]] ;
						is_j_stall = true;
						PCnext = ID_ALU.PC_new ;
						// cout << "jumping to new PC " << ID_ALU.PC_new << "\n" ; 
					}

					else if (instruction_name == "beq" || instruction_name == "bne" ){
						  rs = (IF_ID.currcomand)[1]; 
						  rt = (IF_ID.currcomand)[2]; 
                        
                        ID_ALU.RS = registerMap[rs] ; 
						ID_ALU.RT = registerMap[rt] ;  

						ID_ALU.RegReadData1 = registers[registerMap[rs]] ; 
						ID_ALU.RegReadData2 = registers[registerMap[rt]] ; 
						ID_ALU.PC_new = address[IF_ID.currcomand[3]] ; 


					}
                }
			
			
			
			}
			ID_ALU.at_PC = IF_ID.at_PC ;
			


			// IF stage 			
			if ( (PCcurr >= 0 && PCcurr < commands.size()) || is_stall){
				if (!is_stall){
                    IF_ID.at_PC = PCcurr ;
                    IF_ID.currcomand = commands[PCcurr] ; // fetching the instruction
                    // cout << "fetched instruction is " << "\n" ; 
                    // print_vector(IF_ID.currcomand) ;
                } 
                else {
                    // cout << "stalled cycle stage " << "\n" ; 
                }
			} 
			else {
				// cout << "no fetched instruction" << "\n" ; 
				IF_ID.at_PC = -1 ;
			}
			


            // doing stall for avoiding data hazards 

            if (is_stall && !is_beq_stall){
                ID_ALU.at_PC = -1 ; 
                PCnext = PCcurr ; 
            }
			
			// doing j stalls
			if (is_j_stall && !is_beq_stall){
				IF_ID.at_PC = -1 ; 
			}

			// doing beq stalls
			if (is_beq_stall){
				IF_ID.at_PC = -1 ; 
				ID_ALU.at_PC = -1 ; 
				// ALU_MEM.at_PC = -1 ; 
			}
			// updating program counter
			if (PCnext >= commands.size()) { PCnext = -1 ; }  
			PCcurr = PCnext ;  
			// resetting control signals
			 is_beq_stall = false ; is_j_stall = false ; check = false ;  check_rs = check_rt = false ; 
            is_stall = false ; 
			printRegistersAndMemoryDelta(clockCycles) ; 
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