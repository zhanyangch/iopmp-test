#include "riscv_iopmp.h"
#include "platform.h"

/* Entry configuration */
#define NAPOT(base, size)                       (uintptr_t)(((size) > 0) ? ((((uintptr_t)(base) & (~((uintptr_t)(size) - 1))) >> 2) | (((uintptr_t)(size) - 1) >> 3)) : 0)
#define NA4(base)                               (uintptr_t)((base) >> 2)
#define TOR(top)                                (unsigned long)((unsigned long)(top) >> 2)

#define SCHEME_NAPOT                            3
#define SCHEME_NA4                              2
#define SCHEME_TOR                              1
#define SCHEME_OFF                              0
#define IOPMP_A_TOR                             SCHEME_TOR
#define IOPMP_A_NA4                             SCHEME_NA4
#define IOPMP_A_NAPOT                           SCHEME_NAPOT
#define IOPMP_A_OFF                             SCHEME_OFF

#define ENTRY_CFG_A                             3
#define ENTRY_CFG_SIRE                          5
#define ENTRY_CFG_SIWE                          6
#define ENTRY_CFG_SIXE                          7
#define ENTRY_CFG_SERE                          8
#define ENTRY_CFG_SEWE                          9
#define ENTRY_CFG_SEXE                          10

/* Association table between requestor and memory domain */
#define SRCMD_EN_L 0
#define SRCMD_EN_MD 1

#define IOPMP_HWCFG0_MDCFG_FMT          0
#define IOPMP_HWCFG0_MDCFG_FMT_MASK     0x3
#define IOPMP_HWCFG0_SRCMD_FMT          2
#define IOPMP_HWCFG0_SRCMD_FMT_MASK     0x3
#define IOPMP_HWCFG0_TOR_EN             4
#define IOPMP_HWCFG0_SPS_EN             5
#define IOPMP_HWCFG0_PRIENT_PROG        7
#define IOPMP_HWCFG0_CHK_X              10
#define IOPMP_HWCFG0_NO_X               11
#define IOPMP_HWCFG0_NO_W               12
#define IOPMP_HWCFG0_MFR_EN             16
#define IOPMP_HWCFG0_MD_ENTRY_NUM       17
#define IOPMP_HWCFG0_MD_ENTRY_NUM_MASK  0x7F
#define IOPMP_HWCFG0_MD_NUM             24
#define IOPMP_HWCFG0_MD_NUM_MASK        0x3F
#define IOPMP_HWCFG0_ENABLE             31

#define IOPMP_HWCFG1_RRID_NUM           0
#define IOPMP_HWCFG1_RRID_NUM_MASK      0xFFFF
#define IOPMP_HWCFG1_ENTRY_NUM          16
#define IOPMP_HWCFG1_ENTRY_NUM_MASK     0xFFFF

#define IOPMP_HWCFG2_PRIO_ENTRY         0
#define IOPMP_HWCFG2_PRIO_ENTRY_MASK    0xFFFF
#define IOPMP_HWCFG2_RRID_TRANSL        16
#define IOPMP_HWCFG2_RRID_TRANSL_MASK   0xFFFF

#define IOPMP_ERR_CFG_IE_ENABLE                      (1 << 1)
#define IOPMP_ERR_CFG_RS_ENABLE                      (1 << 2)
#define IOPMP_ERR_CFG_MSI_EN_ENABLE                  (1 << 3)
#define IOPMP_ERR_CFG_STALL_VIOLATION_EN_ENABLE      (1 << 4)

#define IOPMP_ERR_CFG_MSI_EN                3
#define IOPMP_ERR_CFG_STALL_VIOLATION_EN    4
#define IOPMP_ERR_CFG_MSIDATA               8
#define IOPMP_ERR_CFG_MSIDATA_MASK          0xF

#define IOPMP_ERR_INFO_V                0
#define IOPMP_ERR_INFO_TTYPE            1
#define IOPMP_ERR_INFO_TTYPE_MASK       0x3
#define IOPMP_ERR_INFO_MSI_WERR         3
#define IOPMP_ERR_INFO_ETYPE            4
#define IOPMP_ERR_INFO_ETYPE_MASK       0xF
#define IOPMP_ERR_INFO_SVC              8

#define IOPMP_ERR_REQID_RRID            0
#define IOPMP_ERR_REQID_RRID_MASK       0xFFFF
#define IOPMP_ERR_REQID_EID             16
#define IOPMP_ERR_REQID_EID_MASK        0xFFFF

/* General Functions */
int iopmp_get_imp(IOPMP_RegDef *iopmp_reg) {
	return iopmp_reg->IMPLEMENTATION;
}

int iopmp_get_hwcfg(IOPMP_RegDef *iopmp_reg, int cfg_num) {
	if (cfg_num == 0) {
		return iopmp_reg->HWCFG0;
	} else if (cfg_num == 1) {
		return iopmp_reg->HWCFG1;
	} else if (cfg_num == 2) {
		return iopmp_reg->HWCFG2;
	}
    return 0;
}

int iopmp_get_srcmd_fmt(IOPMP_RegDef *iopmp_reg) {
    return (iopmp_get_hwcfg(iopmp_reg, 0) >> IOPMP_HWCFG0_SRCMD_FMT) & IOPMP_HWCFG0_SRCMD_FMT_MASK;
}

int iopmp_get_mdcfg_fmt(IOPMP_RegDef *iopmp_reg) {
    return (iopmp_get_hwcfg(iopmp_reg, 0) >> IOPMP_HWCFG0_MDCFG_FMT) & IOPMP_HWCFG0_MDCFG_FMT_MASK;
}

int iopmp_get_md_entry_num(IOPMP_RegDef *iopmp_reg) {
    return (iopmp_get_hwcfg(iopmp_reg, 0) >> IOPMP_HWCFG0_MD_ENTRY_NUM) & IOPMP_HWCFG0_MD_ENTRY_NUM_MASK;
}

void iopmp_set_md_entry_num(IOPMP_RegDef *iopmp_reg, int value) {
    int v = iopmp_reg->HWCFG0 & ~(IOPMP_HWCFG0_MD_ENTRY_NUM_MASK << IOPMP_HWCFG0_MD_ENTRY_NUM);
    iopmp_reg->HWCFG0 = v | (value & IOPMP_HWCFG0_MD_ENTRY_NUM_MASK) << IOPMP_HWCFG0_MD_ENTRY_NUM;
}

int iopmp_get_md_num(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_get_hwcfg(iopmp_reg, 0) >> IOPMP_HWCFG0_MD_NUM) & IOPMP_HWCFG0_MD_NUM_MASK;
}

int iopmp_get_rrid_num(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_get_hwcfg(iopmp_reg, 1) >> IOPMP_HWCFG1_RRID_NUM) & IOPMP_HWCFG1_RRID_NUM_MASK;
}

int iopmp_get_entry_num(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_get_hwcfg(iopmp_reg, 1) >> IOPMP_HWCFG1_ENTRY_NUM) & IOPMP_HWCFG1_ENTRY_NUM_MASK;
}

int iopmp_get_prior_entry_num(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_get_hwcfg(iopmp_reg, 2) >> IOPMP_HWCFG2_PRIO_ENTRY) & IOPMP_HWCFG2_PRIO_ENTRY_MASK;
}

void iopmp_set_prior_entry_num(IOPMP_RegDef *iopmp_reg, int value) {
	int v = iopmp_reg->HWCFG2 & ~(IOPMP_HWCFG2_PRIO_ENTRY_MASK << IOPMP_HWCFG2_PRIO_ENTRY);
	iopmp_reg->HWCFG2 = v | ((value & IOPMP_HWCFG2_PRIO_ENTRY_MASK) << IOPMP_HWCFG2_PRIO_ENTRY);
}

int iopmp_support_tor(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_get_hwcfg(iopmp_reg, 0) >> IOPMP_HWCFG0_TOR_EN) & 0x1;
}

int iopmp_get_prient_prog(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_get_hwcfg(iopmp_reg, 0) >> IOPMP_HWCFG0_PRIENT_PROG) & 0x1;
}

int iopmp_get_chk_x(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_get_hwcfg(iopmp_reg, 0) >> IOPMP_HWCFG0_CHK_X) & 0x1;
}

int iopmp_get_no_x(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_get_hwcfg(iopmp_reg, 0) >> IOPMP_HWCFG0_NO_X) & 0x1;
}

int iopmp_get_no_w(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_get_hwcfg(iopmp_reg, 0) >> IOPMP_HWCFG0_NO_W) & 0x1;
}

void iopmp_set_rrid_transl(IOPMP_RegDef *iopmp_reg, int value) {
	int v = iopmp_reg->HWCFG2 & ~(IOPMP_HWCFG2_RRID_TRANSL_MASK << IOPMP_HWCFG2_RRID_TRANSL);
	iopmp_reg->HWCFG2 = v | ((value & IOPMP_HWCFG2_RRID_TRANSL_MASK) << IOPMP_HWCFG2_RRID_TRANSL);
}

int iopmp_get_sps_en(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_get_hwcfg(iopmp_reg, 0) >> IOPMP_HWCFG0_SPS_EN) & 0x1;
}

int iopmp_get_mfr_en(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_get_hwcfg(iopmp_reg, 0) >> IOPMP_HWCFG0_MFR_EN) & 0x1;
}

void iopmp_enable(IOPMP_RegDef *iopmp_reg) {
	iopmp_reg->HWCFG0 |= 1 << IOPMP_HWCFG0_ENABLE;
}

void iopmp_stall_transaction(IOPMP_RegDef *iopmp_reg) {
    iopmp_reg->MDSTALL = 1;
}

void iopmp_resume_transaction(IOPMP_RegDef *iopmp_reg) {
    iopmp_reg->MDSTALL = 0;
}

int iopmp_rridscp_op(IOPMP_RegDef *iopmp_reg, int rrid, int op) {
    int value = rrid | op << 30;
    iopmp_reg->RRIDSCP = value;
    return iopmp_reg->RRIDSCP >> 30;
}

void iopmp_error_reaction(IOPMP_RegDef *iopmp_reg, unsigned long err_cfg) {
    int value = iopmp_reg->ERR_CFG;
    if (err_cfg & ERR_CFG_CTRL_IE_ENABLE) {
        value |= IOPMP_ERR_CFG_IE_ENABLE;
    } else {
        value &= ~IOPMP_ERR_CFG_IE_ENABLE;
    }
    if (err_cfg & ERR_CFG_CTRL_RS_ENABLE) {
        value |= IOPMP_ERR_CFG_RS_ENABLE;
    } else {
        value &= ~IOPMP_ERR_CFG_RS_ENABLE;
    }
    if (err_cfg & ERR_CFG_CTRL_MSI_EN_ENABLE) {
        value |= IOPMP_ERR_CFG_MSI_EN_ENABLE;
    } else {
        value &= ~IOPMP_ERR_CFG_MSI_EN_ENABLE;
    }
    if (err_cfg & ERR_CFG_CTRL_STALL_VIOLATION_EN_ENABLE) {
        value |= IOPMP_ERR_CFG_STALL_VIOLATION_EN_ENABLE;
    } else {
        value &= ~IOPMP_ERR_CFG_STALL_VIOLATION_EN_ENABLE;
    }
	iopmp_reg->ERR_CFG = value;
}

void iopmp_set_msidata(IOPMP_RegDef *iopmp_reg, int value) {
    int cfg = iopmp_reg->ERR_CFG;
    cfg &= ~(IOPMP_ERR_CFG_MSIDATA_MASK << IOPMP_ERR_CFG_MSIDATA);
    cfg |= (value & IOPMP_ERR_CFG_MSIDATA_MASK) << IOPMP_ERR_CFG_MSIDATA;
    iopmp_reg->ERR_CFG = cfg;
}

int iopmp_get_msidata(IOPMP_RegDef *iopmp_reg) {
    return (iopmp_reg->ERR_CFG >> IOPMP_ERR_CFG_MSIDATA) & IOPMP_ERR_CFG_MSIDATA_MASK;
}

int iopmp_get_irq_pending(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_reg->ERR_INFO >> IOPMP_ERR_INFO_V) & 0x1;
}

int iopmp_get_error_ttype(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_reg->ERR_INFO >> IOPMP_ERR_INFO_TTYPE) & IOPMP_ERR_INFO_TTYPE_MASK;
}

int iopmp_get_error_etype(IOPMP_RegDef *iopmp_reg) {
	return (iopmp_reg->ERR_INFO >> IOPMP_ERR_INFO_ETYPE) & IOPMP_ERR_INFO_ETYPE_MASK;
}

int iopmp_get_error_svc(IOPMP_RegDef *iopmp_reg) {
    return (iopmp_reg->ERR_INFO >> IOPMP_ERR_INFO_SVC) & 0x1;
}

uint64_t iopmp_get_error_addr(IOPMP_RegDef *iopmp_reg) {
    return (iopmp_reg->ERR_REQADDRH << 30) | iopmp_reg->ERR_REQADDR >> 2;
}

int iopmp_get_error_rrid(IOPMP_RegDef *iopmp_reg) {
    return (iopmp_reg->ERR_REQID >> IOPMP_ERR_REQID_RRID) & IOPMP_ERR_REQID_RRID_MASK;
}

int iopmp_get_error_eid(IOPMP_RegDef *iopmp_reg) {
    return (iopmp_reg->ERR_REQID >> IOPMP_ERR_REQID_EID) & IOPMP_ERR_REQID_EID_MASK;
}

int iopmp_get_err_mfr(IOPMP_RegDef *iopmp_reg) {
    return iopmp_reg->ERR_MFR;
}

int iopmp_get_msi_werr(IOPMP_RegDef *iopmp_reg) {
    return (iopmp_reg->ERR_INFO >> IOPMP_ERR_INFO_MSI_WERR) & 0x1;
}

void iopmp_clear_msi_werr(IOPMP_RegDef *iopmp_reg) {
    iopmp_reg->ERR_INFO |= 0x1 << IOPMP_ERR_INFO_MSI_WERR;
}

void iopmp_set_mdcfg(IOPMP_RegDef *iopmp_reg, int md, int t) {
    iopmp_reg->MDCFG[md] = t;
}

void iopmp_clear_irq_pending(IOPMP_RegDef *iopmp_reg) {
	iopmp_reg->ERR_INFO = 1 << IOPMP_ERR_INFO_V;
}

void iopmp_srcmd_lock(IOPMP_RegDef *iopmp_reg, int rrid) {
	iopmp_reg->SRCMD[rrid].EN |= (0x1 << SRCMD_EN_L);
}

void iopmp_srcmd_add_md(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx) {
    if (iopmp_get_srcmd_fmt(iopmp_reg) != 2) {
        if (md_idx < 31) {
            iopmp_reg->SRCMD[rrid].EN = (iopmp_reg->SRCMD[rrid].EN | (1ULL << (md_idx + 1)));
        } else {
            iopmp_reg->SRCMD[rrid].ENH = (iopmp_reg->SRCMD[rrid].ENH | (1UL << (md_idx - 31)));
        }
    }
}

void iopmp_srcmd_remove_md(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx) {
    if (iopmp_get_srcmd_fmt(iopmp_reg) != 2) {
        if (md_idx < 31) {
            iopmp_reg->SRCMD[rrid].EN = (iopmp_reg->SRCMD[rrid].EN & (~(1ULL << (md_idx + 1))));
        } else {
            iopmp_reg->SRCMD[rrid].ENH = (iopmp_reg->SRCMD[rrid].ENH & (~(1ULL << (md_idx - 31))));
        }
    }
}

void iopmp_srcmd_perm_add(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx, int write) {
    if (iopmp_get_srcmd_fmt(iopmp_reg) == 2) {
        if (rrid < 16) {
            iopmp_reg->SRCMD[md_idx].PERM |= (1 << (rrid * 2 + write));
        } else {
            iopmp_reg->SRCMD[md_idx].PERMH |= (1 << ((rrid - 16) * 2 + write));
        }
    }
}

void iopmp_srcmd_perm_remove(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx, int write) {
    if (iopmp_get_srcmd_fmt(iopmp_reg) == 2) {
        if (rrid < 16) {
            iopmp_reg->SRCMD[md_idx].PERM &= ~(1 << (rrid * 2 + write));
        } else {
            iopmp_reg->SRCMD[md_idx].PERMH &= ~(1 << ((rrid - 16) * 2 + write));
        }
    }
}

void iopmp_srcmd_r_add_md(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx) {
    if (iopmp_get_sps_en(iopmp_reg)) {
        if (md_idx < 31) {
            iopmp_reg->SRCMD[rrid].R = (iopmp_reg->SRCMD[rrid].R | (1ULL << (md_idx + 1)));
        } else {
            iopmp_reg->SRCMD[rrid].RH = (iopmp_reg->SRCMD[rrid].RH | (1UL << (md_idx - 31)));
        }
    }
}

void iopmp_srcmd_r_remove_md(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx) {
    if (iopmp_get_sps_en(iopmp_reg)) {
        if (md_idx < 31) {
            iopmp_reg->SRCMD[rrid].R = (iopmp_reg->SRCMD[rrid].R & (~(1ULL << (md_idx + 1))));
        } else {
            iopmp_reg->SRCMD[rrid].RH = (iopmp_reg->SRCMD[rrid].RH & (~(1ULL << (md_idx - 31))));
        }
    }
}

void iopmp_srcmd_w_add_md(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx) {
    if (iopmp_get_sps_en(iopmp_reg)) {
        if (md_idx < 31) {
            iopmp_reg->SRCMD[rrid].W = (iopmp_reg->SRCMD[rrid].W | (1ULL << (md_idx + 1)));
        } else {
            iopmp_reg->SRCMD[rrid].WH = (iopmp_reg->SRCMD[rrid].WH | (1UL << (md_idx - 31)));
        }
    }
}

void iopmp_srcmd_w_remove_md(IOPMP_RegDef *iopmp_reg, int rrid, int md_idx) {
    if (iopmp_get_sps_en(iopmp_reg)) {
        if (md_idx < 31) {
            iopmp_reg->SRCMD[rrid].WH = (iopmp_reg->SRCMD[rrid].W & (~(1ULL << (md_idx + 1))));
        } else {
            iopmp_reg->SRCMD[rrid].WH = (iopmp_reg->SRCMD[rrid].WH & (~(1ULL << (md_idx - 31))));
        }
    }
}

void iopmp_write_entry(IOPMP_RegDef *iopmp_reg,int entry, unsigned int mode,  void* va, unsigned long size, int cfg) {
	unsigned long addr = (unsigned long)va;

	if (mode == IOPMP_A_OFF) {
		addr >>= 2;
	}
	if (mode == IOPMP_A_TOR) {
		addr = TOR(addr);
	}
	if (mode == IOPMP_A_NA4) {
		addr >>= 2;
	}
	if (mode == IOPMP_A_NAPOT) {
		addr = NAPOT(addr, size);
	}

	iopmp_reg->ENTRY[entry].ADDR = addr & 0xFFFFFFFF;
	iopmp_reg->ENTRY[entry].CFG = (cfg | mode << ENTRY_CFG_A);
}

void iopmp_napot_config(IOPMP_RegDef *iopmp_reg, unsigned int entry, void* va, unsigned long size, int pmpcfg)
{
	iopmp_write_entry(iopmp_reg, entry, IOPMP_A_NAPOT, va, size, pmpcfg);
}

void iopmp_na4_config(IOPMP_RegDef *iopmp_reg, unsigned int entry, void* va, int pmpcfg)
{
	iopmp_write_entry(iopmp_reg, entry, IOPMP_A_NA4, va, 4, pmpcfg);
}

void iopmp_tor_config(IOPMP_RegDef *iopmp_reg, unsigned int entry, void* va, int pmpcfg)
{
	iopmp_write_entry(iopmp_reg, entry, IOPMP_A_TOR, va, 0, pmpcfg);
}

void iopmp_off_config(IOPMP_RegDef *iopmp_reg, unsigned int entry, void* va)
{
	iopmp_write_entry(iopmp_reg, entry, IOPMP_A_OFF, va, 0, 0);
}

int iopmp_get_mdcfg(IOPMP_RegDef *iopmp_reg, int md) {
	return iopmp_reg->MDCFG[md];
}

RISCV_IOPMP_OPS riscv_iopmp_ops = {
        iopmp_get_hwcfg,
        iopmp_get_srcmd_fmt,
        iopmp_get_mdcfg_fmt,
        iopmp_get_md_entry_num,
        iopmp_set_md_entry_num,
        iopmp_get_md_num,
        iopmp_get_rrid_num,
        iopmp_get_entry_num,
        iopmp_get_prior_entry_num,
        iopmp_set_prior_entry_num,
        iopmp_support_tor,
        iopmp_get_prient_prog,
        iopmp_get_chk_x,
        iopmp_get_no_x,
        iopmp_get_no_w,
        iopmp_set_rrid_transl,
        iopmp_get_sps_en,
        iopmp_get_mfr_en,
        iopmp_enable,
        iopmp_stall_transaction,
        iopmp_resume_transaction,
        iopmp_rridscp_op,
        iopmp_error_reaction,
        iopmp_set_msidata,
        iopmp_get_msidata,
        iopmp_get_irq_pending,
        iopmp_get_error_ttype,
        iopmp_get_error_etype,
        iopmp_get_error_svc,
        iopmp_get_error_addr,
        iopmp_get_error_rrid,
        iopmp_get_error_eid,
        iopmp_clear_irq_pending,
        iopmp_get_err_mfr,
        iopmp_get_msi_werr,
        iopmp_clear_msi_werr,
        iopmp_set_mdcfg,
        iopmp_srcmd_lock,
        iopmp_srcmd_add_md,
        iopmp_srcmd_remove_md,
        iopmp_srcmd_perm_add,
        iopmp_srcmd_perm_remove,
        iopmp_srcmd_r_add_md,
        iopmp_srcmd_r_remove_md,
        iopmp_srcmd_w_add_md,
        iopmp_srcmd_w_remove_md,
        iopmp_write_entry,
        iopmp_get_mdcfg,
        iopmp_napot_config,
        iopmp_na4_config,
        iopmp_tor_config,
        iopmp_off_config,
};