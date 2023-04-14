all: sample

sample: sample.cpp MIPS_Processor.hpp
	g++ sample.cpp MIPS_Processor.hpp -o sample

clean:
	rm sample
compile:
	g++ 5stage.cpp 5stage.hpp -o 5stage
	g++ 5stage_bypass.cpp 5stage_bypass.hpp -o 5stage_bypass
	g++ 79stage.cpp 79stage.hpp -o 79stage
	g++ 79stage_bypass.cpp 79stage_bypass.hpp -o 79stage_bypass
branch:
	g++ BranchPredictor.cpp BranchPredictor.hpp -o BranchPredictor
	./BranchPredictor
run_5stage:
	./5stage input.asm
run_5stage_bypass:
	./5stage_bypass input.asm
run_79stage:
	./79stage input.asm
run_79stage_bypass:
	./79stage_bypass input.asm