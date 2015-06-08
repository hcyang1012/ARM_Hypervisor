#ifndef __SWTPM_H
#define __SWTPM_H

#ifdef CONFIG_XEN
#include <xen/types.h>
#endif

#ifdef THIN_HYP
#include <asm/ept_violation.h>
#include <asm/traps.h>
#endif

#define TPM_CMD_BUF_SIZE	4096

// wizrampa
#define TPM_FAKE_MEM_BASE  0xfed50000
//#define TPM_MEM_LEN        0x5000
#define TPM_MEM_LEN        0x1000 // XXX: locality 0
#define TPM_MEM_BASE       0xfed40000

#define TCPA 0x41504354
#define VENDOR_ID 0x4554485A
#define REV_ID 0x01

// Base address
#define TPM_LOCALITY_BASE        0xfed40000
#define TPM_LOCALITY_0           TPM_LOCALITY_BASE
#define TPM_LOCALITY_1           (TPM_LOCALITY_BASE | 0x1000)
#define TPM_LOCALITY_2           (TPM_LOCALITY_BASE | 0x2000)
#define TPM_LOCALITY_3           (TPM_LOCALITY_BASE | 0x3000)
#define TPM_LOCALITY_4           (TPM_LOCALITY_BASE | 0x4000)
#define TPM_LOCALITY_BASE_N(n)   (TPM_LOCALITY_BASE | ((n) << 12))

#define TPM_REG_ACCESS 0x0000
#define TPM_REG_INT_ENABLE 0x0008
#define TPM_REG_INT_VECTOR 0x000C
#define TPM_REG_INT_STATUS 0x0010
#define TPM_REG_INTF_CAPS 0x0014
#define TPM_REG_STS 0x0018
#define TPM_REG_DATA_FIFO 0x0024
#define TPM_REG_DID_VID 0x0F00
#define TPM_REG_RID 0x0F04

#define TPM_CMD_SIZE_MAX   768
#define TPM_RSP_SIZE_MAX   768
// wizrampa

typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;

typedef union {
	u8 _raw[1];                      /* 1-byte reg */
	struct __attribute__ ((packed)) {
		u8 tpm_establishment   : 1;  /* RO, 0=T/OS has been established
												  before */
		u8 request_use         : 1;  /* RW, 1=locality is requesting TPM use */
		u8 pending_request     : 1;  /* RO, 1=other locality is requesting
												  TPM usage */
		u8 seize               : 1;  /* WO, 1=seize locality */
		u8 been_seized         : 1;  /* RW, 1=locality seized while active */
		u8 active_locality     : 1;  /* RW, 1=locality is active */
		u8 reserved            : 1;
		u8 tpm_reg_valid_sts   : 1;  /* RO, 1=other bits are valid */
	};
} tpm_reg_access_t;

typedef union {
    u8 _raw[4];                 /* 4-byte reg */
    struct __attribute__ ((packed)) {
		u8 dataAvailIntEnable       :  1; /* RW, 0= disable, 1= enable */
		u8 stsValidIntEnable        :  1; /* RW, 0= disable, 1= enable */
		u8 localityChangeIntEnable  :  1; /* RW, 0= disable, 1= enable */
		u8 typePolarity             :  2; /* RW, 01= lowlevel */
		u8 reserved1                :  2; /* always return 0 */
		u8 commandReadyEnable       :  1; /* RW, 0= disable, 1= enable */
		u32 reserved2               : 23; /* always return 0 */
		u8 globalIntEnable          :  1; /* RW, 0= all ints disable */
        };
} tpm_reg_int_enable_t;

typedef union {
	u8 _raw[3];                  /* 3-byte reg */
	struct __attribute__ ((packed)) {
		u8 reserved1       : 1;
		u8 response_retry  : 1;  /* WO, 1=re-send response */
		u8 reserved2       : 1;
		u8 expect          : 1;  /* RO, 1=more data for command expected */
		u8 data_avail      : 1;  /* RO, 0=no more data for response */
		u8 tpm_go          : 1;  /* WO, 1=execute sent command */
		u8 command_ready   : 1;  /* RW, 1=TPM ready to receive new cmd */
		u8 sts_valid       : 1;  /* RO, 1=data_avail and expect bits are
											 valid */
		u16 burst_count    : 16; /* RO, # read/writes bytes before wait */
	};
} tpm_reg_sts_t;

typedef union {
	u8 _raw[1];                      /* 1-byte reg */
} tpm_reg_data_fifo_t;

typedef union {
	u8 _raw[1]; /* 1-byte reg */
} tpm_reg_rid_t;

typedef union {
	u8 _raw[4];
	struct __attribute__ ((packed)) {
		u8 value3 : 8;
		u8 value2 : 8;
		u8 value1 : 8;
		u8 value0 : 8;
	};
} tpm_reg_did_vid_t;

typedef union {
	u32 _raw[4];
	struct __attribute__ ((packed)) {
		u8 data_avail : 1;
		u8 sts_valid : 1;
		u8 locality_change : 1;
		u8 int_level_high : 1;
		u8 int_level_low : 1;
		u8 int_edge_rising : 1;
		u8 int_edge_falling : 1;
		u8 cmd_ready : 1;
		u8 burst_count : 1;
		u8 data_transfer_size : 2;
		u32 reserved1 : 17;
		u8 interface_version : 3;
		u8 reserved2 : 1;
	};
} tpm_reg_intf_cap_t;

typedef union {
	u8 _raw[1];
	struct __attribute__ ((packed)) {
	    u8 sirqVec	:4;
	    u8 reserved0:4;
	};
} tpm_reg_int_vec_t;

int assert_swtpm_running (void);
int integrate_swtpm (u8 *in, u32 *in_size, u8 *out, u32 *out_size);
#ifdef THIN_HYP
void tpm_mmio_read(const struct ept_violation_info_t *info);
void tpm_mmio_write(const struct ept_violation_info_t *info);
#endif
#endif
