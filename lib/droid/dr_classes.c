#include "dr_int.h"

static const t_dbuiltin	g_dr_classes[] = {
{DR_EXC, DR_THR, NULL, 1, 0},
{DR_RTE, DR_EXC, NULL, 1, 0},
{"Ljava/lang/ArithmeticException;", DR_RTE, NULL, 1, 0},
{DR_NPE, DR_RTE, NULL, 1, 0},
{DR_IOOBE, DR_RTE, NULL, 1, 0},
{DR_AIOOBE, DR_IOOBE, NULL, 1, 0},
{DR_SIOOBE, DR_IOOBE, NULL, 1, 0},
{"Ljava/lang/NegativeArraySizeException;", DR_RTE, NULL, 1, 0},
{DR_CCE, DR_RTE, NULL, 1, 0},
{DR_IAE, DR_RTE, NULL, 1, 0},
{DR_NFE, DR_IAE, NULL, 1, 0},
{DR_ISE, DR_RTE, NULL, 1, 0},
{DR_ASE, DR_RTE, NULL, 1, 0},
{DR_ERR, DR_THR, NULL, 1, 0},
{"Ljava/lang/StackOverflowError;", DR_ERR, NULL, 1, 0},
{DR_OOME, DR_ERR, NULL, 1, 0},
{DR_NSME, DR_ERR, NULL, 1, 0},
{"Ljava/lang/NoClassDefFoundError;", DR_ERR, NULL, 1, 0},
{"Ljava/lang/VerifyError;", DR_ERR, NULL, 1, 0},
{"Ljava/lang/IncompatibleClassChangeError;", DR_ERR, NULL, 1, 0},
{"Ljava/lang/AbstractMethodError;", DR_ERR, NULL, 1, 0},
{DR_SB, DR_OBJ, DR_CSQ, 0x11, sizeof(t_drsb)},
{DR_INT, DR_OBJ, NULL, 0x11, 0},
{DR_MATH, DR_OBJ, NULL, 0x11, 0},
{DR_SYS, DR_OBJ, NULL, 0x11, 0},
{DR_CTX, DR_OBJ, NULL, 1, 0},
{DR_BUNDLE, DR_OBJ, NULL, 0x11, 0},
{DR_ACT, DR_CTX, NULL, 1, 0},
{DR_LOG, DR_OBJ, NULL, 0x11, 0},
{DR_OCL, NULL, NULL, 0x601, 0},
{DR_VIEW, DR_OBJ, NULL, 1, 8},
{DR_VG, DR_VIEW, NULL, 1, 0},
{DR_TV, DR_VIEW, NULL, 1, 0},
{DR_BTN, DR_TV, NULL, 1, 0},
{DR_LL, DR_VG, NULL, 1, 0}
};

uint32_t	dr_classes(const t_dbuiltin **tab)
{
	*tab = g_dr_classes;
	return (sizeof(g_dr_classes) / sizeof(g_dr_classes[0]));
}
