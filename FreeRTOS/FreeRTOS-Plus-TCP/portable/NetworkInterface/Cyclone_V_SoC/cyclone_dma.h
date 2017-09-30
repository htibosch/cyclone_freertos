#ifndef CYCLONE_DMA_H

#define CYCLONE_DMA_H

/* DMA CRS Control and Status Register Mapping */
#define DMA_BUS_MODE		0x00001000	/* Bus Mode */
#define DMA_XMT_POLL_DEMAND	0x00001004	/* Transmit Poll Demand */
#define DMA_RCV_POLL_DEMAND	0x00001008	/* Received Poll Demand */
#define DMA_RCV_BASE_ADDR	0x0000100c	/* Receive List Base */
#define DMA_TX_BASE_ADDR	0x00001010	/* Transmit List Base */
#define DMA_STATUS		0x00001014	/* Status Register */
#define DMA_CONTROL		0x00001018	/* Ctrl (Operational Mode) */
#define DMA_INTR_ENA		0x0000101c	/* Interrupt Enable */
#define DMA_MISSED_FRAME_CTR	0x00001020	/* Missed Frame Counter */


/*--- DMA BLOCK defines ---*/
/* DMA Bus Mode register defines */
#define DMA_BUS_MODE_DA		0x00000002	/* Arbitration scheme */
#define DMA_BUS_MODE_DSL_MASK	0x0000007c	/* Descriptor Skip Length */
#define DMA_BUS_MODE_DSL_SHIFT	2		/*   (in DWORDS)      */
/* Programmable burst length (passed thorugh platform)*/
#define DMA_BUS_MODE_PBL_MASK	0x00003f00	/* Programmable Burst Len */
#define DMA_BUS_MODE_PBL_SHIFT	8
#define DMA_BUS_MODE_ATDS	0x00000080	/* Alternate Descriptor Size */

enum rx_tx_priority_ratio {
	double_ratio = 0x00004000,	/* 2:1 */
	triple_ratio = 0x00008000,	/* 3:1 */
	quadruple_ratio = 0x0000c000,	/* 4:1 */
};

#define DMA_BUS_MODE_FB		0x00010000	/* Fixed burst */
#define DMA_BUS_MODE_MB		0x04000000	/* Mixed burst */
#define DMA_BUS_MODE_RPBL_MASK	0x007e0000	/* Rx-Programmable Burst Len */
#define DMA_BUS_MODE_RPBL_SHIFT	17
#define DMA_BUS_MODE_USP	0x00800000
#define DMA_BUS_MODE_MAXPBL	0x01000000
#define DMA_BUS_MODE_AAL	0x02000000

/* DMA Normal interrupt */
#define DMA_INTR_ENA_NIE 0x00010000	/* Normal Summary */
#define DMA_INTR_ENA_TIE 0x00000001	/* Transmit Interrupt */
#define DMA_INTR_ENA_TUE 0x00000004	/* Transmit Buffer Unavailable */
#define DMA_INTR_ENA_RIE 0x00000040	/* Receive Interrupt */
#define DMA_INTR_ENA_ERE 0x00004000	/* Early Receive */

#define DMA_INTR_NORMAL	(DMA_INTR_ENA_NIE | DMA_INTR_ENA_RIE | \
			DMA_INTR_ENA_TIE)

/* DMA Abnormal interrupt */
#define DMA_INTR_ENA_AIE 0x00008000	/* Abnormal Summary */
#define DMA_INTR_ENA_FBE 0x00002000	/* Fatal Bus Error */
#define DMA_INTR_ENA_ETE 0x00000400	/* Early Transmit */
#define DMA_INTR_ENA_RWE 0x00000200	/* Receive Watchdog */
#define DMA_INTR_ENA_RSE 0x00000100	/* Receive Stopped */
#define DMA_INTR_ENA_RUE 0x00000080	/* Receive Buffer Unavailable */
#define DMA_INTR_ENA_UNE 0x00000020	/* Tx Underflow */
#define DMA_INTR_ENA_OVE 0x00000010	/* Receive Overflow */
#define DMA_INTR_ENA_TJE 0x00000008	/* Transmit Jabber */
#define DMA_INTR_ENA_TSE 0x00000002	/* Transmit Stopped */

#define DMA_INTR_ABNORMAL	(DMA_INTR_ENA_AIE | DMA_INTR_ENA_FBE | \
				DMA_INTR_ENA_UNE)

/* DMA default interrupt mask */
#define DMA_INTR_DEFAULT_MASK	(DMA_INTR_NORMAL | DMA_INTR_ABNORMAL)

/* AXI Master Bus Mode */
#define DMA_AXI_BUS_MODE	0x00001028

#define DMA_AXI_EN_LPI		BIT(31)
#define DMA_AXI_LPI_XIT_FRM	BIT(30)
#define DMA_AXI_WR_OSR_LMT	GENMASK(23, 20)
#define DMA_AXI_WR_OSR_LMT_SHIFT	20
#define DMA_AXI_WR_OSR_LMT_MASK	0xf
#define DMA_AXI_RD_OSR_LMT	GENMASK(19, 16)
#define DMA_AXI_RD_OSR_LMT_SHIFT	16
#define DMA_AXI_RD_OSR_LMT_MASK	0xf

#define DMA_AXI_OSR_MAX		0xf
#define DMA_AXI_MAX_OSR_LIMIT ((DMA_AXI_OSR_MAX << DMA_AXI_WR_OSR_LMT_SHIFT) | \
			       (DMA_AXI_OSR_MAX << DMA_AXI_RD_OSR_LMT_SHIFT))
#define	DMA_AXI_1KBBE		BIT(13)
#define DMA_AXI_AAL		BIT(12)
#define DMA_AXI_BLEN256		BIT(7)
#define DMA_AXI_BLEN128		BIT(6)
#define DMA_AXI_BLEN64		BIT(5)
#define DMA_AXI_BLEN32		BIT(4)
#define DMA_AXI_BLEN16		BIT(3)
#define DMA_AXI_BLEN8		BIT(2)
#define DMA_AXI_BLEN4		BIT(1)
#define DMA_BURST_LEN_DEFAULT	(DMA_AXI_BLEN256 | DMA_AXI_BLEN128 | \
				 DMA_AXI_BLEN64 | DMA_AXI_BLEN32 | \
				 DMA_AXI_BLEN16 | DMA_AXI_BLEN8 | \
				 DMA_AXI_BLEN4)

#define DMA_AXI_UNDEF		BIT(0)

#define AXI_BLEN	7

struct gmac_tx_descriptor
{
	uint32_t
		deferred_bit : 1,			/*  0 */
		underflow_error : 1,        /*  1 */
		excessive_deferral : 1,     /*  2 */
		collision_count : 4,        /*  3 */
		vlan_frame : 1,             /*  7 */
		excessive_collision : 1,    /*  8 */
		late_collision : 1,         /*  9 */
		no_carrier : 1,             /* 10 */
		loss_of_carrier : 1,        /* 11 */
		IP_payload_error : 1,       /* 12 */
		frame_flushed : 1,          /* 13 */
		jabber_timeout : 1,         /* 14 */
		error_summary : 1,          /* 15 */
		ip_header_error : 1,        /* 16 */
		transmit_ttamp_status : 1,	/* 17 This field is used as a status bit to indicate
									      that a timestamp was captured for the described transmit frame */
		reserved_2 : 2,				/* 18 */
		second_addr_chained : 1,	/* 20 When set, this bit indicates that the second address
									      in the descriptor is the Next descriptor
									      address rather than the second buffer address */
		transmit_end_of_ting : 1,	/* 21 When set, this bit indicates that the descriptor list
									      reached its final descriptor */
		checksum_mode : 2,			/* 22 0x0 : none. 0x1: IP-header, 2: IP & payload, 3: complete */
		reserved_1 : 1,				/* 24 */
		timestamp_enable : 1		/* 25 */,
		disable_pad : 1,			/* 26 When set, the MAC does not add padding to a frame shorter than 64 bytes. */
		disable_crc : 1,			/* 27 */
		first_segment : 1,			/* 28 */
		last_segment : 1,			/* 29 */
		int_on_completion : 1,		/* 30 */
		own : 1;					/* 31 */
	uint32_t
		buf1_byte_count : 13,
		res1 : 3,
		buf2_byte_count : 13,
		res2 : 3;
	uint32_t
		buf1_address;
	uint32_t
		next_descriptor;
};

struct gmac_rx_descriptor
{
	uint32_t
		ext_status_available : 1,	/*  0 */
		crc_error : 1,				/*  1 */
		dribble_bit_error : 1,		/*  2 */
		receive_error : 1,			/*  3 */
		recv_wdt_timeout : 1,		/*  4 */
		frame_type : 1,				/*  5 */
		late_collision : 1,			/*  6 */
		time_stamp_available : 1,	/*  7 */
		last_descriptor : 1,		/*  8 */
		first_descriptor : 1,		/*  9 */
		vlan_tag : 1,				/* 10 */
		overflow_error : 1,			/* 11 */
		length_error : 1,			/* 12 */
		source_addr_filter_fail : 1,/* 13 Source Address Filter Fail */
		description_error : 1,		/* 14 */
		error_summary : 1,			/* 15 */
		frame_length : 14,			/* 16 */
		dest_addr_filter_fail : 1,	/* 30 Destination Address Filter Fail */
		own : 1;					/* 31 */
	uint32_t
		buf1_byte_count : 13,
		res1 : 3,
		buf2_byte_count : 13,
		res2 : 3;
	uint32_t
		buf1_address;
	uint32_t
		next_descriptor;
};

typedef struct gmac_tx_descriptor gmac_tx_descriptor_t;
typedef struct gmac_rx_descriptor gmac_rx_descriptor_t;

struct xEMACInterface {
};

typedef struct xEMACInterface EMACInterface_t;

void gmac_check_rx( EMACInterface_t *pxEMACif );
void gmac_check_tx( EMACInterface_t *pxEMACif );
void gmac_check_errors( EMACInterface_t *pxEMACif );

struct stmmac_axi
{
	/* When set to 1, this bit enables the LPI mode */
	bool axi_lpi_en;
	/* When set to 1, this bit enables the GMAC-AXI to
	 * come out of the LPI mode only when the Remote
	 * Wake Up Packet is received. When set to 0, this bit
	 * enables the GMAC-AXI to come out of LPI mode
	 * when any frame is received. This bit must be set to 0. */
	bool axi_xit_frm;
	/* AXI Maximum Write OutStanding Request Limit. */
	uint32_t axi_wr_osr_lmt;
	/* This value limits the maximum outstanding request
	 * on the AXI read interface. Maximum outstanding
	 * requests = RD_OSR_LMT+1 */
	uint32_t axi_rd_osr_lmt;
	/*  1 KB Boundary Crossing Enable for the GMAC-AXI
	 * Master When set, the GMAC-AXI Master performs
	 * burst transfers that do not cross 1 KB boundary.
	 * When reset, the GMAC-AXI Master performs burst
	 * transfers that do not cross 4 KB boundary.*/
	bool axi_kbbe;
	uint32_t axi_blen[AXI_BLEN];
};

struct stmmac_dma_cfg
{
	int pbl;
	int txpbl;
	int rxpbl;
	bool pblx8;
	int fixed_burst;
	int mixed_burst;
	bool aal;
};

extern void dwmac1000_dma_axi( int iMacID, struct stmmac_axi *axi );
extern void dwmac1000_dma_init( int iMacID,
			       struct stmmac_dma_cfg *dma_cfg,
			       uint32_t dma_tx, uint32_t dma_rx, int atds );

#endif /* CYCLONE_DMA_H */
