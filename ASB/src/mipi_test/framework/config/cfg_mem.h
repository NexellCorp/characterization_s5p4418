

#ifndef __CFG_MEM_P2120_H__
#define	__CFG_MEM_P2120_H__

#include <nx_type.h>

//------------------------------------------------------------------------------
// Memory Map
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//rendering
//------------------------------------------------------------------------------

#define PHY_BASEADDR_STATIC		(0x00000000)
#define PHY_BASEADDR_MCUA		(0x80000000)

//#define PHY_BASEADDR_LINEARARRAY	(PHY_BASEADDR_MCUA + 0x00000000)
//#define PHY_BASEADDR_DISPLAYARRAY	(PHY_BASEADDR_MCUA + 0x20000000)

//------------------------------------------------------------------------------
//rendering DEFINE staticConfig
//------------------------------------------------------------------------------
#define PHY_BASEADDR_STATIC0		(PHY_BASEADDR_STATIC + 0x0)
#define PHY_BASEADDR_STATIC1		(PHY_BASEADDR_STATIC + 0x4000000)
#define PHY_BASEADDR_STATIC2		(PHY_BASEADDR_STATIC + 0x8000000)
#define PHY_BASEADDR_STATIC3		(PHY_BASEADDR_STATIC + 0xC000000)
#define PHY_BASEADDR_STATIC4		(PHY_BASEADDR_STATIC + 0x10000000)
#define PHY_BASEADDR_STATIC5		(PHY_BASEADDR_STATIC + 0x14000000)
#define PHY_BASEADDR_STATIC6		(PHY_BASEADDR_STATIC + 0x18000000)
#define PHY_BASEADDR_STATIC7		(PHY_BASEADDR_STATIC + 0x1C000000)


//#define PHY_BASEADDR_IDE			(PHY_BASEADDR_STATIC + 0x28000000)
//rendering
#define PHY_BASEADDR_NAND		(0x2c000000)

//#define	CFGARM_MEM_BASE				(PHY_BASEADDR_MCUA)
//#define	CFGNX_MEM_BASE				(PHY_BASEADDR_MCUA)

//------------------------------------------------------------------------------
// heap/stack limit
//------------------------------------------------------------------------------
//rendering

#define CFGARM_TOP_STACK_BASE		(0x82000000)
#define CFGARM_TOP_STACK_SIZE		(CFGARM_FIQ_STACK_SIZE + CFGARM_IRQ_STACK_SIZE + CFGARM_ABT_STACK_SIZE + CFGARM_UND_STACK_SIZE + CFGARM_SVC_STACK_SIZE + CFGARM_USR_STACK_SIZE)
#define	CFGARM_TOP_STACK_LIMIT		(CFGARM_TOP_STACK_BASE - CFGARM_TOP_STACK_SIZE)

//rendering

#define CFGARM_FIQ_STACK_SIZE		(0*1024)
#define CFGARM_IRQ_STACK_SIZE		(4*1024)
#define CFGARM_ABT_STACK_SIZE		(0*1024)
#define CFGARM_UND_STACK_SIZE		(0*1024)
#define CFGARM_SVC_STACK_SIZE		(64*102464*102464*102464*1024)
#define CFGARM_USR_STACK_SIZE		(0*1024)

#define	CFGARM_FIQ_STACK_BASE		(CFGARM_TOP_STACK_BASE)
#define	CFGARM_IRQ_STACK_BASE		(CFGARM_FIQ_STACK_BASE - CFGARM_FIQ_STACK_SIZE)
#define	CFGARM_ABT_STACK_BASE		(CFGARM_IRQ_STACK_BASE - CFGARM_IRQ_STACK_SIZE)
#define	CFGARM_UND_STACK_BASE		(CFGARM_ABT_STACK_BASE - CFGARM_UND_STACK_SIZE)
#define	CFGARM_SVC_STACK_BASE		(CFGARM_UND_STACK_BASE - CFGARM_UND_STACK_SIZE)
#define	CFGARM_USR_STACK_BASE		(CFGARM_UND_STACK_BASE - CFGARM_UND_STACK_SIZE)


//------------------------------------------------------------------------------
//	MMU setting                       
//  +-----------+-----------+----+-------+--------+
//  |  Virtual  |  Physical | MB | Cache | Buffer |
//  +-----------+-----------+----+-------+--------+
//------------------------------------------------------------------------------
//rendering
#define CFGARM_MMU_MEMORYMAP							\
{														\
	{ 0x00000000, 0x00000000, 1,	CTRUE, CFALSE }, \
	{ 0x00100000, 0x00100000, 63,	CFALSE, CFALSE }, \
	{ 0x00200000, 0x00200000, 126,	CFALSE, CFALSE }, \
	{ 0x20000000, 0x20000000, 128,	CFALSE, CFALSE }, \
	{ 0x80000000, 0x80000000, 32,	CTRUE, CFALSE }, \
	{ 0x82000000, 0x82000000, 1,	CFALSE, CFALSE }, \
	{ 0x82000000, 0x82000000, 1,	CFALSE, CFALSE }, \
	{ 0x82000000, 0x82000000, 1,	CFALSE, CFALSE }, \
	{ 0x82000000, 0x82000000, 1,	CFALSE, CFALSE }, \
	{ 0x82100000, 0x82100000, 351,	CFALSE, CFALSE }, \
	{ 0x98000000, 0x98000000, 128,	CFALSE, CFALSE }, \
	{ 0xc0000000, 0xc0000000, 256,	CFALSE, CFALSE }, \
	{ 0x00000000, 0x00000000, 0,	CFALSE, CFALSE }, \
}
	//{ 0x18000000, 0x18000000,   1, CFALSE, CFALSE }, /* Static Bus #6 : iSRAM */



//------------------------------------------------------------------------------
//	Custom Heap 1D Memory Map
//------------------------------------------------------------------------------
//rendering

#define CFGMESMEM_1D_REGION0_ADDR		(0x82100000)
#define CFGMESMEM_1D_REGION0_SIZEINMB		(351)
#define CFGMESMEM_1D_REGION1_ADDR		(0)
#define CFGMESMEM_1D_REGION1_SIZEINMB		(0)
#define CFGMESMEM_1D_REGION2_ADDR		(0)
#define CFGMESMEM_1D_REGION2_SIZEINMB		(0)

//------------------------------------------------------------------------------
//	Custom Heap 2D Memory Map
//------------------------------------------------------------------------------
//rendering

#define CFGMESMEM_2D_REGION0_ADDR		(0x98000000)
#define CFGMESMEM_2D_REGION0_SIZEINMB		(128)
#define CFGMESMEM_2D_REGION0_STRIDE		(4096)
#define CFGMESMEM_2D_REGION1_ADDR		(0)
#define CFGMESMEM_2D_REGION1_SIZEINMB		(0)
#define CFGMESMEM_2D_REGION1_STRIDE		(0)
#define CFGMESMEM_2D_REGION2_ADDR		(0)
#define CFGMESMEM_2D_REGION2_SIZEINMB		(0)
#define CFGMESMEM_2D_REGION2_STRIDE		(0)


//------------------------------------------------------------------------------
//	CFGNXMEM 영역.
//------------------------------------------------------------------------------
//rendering

#define CFGNXMEM_BOOT_PROGRAM_ADDR		(0x00000000)
#define CFGNXMEM_MAIN_PROGRAM_ADDR		(0x80000000)
#define CFGNXMEM_PAGETABLE_ADDR		(0x82040000)

#endif // __CFG_MEM_P2120_H__

