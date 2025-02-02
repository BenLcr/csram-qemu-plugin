/* Autogenerated C file */
/* Generation date / time : 2023-04-17 17:08:01.696641 */
#ifndef CXRAM_VM_H
#define CXRAM_VM_H
/* Autogenerated CXRAM configuration from ISA-2.0/csram-v2.0.5.json */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
/* Vector len in bytes */
#define CXRAM_VECLEN (16)
/* Memory base address */
#define CXRAM_BASE_ADDR (0x10000000)
/* Memory line size in byte */
#define CXRAM_BASE_SIZE (((2<<14)*16))
/* Type definition */

typedef  union {
  int8_t            i8[CXRAM_VECLEN];
  int16_t           i16[CXRAM_VECLEN/2];
  int32_t           i32[CXRAM_VECLEN/4];
  int64_t           i64[CXRAM_VECLEN/8];
} h2_cxram_line_t;
typedef h2_cxram_line_t (*h2_cxram_array_t);
#define CXRAM_IMC_ADDR  0x80000000 /* Dummy storage to avoid seg fault during sw */
#define CXRAM_IMC_SIZE  0x04000000


/* Autogenerated CXRAM opcodes from ISA-2.0/csram-opcodes-v2.0.5.csv */
#define CXRAM_ISA_RELEASE_MAJOR (2)
#define CXRAM_ISA_RELEASE_MINOR (0)
#define _hswap64(dest,src1)  ;
#define _hswap32(dest,src1)  ;
#define _redor(dest,src1)  ;
#define max_val(i)  ((uint64_t)(((uint64_t)1) << (uint8_t)(sizeof(i) * 8)) - 1)
enum CXRAMOPCODES {
  OP__cm_copy = 0xC0,
  OP__cm_hswap32 = 0x81,
  OP__cm_hswap64 = 0xC1,
  OP__cm_copyeq8 = 0x18,
  OP__cm_copyeq16 = 0x58,
  OP__cm_copyeq32 = 0x98,
  OP__cm_copygeq8 = 0x19,
  OP__cm_copygeq16 = 0x59,
  OP__cm_copygeq32 = 0x99,
  OP__cm_copygt8 = 0x1A,
  OP__cm_copygt16 = 0x5A,
  OP__cm_copygt32 = 0x9A,
  OP__cm_copyleq8 = 0x1B,
  OP__cm_copyleq16 = 0x5B,
  OP__cm_copyleq32 = 0x9B,
  OP__cm_copylt8 = 0x1C,
  OP__cm_copylt16 = 0x5C,
  OP__cm_copylt32 = 0x9C,
  OP__cm_copyneq8 = 0x1D,
  OP__cm_copyneq16 = 0x5D,
  OP__cm_copyneq32 = 0x9D,
  OP__cm_bcast8 = 0x10,
  OP__cm_bcast16 = 0x50,
  OP__cm_bcast32 = 0x90,
  OP__cm_slli8 = 0x20,
  OP__cm_slli16 = 0x60,
  OP__cm_slli32 = 0xA0,
  OP__cm_srli8 = 0x21,
  OP__cm_srli16 = 0x61,
  OP__cm_srli32 = 0xA1,
  OP__cm_not = 0xE2,
  OP__cm_redor = 0xE3,
  OP__cm_and = 0xE8,
  OP__cm_or = 0xE9,
  OP__cm_xor = 0xEA,
  OP__cm_nand = 0xEB,
  OP__cm_nor = 0xEC,
  OP__cm_xnor = 0xED,
  OP__cm_abs8 = 0x30,
  OP__cm_abs16 = 0x70,
  OP__cm_abs32 = 0xB0,
  OP__cm_adds8 = 0x48,
  OP__cm_adds16 = 0x88,
  OP__cm_adds32 = 0xC8,
  OP__cm_add8 = 0x38,
  OP__cm_add16 = 0x78,
  OP__cm_add32 = 0xB8,
  OP__cm_sub8 = 0x39,
  OP__cm_sub16 = 0x79,
  OP__cm_sub32 = 0xB9,
  OP__cm_subs8 = 0x49,
  OP__cm_subs16 = 0x89,
  OP__cm_subs32 = 0xC9,
  OP__cm_cmp8 = 0x3A,
  OP__cm_cmp16 = 0x7A,
  OP__cm_cmp32 = 0xBA,
  OP__cm_mul8 = 0x3B,
  OP__cm_mul16 = 0x7B,
  OP__cm_mul32 = 0xBB,
  OP__cm_fmac8 = 0x3C,
  OP__cm_fmac16 = 0x7C,
  OP__cm_fmac32 = 0xBC,
  OP__cm_mod8 = 100,
  OP__cm_mod16 = 101,
  OP__cm_mod32 = 102,
};
enum CMP_FLAG {CMP_FLAG_GT = 1, CMP_FLAG_LT = 2,  CMP_FLAG_EQ = 4, CMP_FLAG_GEQ = 5, CMP_FLAG_LEQ = 6};
enum CMP_SHAM {CMP_SHAM_GT = 1, CMP_SHAM_LT = 2,  CMP_SHAM_EQ = 4};
enum MUXTYPE { grev = 0b0000, shuffle = 0b0001, pshuffle = 0b0010, ror = 0b0100, srl = 0b0101, rol = 0b0110, sll = 0b0111};
uint32_t cxram_emulate(uint64_t cxram_insn, h2_cxram_array_t h2_cxram_tiles, h2_cxram_line_t *cxramReg, bool cxram_trace);
void execute_cxram_emulate(int opcode, h2_cxram_line_t *src1, h2_cxram_line_t *src2, h2_cxram_line_t *dest, h2_cxram_line_t *cxramReg, uint32_t imm32, uint16_t imm16, uint8_t imm8, bool cxram_trace);
uint32_t getWordSize(uint32_t cxram_opcode);
void printVector (char * vectorName, int vectorLen, int wordSize, h2_cxram_line_t *vectorValues);
void printVector8(char * vectorName, int vectorLen, h2_cxram_line_t *vectorValues);
void printVector16(char * vectorName, int vectorLen, h2_cxram_line_t *vectorValues);
void printVector32(char * vectorName, int vectorLen, h2_cxram_line_t *vectorValues); 
#endif
/* End of generated file cxram-vm.h */
