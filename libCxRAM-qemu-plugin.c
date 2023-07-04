#include <fcntl.h>
#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#include <qemu/qemu-plugin.h>

#include <InsnCategory-riscv.h>
#include <cxram-stats.h>
#include <cxram-vm.h>
#include <perf-model.h>

// Internal hard-coded sizeof() to access hidden data structures
// of QEMU for RISC-V CPU (check QEMU_ROOT/target/riscv/cpu.[c|h]
// for more info struct CPURISCVState & struct RISCVCPU
#define RISCV_SIZEOF_CPUSTATE 33512 /* Utiliser gdb pour qemu pour afficher sizeof(struct CPURISCVState) */
#define RISCV_SIZEOF_CPUNEGATIVEOFFSETSTATE 8

/* RISCV internal importation */
void *cpu_global;
void *qemu_get_cpu(int index);
extern uintptr_t guest_base;
extern bool have_guest_base;

// Internal data structures to store user parameters
bool qemu_cxram_verbose, qemu_cxram_trace, qemu_cxram_report;
h2_cxram_line_t cxramReg;
/* Data gathered during program run */
static int insnCount;

/* Should export this symbol to be recognized as a qemu plugin */
QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

static uint32_t riscv_get_gpr(void *cpu_, unsigned int regNo)
{ /* Vérifier que les target_ulong de qemu sont bien sur 32 bits pour valider regNo * 4 */
	uint8_t *cpu = (uint8_t *)cpu_;
	return *(uint32_t *)(cpu + RISCV_SIZEOF_CPUSTATE + RISCV_SIZEOF_CPUNEGATIVEOFFSETSTATE + regNo * (4));
}

void fail(char *msg)
{
	fprintf(stderr, "libCXRAM fail : %s\n", msg);
	exit(-1);
}

void trace(char *msg)
{
	if (qemu_cxram_verbose)
		fprintf(stderr, "(%d) %s\n", QEMU_PLUGIN_VERSION, msg);
}

void print_perf(FILE* file, char* name, int nb_ops, int indice, int *TOTAL){
	fprintf(file,"%s_NB_OPS;%ld\n",name , nb_ops);
	fprintf(file,"%s_NB_CYCLES;%ld\n",name , nb_ops * latency[indice]);
	fprintf(file,"%s_ENERGY;%ld\n",name , nb_ops * energy[indice]);
	TOTAL[0] += nb_ops;
	TOTAL[1] += nb_ops * latency[indice];
	TOTAL[2] += nb_ops * energy[indice];
}

void report_perf(char *path_csv){
	trace("print_perf");
	int i, RVCalcul, RVCtrl, ST, LD, cxramMem, cxramALU;
	int TOTAL [] = {0, 0, 0}; //{nb_opp, latency, energy}
	FILE* file = NULL;
	file = fopen(path_csv, "w");
	fprintf(file,"name;value\n");
	fprintf(file,"CPU_NB_INSNS;%d\n",insnCount);
	// RISCV count
	RVCalcul = 0; RVCtrl = 0;
	for (i = 0; i < ARRAY_SIZE(categoryNames); i++){
		if (!strcmp(categoryNames[i],"COUNT_ST")){
			ST = riscvStats[i];}
		else if (!strcmp(categoryNames[i],"COUNT_LD"))
			LD = riscvStats[i];
		else if (!strcmp(categoryNames[i], "COUNT_IOP") || !strcmp(categoryNames[i],"COUNT_BIT"))
			RVCalcul += riscvStats[i];
		else RVCtrl += riscvStats[i];
	}
	print_perf(file, "MEM_STORE", ST, ST_MEM_RV, TOTAL);
	print_perf(file, "MEM_LOAD", LD, LD_MEM_RV, TOTAL);
	print_perf(file, "RISCV_CTRL", RVCtrl, CTRL_RV, TOTAL);
	print_perf(file, "RISCV_CALCUL", RVCalcul, ALU_RV, TOTAL);
	cxramALU = 0;
	for (i = 0; i < ARRAY_SIZE(categoryNamesCxRAM); i++){
		if (!strcmp(categoryNamesCxRAM[i], "COUNT_MEM"))
			cxramMem = cxramCategoryStats[i];
		else cxramALU += cxramCategoryStats[i];
	}
	print_perf(file, "CXRAM_MEM", cxramMem, MEM_CXRAM, TOTAL);
	print_perf(file, "CXRAM_ALU", cxramALU, ALU_CXRAM, TOTAL);
	//TOTAL
	fprintf(file,"TOTAL_NB_OPS;%d\n",TOTAL[0]);
	fprintf(file,"TOTAL_NB_CYCLES;%d\n",TOTAL[1]);
	fprintf(file,"TOTAL_ENERGY;%d\n",TOTAL[2]);
	fclose(file);
}

/* add inline call-back to count RISCV insn by category */
void cntRiscvInsn(struct qemu_plugin_insn *insnPtr, uint32_t insnData, uint32_t insnSize)
{
  int category = -1;
  switch (insnSize)
    {
    case 0:
    	printf ("Instruction size = 0 (due to mode debug ?)\n");
      	break;
    case 2:
      	insnData = insnData & 0xFFFF;
      	for (int patternN = 0; patternN < ARRAY_SIZE(riscv16_insn_class); patternN++)
          {
        	if (riscv16_insn_class[patternN].pattern == (insnData & riscv16_insn_class[patternN].mask))
		  	{
				category = riscv16_insn_class[patternN].what;
				break;
            }
          }
		break;
	case 4:
		insnData = insnData & 0xFFFF;
		for (int patternN = 0; patternN < ARRAY_SIZE(riscv32_insn_class); patternN++)
          {
		  	if (riscv32_insn_class[patternN].pattern == (insnData & riscv32_insn_class[patternN].mask))
              {
                category = riscv32_insn_class[patternN].what;
                break;
              }
          }
		break;
	default:
		fail("Unknonw RISCV instruction len\n");
		break;
	}
	qemu_plugin_register_vcpu_insn_exec_inline(insnPtr, QEMU_PLUGIN_INLINE_ADD_U64, &riscvStats[category], 1);
	qemu_plugin_register_vcpu_insn_exec_inline(insnPtr, QEMU_PLUGIN_INLINE_ADD_U64, &insnCount, 1);
}

/* check if it is a store word 32 bits instruction **aka possible csram instruction** */
bool is_possibleCSRAMinsn(uint32_t insnData, uint32_t insnSize)
{
	if (insnSize == 4) // is a 32 bits insn
	{
		if (riscv32_insn_class[0].pattern == (insnData & riscv32_insn_class[0].mask)) // is a SW
			return true;
	}
	else // is a 16 bits insn
	{
		if (riscv16_insn_class[0].pattern == (insnData & riscv16_insn_class[0].mask)) // is a C.SW
			return true;
	}
	return false;
}

/* Call-back for a store instruction aka : possible CXRAM instruction */
static void cxram_emulation_cb(void *cpu, uint32_t insnCode, uint32_t insnSize)
{
	uint32_t rs1Value, rs2Value, cxram_opcode;
	int8_t rs1, rs2;
	uint64_t cxram_insn;
	h2_cxram_array_t h2_cxram_tiles;

	trace("cxram_emulation_cb");
	cpu_global = cpu;
	h2_cxram_tiles = (h2_cxram_array_t)(guest_base + CXRAM_BASE_ADDR);
	if (insnSize == 4)
	{
		/* 31    25  24 20 19 15 14 12 11     7 6     0 */
		/* imm[11:5] rs2   rs1   010   imm[4:0] 0100011 SW  */
		rs1 = (insnCode >> 15) & 0x1F;
		rs2 = (insnCode >> 20) & 0x1F;
	}
	else
	{ /* insnSize == 2 */
		/* 15 13 12     10 9 7  6       5 4 2  1 0       */
		/* 110   uimm[5:3] rs1' uimm[2|6] rs2' 00  C.SW  */
		rs1 = ((insnCode >> 7) & 0x7) + 8;
		rs2 = ((insnCode >> 2) & 0x7) + 8;
	}
	rs1Value = riscv_get_gpr(cpu, rs1);
	rs2Value = riscv_get_gpr(cpu, rs2);
	//	printf ("rs1Value >> 24 : 0x%04x\n", (rs1Value >> (32 - 6)));
	if ((rs1Value >> (32 - 6)) == 0x20) // Indicateur d'instruction CSRAM = 100000
	{
		trace("cxram_emulation_cb/insn emulation");
		cxram_insn = ((uint64_t)rs1Value << 32) + rs2Value;
		cxram_opcode = cxram_emulate(cxram_insn, h2_cxram_tiles, &cxramReg, qemu_cxram_trace);
		/* Count cxram insn */
		for (int i = 0; i < ARRAY_SIZE(cxramInsnClass); i++)
		{
			if (cxram_opcode == cxramInsnClass[i].opcode){
				cxramInsnStats[i]++;
				cxramCategoryStats[cxramInsnClass[i].what]++;
			}
		}
	}
}

/* Call-back put on the sw insn */
static void vcpu_mem(unsigned int cpu_index, qemu_plugin_meminfo_t meminfo,
					 uint64_t vaddr, void *udata)
{
	trace("vcpu_mem");
	void *cpu = qemu_get_cpu(cpu_index);
	uint32_t insnData = (uint32_t)((uintptr_t)udata & 0xFFFFFFFF);
	uint32_t insnSize = ((uintptr_t)udata >> 33);
	if (!is_possibleCSRAMinsn(insnData, insnSize))
		return;
	cxram_emulation_cb(cpu, insnData, insnSize);
}

/* Call-back triggered on translation block */
static void vcpu_tb_trans(qemu_plugin_id_t id, struct qemu_plugin_tb *tb)
{
	struct qemu_plugin_insn *insnPtr;
	uint32_t insnData, insnSize;

	trace("vcpu_tb_trans");
	size_t n = qemu_plugin_tb_n_insns(tb);
	for (size_t insnN = 0; insnN < n; insnN++)
	{
		insnPtr = qemu_plugin_tb_get_insn(tb, insnN);
		insnData = (uint32_t)(*(uint64_t *)(qemu_plugin_insn_data(insnPtr)) & 0xFFFFFFFF);
		insnSize = qemu_plugin_insn_size(insnPtr);
		if (is_possibleCSRAMinsn(insnData, insnSize))
		{
			uint64_t udata = ((uint64_t)insnSize << 33) | insnData;
			qemu_plugin_register_vcpu_mem_cb(insnPtr, vcpu_mem,
											QEMU_PLUGIN_CB_RW_REGS,
											QEMU_PLUGIN_MEM_RW,
											(void *)((uintptr_t)udata));
		}
		cntRiscvInsn(insnPtr, insnData, insnSize);
	}
}

// Call-back triggered on termination of the guest program
static void plugin_exit(qemu_plugin_id_t id, void *p)
{
	int i;
	printf("\nTotal RISC insn count %d\n", insnCount);
	printf("\nRISC insn category stats :\n");
	for (i = 0; i < ARRAY_SIZE(categoryNames); i++)
	{
		printf("%15s : %ld \n", categoryNames[i], riscvStats[i]);
	}
	printf("\nCXRAM insn count (!= 0) :\n");
	for (i = 0; i < ARRAY_SIZE(cxramInsnStats); i++)
	{
		if (0 != cxramInsnStats[i])
			printf("%15s : %ld \n", cxramInsnClass[i].name, cxramInsnStats[i]);
	}
	if (qemu_cxram_report)
		report_perf("perf_report.csv");
	trace("qemu_plugin_exit");
}

QEMU_PLUGIN_EXPORT int qemu_plugin_install(qemu_plugin_id_t id,
										   const qemu_info_t *info,
										   int argc, char **argv)
{
	int i;
	/* Get parameters from environment */
	qemu_cxram_trace = getenv("QEMU_CXRAM_TRACE") != NULL;
	qemu_cxram_verbose = getenv("QEMU_CXRAM_VERBOSE") != NULL;
	qemu_cxram_report = getenv("QEMU_CXRAM_REPORT") != NULL;   

    printf ("CxRAM plugin start. Instruction set V%d.%d\n", CXRAM_ISA_RELEASE_MAJOR, CXRAM_ISA_RELEASE_MINOR);
	trace("qemu_plugin_install");

	if (0 != strcmp("riscv32", info->target_name))
	{
		printf("RISCV decode only\n");
		exit(-1);
	}
	for (i = 0; i < ARRAY_SIZE(categoryNames); i++)
	{ /* Statistic initialization */
		riscvStats[i] = 0;
	}
	for (i = 0; i < ARRAY_SIZE(cxramInsnStats); i++)
	{
		cxramInsnStats[i] = 0;
	}
	for (i = 0; i < CXRAM_VECLEN; i++)
	{ /* CxRAM initialization */
		cxramReg.i8[i] = 0;
	}
	insnCount = 0;
	qemu_plugin_register_vcpu_tb_trans_cb(id, vcpu_tb_trans);
	qemu_plugin_register_atexit_cb(id, plugin_exit, NULL);
	return 0;
}
