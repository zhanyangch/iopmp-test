
#ifndef __RISCV_IOPMP_H__
#define __RISCV_IOPMP_H__

#include <inttypes.h>
#include "platform.h"

#define ENTRY_X_ON                              1
#define ENTRY_X_OFF                             0
#define ENTRY_W_ON                              1
#define ENTRY_W_OFF                             0
#define ENTRY_R_ON                              1
#define ENTRY_R_OFF                             0

#define ENTRY_CFG_XWR(x, w, r)                  (((x) << 2) | ((w) << 1) | ((r) << 0))
#define ENTRY_CFG_SIE_XWR(x, w, r)              (((x) << 7) | ((w) << 6) | ((r) << 5))
#define ENTRY_CFG_SEE_XWR(x, w, r)              (((x) << 10) | ((w) << 9) | ((r) << 8))
/* Error reaction commands */
#define ERR_CFG_CTRL_IE_ENABLE                     (1 << 1)
#define ERR_CFG_CTRL_IE_DISABLE                    (0 << 1)
#define ERR_CFG_CTRL_RS_ENABLE                     (1 << 2)
#define ERR_CFG_CTRL_RS_DISABLE                    (0 << 2)
#define ERR_CFG_CTRL_MSI_EN_ENABLE                 (1 << 3)
#define ERR_CFG_CTRL_MSI_EN_DISABLE                (0 << 3)
#define ERR_CFG_CTRL_STALL_VIOLATION_EN_ENABLE     (1 << 4)
#define ERR_CFG_CTRL_STALL_VIOLATION_EN_DISABLE    (0 << 4)

/* Transaction type of error */
#define TTYPE_READ                  1
#define TTYPE_WRITE                 2
#define TTYPE_FETCH                 3

#define ETYPE_READ                  1
#define ETYPE_WRITE                 2
#define ETYPE_FETCH                 3
#define ETYPE_PARTIAL_HIT           4
#define ETYPE_NO_HIT                5
#define ETYPE_RRID                  6
#define ETYPE_STALL                 8

#define RRIDSCP_OP_QUERY 0
#define RRIDSCP_OP_STALL 1
#define RRIDSCP_OP_NO_STALL 2

typedef struct {
    union
    {
        struct
        {
            volatile unsigned int EN;           /* 0x00 */
            volatile unsigned int ENH;          /* 0x04 */
        };
        struct
        {
            volatile unsigned int PERM;         /* 0x00 */
            volatile unsigned int PERMH;        /* 0x04 */
        };
    };
    volatile unsigned int R;                    /* 0x08 */
    volatile unsigned int RH;                   /* 0x0C */
    volatile unsigned int W;                    /* 0x10 */
    volatile unsigned int WH;                   /* 0x14 */
             unsigned int RESERVED0[2];         /* 0x18 ~ 0x20*/
} SRCMD_RegDef;

typedef struct {
    volatile unsigned int ADDR;                  /* 0x00 */
    volatile unsigned int ADDRH;                 /* 0x04 */
    volatile unsigned int CFG;                   /* 0x08 */
    volatile unsigned int USER_CFG;              /* 0x0C */
} ENTRY_RegDef;

typedef struct {
    volatile unsigned int VERSION;                      /* 0x00 */
    volatile unsigned int IMPLEMENTATION;               /* 0x04 */
    volatile unsigned int HWCFG0;                       /* 0x08 */
    volatile unsigned int HWCFG1;                       /* 0x0C */
    volatile unsigned int HWCFG2;                       /* 0x10 */
    volatile unsigned int OFFSET;                       /* 0x14 */
    volatile unsigned int RESERVED0[6];                 /* 0x18 ~ 0x2C */
    volatile unsigned int MDSTALL;                      /* 0x30 */
    volatile unsigned int MDSTALLH;                     /* 0x34 */
    volatile unsigned int RRIDSCP;                      /* 0x38 */
         unsigned int RESERVED3[1];                 /* 0x3C */
    volatile unsigned int MDLCK;                        /* 0x40 */
    volatile unsigned int MDLCKH;                       /* 0x44 */
    volatile unsigned int MDCFGLCK;                     /* 0x48 */
    volatile unsigned int ENTRYLCK;                     /* 0x4C */
         unsigned int RESERVED2[4];                 /* 0x50~0x5C */
    volatile unsigned int ERR_CFG;                      /* 0x60 */
    volatile unsigned int ERR_INFO;                  /* 0x64 */
    volatile unsigned int ERR_REQADDR;                  /* 0x68 */
    volatile unsigned int ERR_REQADDRH;                 /* 0x6C */
    volatile unsigned int ERR_REQID;                    /* 0x70 */
    volatile unsigned int ERR_MFR;                      /* 0x74 */
             unsigned int RESERVED5[482];               /* 0x78 ~ 0x7FC */
    volatile unsigned int MDCFG[63];                    /* 0x800 ~ 0x8F4 */
             unsigned int RESERVED6[449];               /* 0x8F8 ~ 0xFFC */
             SRCMD_RegDef SRCMD[256];                   /* 0x1000 ~ 0x2FF4 */
} IOPMP_RegDef;

/* General Functions */
int iopmp_get_imp(IOPMP_RegDef *iopmp_reg);
void iopmp_srcmd_lock(IOPMP_RegDef *iopmp_reg, int rrid);
void iopmp_srcmd_add_md(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx);
void iopmp_srcmd_remove_md(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx);
void iopmp_write_entry(IOPMP_RegDef *iopmp_reg,int entry, unsigned int mode,  void* va, unsigned long size, int cfg);
void iopmp_napot_config(IOPMP_RegDef *iopmp_reg, unsigned int entry, void* va, unsigned long size, int pmpcfg);
void iopmp_na4_config(IOPMP_RegDef *iopmp_reg, unsigned int entry, void* va, int pmpcfg);
void iopmp_tor_config(IOPMP_RegDef *iopmp_reg, unsigned int entry, void* va, int pmpcfg);
void iopmp_off_config(IOPMP_RegDef *iopmp_reg, unsigned int entry, void* va);
int iopmp_get_mdcfg(IOPMP_RegDef *iopmp_reg, int md);

typedef struct _RISCV_IOPMP_OPS {
        int (*get_hwcfg)(IOPMP_RegDef *iopmp_reg, int cfg_num);
        int (*get_srcmd_fmt)(IOPMP_RegDef *iopmp_reg);
        int (*get_mdcfg_fmt)(IOPMP_RegDef *iopmp_reg);
        int (*get_md_entry_num)(IOPMP_RegDef *iopmp_reg);
        void (*set_md_entry_num)(IOPMP_RegDef *iopmp_reg, int value);
        int (*get_md_num)(IOPMP_RegDef *iopmp_reg);
        int (*get_rrid_num)(IOPMP_RegDef *iopmp_reg);
        int (*get_entry_num)(IOPMP_RegDef *iopmp_reg);
        int (*get_prior_entry_num)(IOPMP_RegDef *iopmp_reg);
        void (*set_prior_entry_num)(IOPMP_RegDef *iopmp_reg, int value);
        int (*support_tor)(IOPMP_RegDef *iopmp_reg);
        int (*get_prient_prog)(IOPMP_RegDef *iopmp_reg);
        int (*get_chk_x)(IOPMP_RegDef *iopmp_reg);
        int (*get_no_x)(IOPMP_RegDef *iopmp_reg);
        int (*get_no_w)(IOPMP_RegDef *iopmp_reg);
        void (*set_rrid_transl)(IOPMP_RegDef *iopmp_reg, int value);
        int (*get_sps_en)(IOPMP_RegDef *iopmp_reg);
        int (*get_mfr_en)(IOPMP_RegDef *iopmp_reg);
        void (*enable)(IOPMP_RegDef *iopmp_reg);
        void (*stall_transaction)(IOPMP_RegDef *iopmp_reg);
        void (*resume_transaction)(IOPMP_RegDef *iopmp_reg);
        int (*rridscp_op)(IOPMP_RegDef *iopmp_reg, int rrid, int op);
        void (*error_reaction)(IOPMP_RegDef *iopmp_reg, unsigned long errreact);
        void (*set_msidata)(IOPMP_RegDef *iopmp_reg, int value);
        int (*get_msidata)(IOPMP_RegDef *iopmp_reg);
        int (*get_irq_pending)(IOPMP_RegDef *iopmp_reg);
        int (*get_error_ttype)(IOPMP_RegDef *iopmp_reg);
        int (*get_error_etype)(IOPMP_RegDef *iopmp_reg);
        int (*get_error_svc)(IOPMP_RegDef *iopmp_reg);
        uint64_t (*get_error_addr)(IOPMP_RegDef *iopmp_reg);
        int (*get_error_rrid)(IOPMP_RegDef *iopmp_reg);
        int (*get_error_eid)(IOPMP_RegDef *iopmp_reg);
        void (*clear_irq_pending)(IOPMP_RegDef *iopmp_reg);
        int (*get_err_mfr)(IOPMP_RegDef *iopmp_reg);
        int (*get_msi_werr)(IOPMP_RegDef *iopmp_reg);
        void (*clear_msi_werr)(IOPMP_RegDef *iopmp_reg);
        void (*set_mdcfg)(IOPMP_RegDef *iopmp_reg, int md, int t);
        void (*srcmd_lock)(IOPMP_RegDef *iopmp_reg, int rrid);
        void (*srcmd_add_md)(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx);
        void (*srcmd_remove_md)(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx);
        void (*srcmd_perm_add)(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx, int write);
        void (*srcmd_perm_remove)(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx, int write);
        void (*srcmd_r_add_md)(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx);
        void (*srcmd_r_remove_md)(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx);
        void (*srcmd_w_add_md)(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx);
        void (*srcmd_w_remove_md)(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx);
        void (*write_entry)(IOPMP_RegDef *iopmp_reg, int entry, unsigned int mode, void* va, unsigned long size, int cfg);
        int (*mdcfg_entry_num_get)(IOPMP_RegDef *iopmp_reg, int md);

        void (*napot_config)(IOPMP_RegDef *iopmp_reg, unsigned int entry, void* va, unsigned long size, int pmpcfg);
        void (*na4_config)(IOPMP_RegDef *iopmp_reg, unsigned int entry, void* va, int pmpcfg);
        void (*tor_config)(IOPMP_RegDef *iopmp_reg, unsigned int entry, void* va, int pmpcfg);
        void (*off_config)(IOPMP_RegDef *iopmp_reg, unsigned int entry, void* va);
} const RISCV_IOPMP_OPS;

#endif
