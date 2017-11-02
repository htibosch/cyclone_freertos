/*
 * Access functions to the hard-wired EMAC in Cyclone V SoC
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"
#include "NetworkInterface.h"

#include "cyclone_dma.h"
#include "cyclone_emac.h"

#include "socal/alt_rstmgr.h"

#include "UDPLoggingPrintf.h"

#ifndef ARRAY_SIZE
	#define ARRAY_SIZE(x)	(BaseType_t)(sizeof(x)/sizeof(x)[0])
#endif

/* The lowest 8 bits contain the IP's version number. We expect 0x37. */
#define HAS_GMAC4		( ( ulEMACVersion & 0xff ) >= 0x40 )

extern const uint8_t ucMACAddress[ 6 ];

uint32_t ulEMACVersion;
uint32_t ulUsePHYAddress;

uint32_t dwmac1000_read_version( int iMacID )
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t value;

	value = readl(ioaddr + GMAC_VERSION);
	return value;
}

void dwmac1000_core_init(int iMacID, EMACDeviceInfo_t *hw, int mtu)
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t value;

	value = readl(ioaddr + GMAC_CONTROL);

	/* Configure GMAC core */
	value |= GMAC_CORE_INIT;

	if (mtu > 1500)
		value |= GMAC_CONTROL_2K;
	if (mtu > 2000)
		value |= GMAC_CONTROL_JE;

	if (hw->ps) {
		value |= GMAC_CONTROL_TE;

		if (hw->ps == SPEED_1000) {
			value &= ~GMAC_CONTROL_PS;
			//value |= GMAC_CONTROL_FES;
			value &= ~GMAC_CONTROL_FES;
		} else {
			value |= GMAC_CONTROL_PS;

//			if (hw->ps == SPEED_10)
				value &= ~GMAC_CONTROL_FES;
//			else
//				value |= GMAC_CONTROL_FES;
		}
	}

	value |= GMAC_CONTROL_DM;

	writel(value, ioaddr + GMAC_CONTROL);

	/* Mask GMAC interrupts.
	This will be set later. */
	writel(0u, ioaddr + GMAC_INT_MASK);

	/* get flow-control same as Linux. */
	value = 0xFFFF0008;
	writel(value, ioaddr + GMAC_FLOW_CTRL);

#ifdef STMMAC_VLAN_TAG_USED
	/* Tag detection without filtering */
	writel(0x0, ioaddr + GMAC_VLAN_TAG);
#endif
}

/* Enable disable MAC RX/TX */
void gmac_enable_transmission(int iMacID, bool enable)
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t value;

	value = readl(ioaddr + MAC_CTRL_REG);

	if (enable)
	{
		value |= MAC_ENABLE_RX | MAC_ENABLE_TX;
	}
	else
	{
		value &= ~( MAC_ENABLE_TX | MAC_ENABLE_RX );
	}

	writel(value, ioaddr + MAC_CTRL_REG);
}

int dwmac1000_rx_ipc_enable(int iMacID, bool enable)
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t value;

	value = readl(ioaddr + GMAC_CONTROL);

	/*
	 * IPC : When this bit is set, the MAC calculates the 16-bit
	 * ones complement of the ones complement sum of all
	 * received Ethernet frame payloads.
	 */

	if (enable)
		value |= GMAC_CONTROL_IPC;
	else
		value &= ~GMAC_CONTROL_IPC;

	writel(value, ioaddr + GMAC_CONTROL);

	value = readl(ioaddr + GMAC_CONTROL);

	return !!(value & GMAC_CONTROL_IPC);
}

void stmmac_set_mac_addr( int iMacID, const uint8_t addr[6],
			 uint32_t high, uint32_t low)
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t data;

	data = (addr[5] << 8) | addr[4];
	/* For MAC Addr registers we have to set the Address Enable (AE)
	 * bit that has no effect on the High Reg 0 where the bit 31 (MO)
	 * is RO.
	 */
	writel(data | GMAC_HI_REG_AE, ioaddr + high);
	data = (addr[3] << 24) | (addr[2] << 16) | (addr[1] << 8) | addr[0];
	writel(data, ioaddr + low);
}

void dwmac1000_set_umac_addr(int iMacID,
				    const uint8_t *ucMACAddress,
				    uint32_t reg_n)
{
	stmmac_set_mac_addr(iMacID, ucMACAddress, GMAC_ADDR_HIGH(reg_n),
			    GMAC_ADDR_LOW(reg_n));
}

static void dwmac1000_set_eee_mode( int iMacID, int status,
				   bool en_tx_lpi_clockgating)
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t value;

	/*TODO - en_tx_lpi_clockgating treatment */

	/* Enable the link status receive on RGMII, SGMII ore SMII
	 * receive path and instruct the transmit to enter in LPI
	 * state.
	 */
	value = readl(ioaddr + LPI_CTRL_STATUS);
	if( status )
	{
		value |= LPI_CTRL_STATUS_LPIEN | LPI_CTRL_STATUS_LPITXA;
	} else {
		value &= ~( LPI_CTRL_STATUS_LPIEN | LPI_CTRL_STATUS_LPITXA );
	}
	writel(value, ioaddr + LPI_CTRL_STATUS);
}

static void dwmac1000_reset_eee_mode( int iMacID )
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t value;

	value = readl(ioaddr + LPI_CTRL_STATUS);
	value &= ~(LPI_CTRL_STATUS_LPIEN | LPI_CTRL_STATUS_LPITXA);
	writel(value, ioaddr + LPI_CTRL_STATUS);
}

static void dwmac1000_set_eee_pls( int iMacID, int link )
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t value;

	value = readl(ioaddr + LPI_CTRL_STATUS);

	/* PHY Link Status */
	if (link)
		value |= LPI_CTRL_STATUS_PLS;
	else
		value &= ~LPI_CTRL_STATUS_PLS;

	writel(value, ioaddr + LPI_CTRL_STATUS);
}

static void dwmac1000_set_eee_pls_enable( int iMacID, int link )
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t value;

	value = readl(ioaddr + LPI_CTRL_STATUS);

	/* PHY Link Status Enable */

	if (link)
		value |= LPI_CTRL_STATUS_PLSEN;
	else
		value &= ~LPI_CTRL_STATUS_PLSEN;

	writel(value, ioaddr + LPI_CTRL_STATUS);
}

static void dwmac1000_set_eee_timer( int iMacID, int ls, int tw )
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t value;

	/*
	 * lst: This field specifies the minimum time (in milliseconds) for which the link status from the PHY should
	 * be up (OKAY) before the LPI pattern can be
	 * transmitted to the PHY.
	 */
	/*
	 * twt: This field specifies the minimum time (in microseconds) for which the MAC waits after it stops
	 * transmitting the LPI pattern to the PHY and before it
	 * resumes the normal transmission. The TLPIEX status
	 * bit is set after the expiry of this timer.
	 */
	value = ((tw & 0xffff)) | ((ls & 0x7ff) << 16);

	/* Program the timers in the LPI timer control register:
	 * LS: minimum time (ms) for which the link
	 *  status from PHY should be ok before transmitting
	 *  the LPI pattern.
	 * TW: minimum time (us) for which the core waits
	 *  after it has stopped transmitting the LPI pattern.
	 */
	writel(value, ioaddr + LPI_TIMER_CTRL);
}

/**
 * dwmac_ctrl_ane - To program the AN Control Register.
 * @ioaddr: IO registers pointer
 * @reg: Base address of the AN Control Register.
 * @ane: to enable the auto-negotiation
 * @srgmi_ral: to manage MAC-2-MAC SGMII connections.
 * @loopback: to cause the PHY to loopback tx data into rx path.
 * Description: this is the main function to configure the AN control register
 * and init the ANE, select loopback (usually for debugging purpose) and
 * configure SGMII RAL.
 */
static void dwmac_ctrl_ane( int iMacID, uint32_t reg, bool ane,
				  bool srgmi_ral, bool loopback)
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t value;

	value = readl(ioaddr + GMAC_AN_CTRL(reg));

	value &= ~( GMAC_AN_CTRL_ANE | GMAC_AN_CTRL_RAN | GMAC_AN_CTRL_SGMRAL | GMAC_AN_CTRL_ELE );

	/* Enable and restart the Auto-Negotiation */
	if (ane)
	{
		value |= GMAC_AN_CTRL_ANE | GMAC_AN_CTRL_RAN;
	}

	/* In case of MAC-2-MAC connection, block is configured to operate
	 * according to MAC conf register.
	 */
	if (srgmi_ral)
	{
		value |= GMAC_AN_CTRL_SGMRAL;
	}

	if (loopback)
	{
		value |= GMAC_AN_CTRL_ELE;
	}

	writel(value, ioaddr + GMAC_AN_CTRL(reg));
}

static void dwmac1000_ctrl_ane( int iMacID, bool ane, bool srgmi_ral, bool loopback )
{
	dwmac_ctrl_ane( iMacID, GMAC_PCS_BASE, ane, srgmi_ral, loopback );
}

/**
 * dwmac_rane - To restart ANE
 * @ioaddr: IO registers pointer
 * @reg: Base address of the AN Control Register.
 * @restart: to restart ANE
 * Description: this is to just restart the Auto-Negotiation.
 */
static inline void dwmac_rane( int iMacID, uint32_t reg, bool restart )
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t value;

	value = readl(ioaddr + GMAC_AN_CTRL(reg));

	if (restart)
		value |= GMAC_AN_CTRL_RAN;

	writel(value, ioaddr + GMAC_AN_CTRL(reg));
}

void dwmac1000_rane( int iMacID, bool restart )
{
	dwmac_rane( iMacID, GMAC_PCS_BASE, restart );
}

/* RGMII or SMII interface */
void dwmac1000_rgsmii( int iMacID, EMACStats_t *pxStats)
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t status;

	status = readl(ioaddr + GMAC_RGSMIIIS);
	pxStats->irq_rgmii_n++;

	/* Check the link status */
	if (status & GMAC_RGSMIIIS_LNKSTS) {
		int speed_value;

		pxStats->pcs_link = 1;

		speed_value = ((status & GMAC_RGSMIIIS_SPEED) >>
			       GMAC_RGSMIIIS_SPEED_SHIFT);
		if (speed_value == GMAC_RGSMIIIS_SPEED_125)
			pxStats->pcs_speed = SPEED_1000;
		else if (speed_value == GMAC_RGSMIIIS_SPEED_25)
			pxStats->pcs_speed = SPEED_100;
		else
			pxStats->pcs_speed = SPEED_10;

		pxStats->pcs_duplex = (status & GMAC_RGSMIIIS_LNKMOD_MASK);

		lUDPLoggingPrintf("Link is Up - %d/%s\n", (int)pxStats->pcs_speed,
			pxStats->pcs_duplex ? "Full" : "Half");
	} else {
		pxStats->pcs_link = 0;
		lUDPLoggingPrintf("Link is Down\n");
	}
}

/**
 * gmac_mdio_read
 * @bus: points to the mii_bus structure
 * @phyaddr: MII addr
 * @phyreg: MII reg
 * Description: it reads data from the MII register from within the phy device.
 * For the 7111 GMAC, we must set the bit 0 in the MII address register while
 * accessing the PHY registers.
 * Fortunately, it seems this has no drawback for the 7109 MAC.
 */
/*
	mac->mii.addr = GMAC_MII_ADDR;
	mac->mii.data = GMAC_MII_DATA;
	mac->mii.addr_shift = 11;
	mac->mii.addr_mask = 0x0000F800;
	mac->mii.reg_shift = 6;
	mac->mii.reg_mask = 0x000007C0;
	mac->mii.clk_csr_shift = 2;
	mac->mii.clk_csr_mask = GENMASK(5, 2);
*/
#define GMAC_MDIO_ADDR_SHIFT		11
#define GMAC_MDIO_ADDR_MASK			0x0000F800u

#define GMAC_MDIO_REG_SHIFT			6
#define GMAC_MDIO_REG_MASK			0x000007C0u
#define GMAC_MDIO_CLK_CSR_SHIFT		2
#define GMAC_MDIO_CLK_CSR_MASK		GENMASK(5, 2)

/* Define the macros for CSR clock range parameters to be passed by
 * platform code.
 * This could also be configured at run time using CPU freq framework. */

/* MDC Clock Selection define*/
#define	STMMAC_CSR_60_100M	0x0	/* MDC = clk_scr_i/42 */
#define	STMMAC_CSR_100_150M	0x1	/* MDC = clk_scr_i/62 */
#define	STMMAC_CSR_20_35M	0x2	/* MDC = clk_scr_i/16 */
#define	STMMAC_CSR_35_60M	0x3	/* MDC = clk_scr_i/26 */
#define	STMMAC_CSR_150_250M	0x4	/* MDC = clk_scr_i/102 */
#define	STMMAC_CSR_250_300M	0x5	/* MDC = clk_scr_i/122 */

#define GMAC_MDIO_CLK_CSR			STMMAC_CSR_250_300M

#define MII_BUSY		0x00000001
#define MII_WRITE		0x00000002

/* GMAC4 defines */
#define MII_GMAC4_GOC_SHIFT		2
#define MII_GMAC4_WRITE			(1 << MII_GMAC4_GOC_SHIFT)
#define MII_GMAC4_READ			(3 << MII_GMAC4_GOC_SHIFT)

static int waitNotBusy( uint8_t *pucAddress, uint32_t timeout_ms )
{
uint32_t value;
TickType_t xStart = xTaskGetTickCount(), xEnd;

	for( ;; )
	{
		value = readl( pucAddress );
		if( ( value & MII_BUSY ) == 0u )
		{
			break;
		}
		xEnd = xTaskGetTickCount();
		if( ( xEnd - xStart ) > timeout_ms )
		{
			/* Time-out reached. */
			return 0;
		}
	}
	return 1;
}

int gmac_mdio_read( int iMacID, int phyaddr, int phyreg)
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t mii_address = GMAC_MII_ADDR;
uint32_t mii_data = GMAC_MII_DATA;
int data;
uint32_t value = MII_BUSY;

	value |= ( phyaddr << GMAC_MDIO_ADDR_SHIFT ) & GMAC_MDIO_ADDR_MASK;

	value |= ( phyreg << GMAC_MDIO_REG_SHIFT ) & GMAC_MDIO_REG_MASK;
	value |= ( GMAC_MDIO_CLK_CSR << GMAC_MDIO_CLK_CSR_SHIFT ) & GMAC_MDIO_CLK_CSR_MASK;

	if( HAS_GMAC4 != 0 )
	{
		value |= MII_GMAC4_READ;
	}
	else
	{
		/* bit-1 remains zero when reading. */
	}

	if( !waitNotBusy( ioaddr + mii_address, 10 ) )
	{
		return -1;
	}

	writel(value, ioaddr + mii_address);

	if ( !waitNotBusy( ioaddr + mii_address, 10 ) )
	{
		return -1;
	}

	/* Read the data from the MII data register */
	data = (int)readl( ioaddr + mii_data );

	return data;
}

/**
 * gmac_mdio_write
 * @bus: points to the mii_bus structure
 * @phyaddr: MII addr
 * @phyreg: MII reg
 * @phydata: phy data
 * Description: it writes the data into the MII register from within the device.
 */
int gmac_mdio_write( int iMacID, int phyaddr, int phyreg, uint16_t phydata )
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t mii_address = GMAC_MII_ADDR;
uint32_t mii_data = GMAC_MII_DATA;
uint32_t value = MII_BUSY;

	value |= ( phyaddr << GMAC_MDIO_ADDR_SHIFT ) & GMAC_MDIO_ADDR_MASK;
	value |= ( phyreg << GMAC_MDIO_REG_SHIFT ) & GMAC_MDIO_REG_MASK;

	value |= ( GMAC_MDIO_CLK_CSR << GMAC_MDIO_CLK_CSR_SHIFT ) & GMAC_MDIO_CLK_CSR_MASK;

	if( HAS_GMAC4 != 0 )
	{
		value |= MII_GMAC4_WRITE;
	}
	else
	{
		value |= MII_WRITE;
	}

	/* Wait until any existing MII operation is complete */
	if( !waitNotBusy( ioaddr + mii_address, 10 ) )
	{
		return -1;
	}

	/* Set the MII address register to write */
	writel( phydata, ioaddr + mii_data );
	writel( value, ioaddr + mii_address );

	/* Wait until any existing MII operation is complete */
	if( !waitNotBusy( ioaddr + mii_address, 10 ) )
	{
		return -1;
	}
	return 0;
}

void gmac_set_emac_interrupt_enable( int iMacID, uint32_t ulMask )
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );

	writel( ulMask, ioaddr + GMAC_INT_MASK );
}

uint32_t gmac_get_emac_interrupt_status( int iMacID, int iClear )
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t ulValue;

	ulValue = readl( ioaddr + GMAC_INT_STATUS );
	if( iClear != 0 )
	{
		writel( ( uint32_t )~0ul, ioaddr + GMAC_INT_STATUS );
	}
	return ulValue;
}

volatile uint32_t phyIDs[ 8 ];
volatile uint32_t *IOaddr;
volatile uint16_t ulLowerID, ulUpperID;

#include "socal/alt_sysmgr.h"
#include "socal/hps.h"

ALT_SYSMGR_t *systemManager = ( ALT_SYSMGR_t * )ALT_SYSMGR_OFST;

void dwmac1000_sys_init( int iMacID )
{
EMACStats_t xStats;
EMACDeviceInfo_t hw;
int phyaddr, iIndex;

//	ulVersion = systemManager->pinmuxgrp.EMACIO0.sel;               /* ALT_SYSMGR_PINMUX_EMACIO0 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO1.sel;               /* ALT_SYSMGR_PINMUX_EMACIO1 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO2.sel;               /* ALT_SYSMGR_PINMUX_EMACIO2 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO3.sel;               /* ALT_SYSMGR_PINMUX_EMACIO3 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO4.sel;               /* ALT_SYSMGR_PINMUX_EMACIO4 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO5.sel;               /* ALT_SYSMGR_PINMUX_EMACIO5 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO6.sel;               /* ALT_SYSMGR_PINMUX_EMACIO6 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO7.sel;               /* ALT_SYSMGR_PINMUX_EMACIO7 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO8.sel;               /* ALT_SYSMGR_PINMUX_EMACIO8 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO9.sel;               /* ALT_SYSMGR_PINMUX_EMACIO9 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO10.sel;              /* ALT_SYSMGR_PINMUX_EMACIO10 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO11.sel;              /* ALT_SYSMGR_PINMUX_EMACIO11 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO12.sel;              /* ALT_SYSMGR_PINMUX_EMACIO12 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO13.sel;              /* ALT_SYSMGR_PINMUX_EMACIO13 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO14.sel;              /* ALT_SYSMGR_PINMUX_EMACIO14 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO15.sel;              /* ALT_SYSMGR_PINMUX_EMACIO15 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO16.sel;              /* ALT_SYSMGR_PINMUX_EMACIO16 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO17.sel;              /* ALT_SYSMGR_PINMUX_EMACIO17 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO18.sel;              /* ALT_SYSMGR_PINMUX_EMACIO18 */
//	ulVersion = systemManager->pinmuxgrp.EMACIO19.sel;              /* ALT_SYSMGR_PINMUX_EMACIO19 */

	IOaddr = ( volatile uint32_t * )ucFirstIOAddres( iMacID );
	memset( &hw, '\0', sizeof hw );

//	We are using EMAC-1 which is in a running state already.

	{
		/* The value 1 is either stored as 0x00, 0x01 (big) or 0x01, 0x00 (little endian). */
		uint16_t usWord = 1;
		/* Peek the contents with a byte poniter. */
		uint8_t *pucByte = ( uint8_t * )&usWord;
		#if( ipconfigBYTE_ORDER == pdFREERTOS_BIG_ENDIAN )
		/* The first byte is a 0x00. */
		uint8_t ucExpected = 0;
		#elif( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN )
		/* The first byte is a 0x01. */
		uint8_t ucExpected = 1;
		#else
			#error ipconfigBYTE_ORDER should be defined as either 
		#endif
		configASSERT( *( pucByte ) == ucExpected );
		FreeRTOS_printf( ( "Endianness is %s\n", *( pucByte ) == ucExpected ? "Ok" : "Wrong" ) );
	}

	{
		gmac_tx_descriptor_t desc;
		memset( &desc, '\0', sizeof desc );
		desc.own = 1;
		desc.buf1_address = 0x000000ff;
		desc.next_descriptor = 0x0000ff00;
	}

	{
		struct stmmac_axi xAXI;
		memset( &xAXI, '\0', sizeof xAXI );
		for( iIndex = 0; iIndex < ARRAY_SIZE( xAXI.axi_blen ); iIndex++ )
		{
			/* Get an array of 4, 8, 16, 32, 64, 128, and 256. */
			xAXI.axi_blen[ iIndex ] = 0x04u << iIndex;
		}
		/* AXI Maximum Write OutStanding Request Limit. */
		xAXI.axi_wr_osr_lmt = 1;
		/* This value limits the maximum outstanding request
		 * on the AXI read interface. Maximum outstanding
		 * requests = RD_OSR_LMT+1 */
		xAXI.axi_rd_osr_lmt = 1;
		/* When set to 1, this bit enables the LPI mode (Low Poer Idle) */
		xAXI.axi_lpi_en = 0;

		/*  1 KB Boundary Crossing Enable for the GMAC-AXI
		 * Master When set, the GMAC-AXI Master performs
		 * burst transfers that do not cross 1 KB boundary.
		 * When reset, the GMAC-AXI Master performs burst
		 * transfers that do not cross 4 KB boundary.*/
		xAXI.axi_kbbe = 0;
		/* When set to 1, this bit enables the GMAC-AXI to
		 * come out of the LPI mode only when the Remote
		 * Wake Up Packet is received. When set to 0, this bit
		 * enables the GMAC-AXI to come out of LPI mode
		 * when any frame is received. This bit must be set to 0. */
		xAXI.axi_xit_frm = 0;

		dwmac1000_dma_axi( iMacID, &xAXI );
	}
	{
		struct stmmac_dma_cfg dma_cfg;
		memset( &dma_cfg, '\0', sizeof dma_cfg );

		dma_cfg.txpbl = 8;
		dma_cfg.rxpbl = 1;
		dma_cfg.pblx8 = pdFALSE;
		dma_cfg.fixed_burst = pdFALSE;
		dma_cfg.mixed_burst = pdFALSE;
		/* When this bit is set high and the FB bit is equal to 1,
		the AHB or AXI interface generates all bursts aligned
		to the start address LS bits. If the FB bit is equal to 0,
		the first burst (accessing the data buffer's start
		address) is not aligned, but subsequent bursts are
		aligned to the address */
		dma_cfg.aal = 0;
		dwmac1000_dma_init( iMacID, &dma_cfg );
		dwmac1000_dma_operation_mode( iMacID, SF_DMA_MODE, SF_DMA_MODE );
	}
	ulUsePHYAddress = 0;
	for( phyaddr = 0; phyaddr < ARRAY_SIZE( phyIDs ); phyaddr++ )
	{
		ulUpperID = gmac_mdio_read( iMacID, phyaddr, 2 );
		ulLowerID = gmac_mdio_read( iMacID, phyaddr, 3 );
		phyIDs[ phyaddr ] = ( ( ( uint32_t ) ulUpperID ) << 16 ) | ( ulLowerID & 0xFFF0 );
		if( ( ulUpperID != 0xffffu ) && ( ulLowerID != 0xffffu ) )
		{
			ulUsePHYAddress = phyaddr;
			lUDPLoggingPrintf( "phy %d found\n", phyaddr );
		}
	}

	ulEMACVersion = dwmac1000_read_version( iMacID );

#define GMAC_EXPECTED_VERSION	0x1037ul
	if( ulEMACVersion != GMAC_EXPECTED_VERSION )
	{
		lUDPLoggingPrintf( "Wrong version : %04lX ( expected %04lX )\n", ulEMACVersion, GMAC_EXPECTED_VERSION );
		return;
	}

	/* Using the RGMII interface. */
	dwmac1000_rgsmii( iMacID, &xStats );

	hw.ps = SPEED_1000;
	dwmac1000_core_init( iMacID, &hw, 1500u );
	gmac_enable_transmission( iMacID, pdFALSE );
	dwmac1000_rx_ipc_enable( iMacID, pdTRUE );

	/* Write the main MAC address at position 0 */
	dwmac1000_set_umac_addr( iMacID, ucMACAddress, 0 );

	dwmac1000_reset_eee_mode( iMacID );
	vTaskDelay( 100 );
	/* Enable the link status receive on RGMII, SGMII or SMII
	 * receive path and instruct the transmit to enter in LPI
	 * state.
	 */
	dwmac1000_set_eee_mode( iMacID, pdFALSE, pdFALSE );

	/* Program the timers in the LPI timer control register:
	 * LS: minimum time (ms) for which the link
	 *  status from PHY should be ok before transmitting
	 *  the LPI pattern.
	 * TW: minimum time (us) for which the core waits
	 *  after it has stopped transmitting the LPI pattern.
	 */
	dwmac1000_set_eee_timer( iMacID, 1000, 0 );

	dwmac1000_set_eee_pls( iMacID, pdFALSE );
	dwmac1000_set_eee_pls_enable( iMacID, pdFALSE );

	dwmac1000_ctrl_ane( iMacID, pdTRUE, pdFALSE, pdFALSE );

	dwmac1000_rane( iMacID, pdTRUE );
	vTaskDelay( 3000 );

	dwmac1000_rgsmii( iMacID, &xStats );
}

uint32_t Phy_Setup( EMACInterface_t *pxEMACif )
{
	return 1000;
}

