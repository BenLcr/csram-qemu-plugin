/* 
* This file contains the C-SRAM performance model, in term of speed and energy. 
* Instructions count are categorized in two sets:
* * RISC-V instructions : ALU_RV, CTRL_RV, LD_MEM_RV, ST_MEM_RV are RISC-V category instructions
* * C-SRAM instructions : ALU_CXRAM are C-SRAM computation instructions, MEM_CXRAM for CSRAM internal transferts
* This model is completely artificial and does not reflect the performance of any C-SRAM circuit
*/
typedef enum {
	ALU_RV, CTRL_RV, LD_MEM_RV, ST_MEM_RV, MEM_CXRAM, ALU_CXRAM,
} PerfType; 

int latency [] = {1, 10, 100, 1000, 10000, 100000};
int energy [] = {2, 20, 200, 2000, 20000, 200000};
