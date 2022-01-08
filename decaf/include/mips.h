/**
 * @file
 * @author Léo Chéneau <leo.cheneau@etu.unistra.fr
 * @version 1.0
 * @brief Génération du code MIPS
 *
 * @defgroup MIPS MIPS
 * @brief Langaage MIPS
 * @ingroup MIPS
 */
#ifndef INCLUDE__GENMIPS__H
#define INCLUDE__GENMIPS__H

#include <stddef.h> // size_t
#include <stdio.h> // FILE (obligatoire, pas possible de FD)

#include "context.h"
#include "quadops.h"

/**
 * @addtogroup ASM
 * @{
 * @addtogroup MIPS
 * @{
 */

#define MIPS_WORD ((int)bt_sizeof(BT_INT))
#define MIPS_RESERVED (2*MIPS_WORD)

enum Mips_adr { REG, IMM, IMR, SYM, NOT};

enum Mips_reg { ZERO, V0, A0, A1, A2, SP, FP, RA, T0, T1, T2, T3, T4, T5, T6, T7, INVALID_REGISTER};

extern const char* Mips_reg_str[INVALID_REGISTER];

#define MIPS_REG_TMP_NB (T7 - T0 + 1)

extern enum Mips_reg Mips_reg_temp[MIPS_REG_TMP_NB];
extern const struct entry* Mips_reg_storing[MIPS_REG_TMP_NB];
extern int Mips_tmp_last_used[MIPS_REG_TMP_NB];

enum Mips_op { ADD, ADDI, DIV, MUL, NEGU, REM, SUB, XOR,
	LI,
	LW, SW, MOVE,
	JAL, JR,
	SYSCALL,
	B, BEQ, BLE, BLT,
	INVALID };

extern int Mips_op_nums[INVALID];
extern enum Mips_adr Mips_op_fmt[INVALID][3];
extern const char* Mips_op_str[INVALID];

struct Mips_loc {
	enum Mips_adr adr;
	int imm;
	enum Mips_reg reg;
	const char* sym;
	quad_id_t qid;
};

#define MIPS_BUILTINS_NB 5
enum Mips_builtins { PRINT_INT = 1, PRINT_STRING = 4, READ_INT = 5, READ_STRING = 8 , EXIT = 10};
extern const char* Mips_builtins_decaf[MIPS_BUILTINS_NB];
extern enum Mips_builtins Mips_builtins_syscall[MIPS_BUILTINS_NB];

void call_builtin(const char* builtin);



struct asm_params;
extern FILE* out;
extern const struct asm_params* global_params;

void set_output(FILE* fd);
void set_parameters(const struct asm_params* params);

struct Mips_loc reg(enum Mips_reg reg);
struct Mips_loc imr(int imm, enum Mips_reg reg);
struct Mips_loc imm(int imm);
struct Mips_loc sym(const char* sym);

void print_loc(struct Mips_loc loc);

void instr(enum Mips_op op, ...);
void instrc(enum Mips_op op, ...);

#define LBL_QUAD_FMT "q__%lu"

void mips_label(const char* lbl, const char* com);
void mips_label_quad(quad_id_t qid);

void alloc_stack(int size);
void push_stack(struct Mips_loc loc, int size);
void pop_stack_args(const struct entry* fonction);

size_t allocated_for_func(const struct context* body_ctx);
void function_header(const struct context* body_ctx);
void function_footer(const struct context* body_ctx);

struct Mips_loc entry_loc(const struct entry* ent);
struct Mips_loc quad_loc(quad_id_t qid);

struct Mips_loc entry_to_reg(const struct entry* ent);
struct Mips_loc available_register(const struct entry* ent);
struct Mips_loc alloc_tmp_register(const struct entry* ent, enum Mips_reg t);
int entry_in_tmp(const struct entry* ent);
void clear_reg_tmp();
void save_reg_tmp();

void mips_translate(quad_id_t qid);

#define MIPS_TYPES_NB 1
extern const char* MIPS_types[MIPS_TYPES_NB];
extern size_t MIPS_types_size[MIPS_TYPES_NB];

#define MAX_TYPE_SIZE 64
void MIPS_type(const struct typedesc* td, char buf[MAX_TYPE_SIZE]);

void make_fct(const struct context* args_ctx);
void MIPS_text_segment(const quad_id_t* qlist, size_t liste_size);
void MIPS_data_segment();
void MIPS_start();
void genMIPS(const quad_id_t* qlist, size_t liste_size, FILE* outfile, const struct asm_params* genp);

/**
 * @defgroup Quadtrad Quadtrad
 * @brief Traduction de quadruplets en MIPS
 *
 * @addtogroup Quadtrad
 * @{
 */

void mips_Q_ADD(const struct entry* res, const struct entry* lhs, const struct entry* rhs);
void mips_Q_SUB(const struct entry* res, const struct entry* lhs, const struct entry* rhs);
void mips_Q_MUL(const struct entry* res, const struct entry* lhs, const struct entry* rhs);
void mips_Q_DIV(const struct entry* res, const struct entry* lhs, const struct entry* rhs);
void mips_Q_MOD(const struct entry* res, const struct entry* lhs, const struct entry* rhs);
void mips_Q_AFF(const struct entry* res, const struct entry* val);
void mips_Q_NEG(const struct entry* res, const struct entry* val);
void mips_Q_CST(const struct entry* res, int cst);
void mips_Q_END(const struct context* ctx);
void mips_Q_RET(const struct context* ctx, const struct entry* result);
void mips_Q_PAR(const struct entry* arg);
void mips_Q_CAL(const struct entry* res, const struct entry* fct);
void mips_Q_PRO(const struct entry* pro);
void mips_Q_GOT(quad_id_t dst);
void mips_Q_IFG(const struct entry* lhs, enum CMP_OP cop, const struct entry* rhs, quad_id_t dst);

/**
 * @}
 * @}
 * @}
 */
#endif


