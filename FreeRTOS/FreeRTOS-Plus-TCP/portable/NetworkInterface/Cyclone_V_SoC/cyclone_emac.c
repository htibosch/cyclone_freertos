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

#include "cyclone_emac.h"

#include "socal/alt_rstmgr.h"

#define RESET_MANAGER_ADDR		0xFFD05000

static ALT_RSTMGR_t *pxResetManager = ( ALT_RSTMGR_t * ) RESET_MANAGER_ADDR;

extern const uint8_t ucMACAddress[ 6 ];

static void pr_info( const char *pcFormat, ... )
{
}

static uint32_t readl( uint8_t *pucAddress )
{
	return *( ( volatile uint32_t *) pucAddress );
}

static void writel( uint32_t ulValue, uint8_t *pucAddress )
{
	*( ( volatile uint32_t *) pucAddress ) = ulValue;
}

static uint8_t *ucFirstIOAddres(int iMacID)
{
uint8_t *ucReturn;

	if( iMacID == 0 )
	{
		ucReturn = ( uint8_t * )EMAC_ID_0_ADDRESS;
	}
	else if( iMacID == 1 )
	{
		ucReturn = ( uint8_t * )EMAC_ID_1_ADDRESS;
	}
	else
	{
		ucReturn = ( uint8_t * )NULL;
	}
	return ucReturn;
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
		} else {
			value |= GMAC_CONTROL_PS;

			if (hw->ps == SPEED_10)
				value &= ~GMAC_CONTROL_FES;
			else
				value |= GMAC_CONTROL_FES;
		}
	}

	writel(value, ioaddr + GMAC_CONTROL);

	/* Mask GMAC interrupts */
	value = GMAC_INT_DEFAULT_MASK;

	if (hw->pmt)
		value &= ~GMAC_INT_DISABLE_PMT;
	if (hw->pcs)
		value &= ~GMAC_INT_DISABLE_PCS;

	writel(value, ioaddr + GMAC_INT_MASK);

#ifdef STMMAC_VLAN_TAG_USED
	/* Tag detection without filtering */
	writel(0x0, ioaddr + GMAC_VLAN_TAG);
#endif
}

/* Enable disable MAC RX/TX */
void stmmac_set_mac(int iMacID, bool enable)
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t value;

	value = readl(ioaddr + MAC_CTRL_REG);

	if (enable)
		value |= MAC_ENABLE_RX | MAC_ENABLE_TX;
	else
		value &= ~(MAC_ENABLE_TX | MAC_ENABLE_RX);

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

static void dwmac1000_set_eee_mode( int iMacID,
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
	value |= LPI_CTRL_STATUS_LPIEN | LPI_CTRL_STATUS_LPITXA;
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

	/* Enable and restart the Auto-Negotiation */
	if (ane)
		value |= GMAC_AN_CTRL_ANE | GMAC_AN_CTRL_RAN;

	/* In case of MAC-2-MAC connection, block is configured to operate
	 * according to MAC conf register.
	 */
	if (srgmi_ral)
		value |= GMAC_AN_CTRL_SGMRAL;

	if (loopback)
		value |= GMAC_AN_CTRL_ELE;

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

		pr_info("Link is Up - %d/%s\n", (int)pxStats->pcs_speed,
			pxStats->pcs_duplex ? "Full" : "Half");
	} else {
		pxStats->pcs_link = 0;
		pr_info("Link is Down\n");
	}
}

/**
 * stmmac_mdio_read
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

#define GMAC_MDIO_CLK_CSR			STMMAC_CSR_60_100M

#define MII_BUSY		0x00000001
#define MII_WRITE		0x00000002

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

static int stmmac_mdio_read( int iMacID, int phyaddr, int phyreg)
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t mii_address = GMAC_MII_ADDR;
uint32_t mii_data = GMAC_MII_DATA;
int data;
uint32_t value = MII_BUSY;

	value |= ( phyaddr << GMAC_MDIO_ADDR_SHIFT ) & GMAC_MDIO_ADDR_MASK;

	value |= ( phyreg << GMAC_MDIO_REG_SHIFT ) & GMAC_MDIO_REG_MASK;
	value |= ( GMAC_MDIO_CLK_CSR << GMAC_MDIO_CLK_CSR_SHIFT ) & GMAC_MDIO_CLK_CSR_MASK;
	#if( HAS_GMAC4 != 0 )
	{
		value |= MII_GMAC4_READ;
	}
	#else
	{
		/* bit-1 remains zero when reading. */
	}
	#endif
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
 * stmmac_mdio_write
 * @bus: points to the mii_bus structure
 * @phyaddr: MII addr
 * @phyreg: MII reg
 * @phydata: phy data
 * Description: it writes the data into the MII register from within the device.
 */
static int stmmac_mdio_write( int iMacID, int phyaddr, int phyreg, uint16_t phydata )
{
uint8_t *ioaddr = ucFirstIOAddres( iMacID );
uint32_t mii_address = GMAC_MII_ADDR;
uint32_t mii_data = GMAC_MII_DATA;
uint32_t value = MII_BUSY;

	value |= ( phyaddr << GMAC_MDIO_ADDR_SHIFT ) & GMAC_MDIO_ADDR_MASK;
	value |= ( phyreg << GMAC_MDIO_REG_SHIFT ) & GMAC_MDIO_REG_MASK;

	value |= ( GMAC_MDIO_CLK_CSR << GMAC_MDIO_CLK_CSR_SHIFT ) & GMAC_MDIO_CLK_CSR_MASK;

	#if( HAS_GMAC4 != 0 )
	{
		value |= MII_GMAC4_WRITE;
	}
	#else
	{
		value |= MII_WRITE;
	}
	#endif

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

volatile uint32_t phyIDs[ 10 ];
volatile uint32_t *IOaddr;

void dwmac1000_sys_init()
{
EMACStats_t xStats;
EMACDeviceInfo_t hw;
const int iMacID = 0;
int phyaddr;

	IOaddr = ( volatile uint32_t * )ucFirstIOAddres( iMacID );
	memset( &hw, '\0', sizeof hw );

	dwmac1000_rgsmii( iMacID, &xStats );

	/* Get EMAC-0 out-of the reset state. */
	pxResetManager->permodrst.emac0 = 0;
	hw.ps = SPEED_1000;
	dwmac1000_core_init( iMacID, &hw, 1500u );
	stmmac_set_mac( iMacID, 1 );
	dwmac1000_rx_ipc_enable( iMacID, true );

	/* Write the main MAC address at position 0 */
	dwmac1000_set_umac_addr( iMacID, ucMACAddress, 0 );

	dwmac1000_reset_eee_mode( iMacID );
	vTaskDelay( 100 );
	/* Enable the link status receive on RGMII, SGMII ore SMII
	 * receive path and instruct the transmit to enter in LPI
	 * state.
	 */
	dwmac1000_set_eee_mode( iMacID, pdTRUE );

	/* Program the timers in the LPI timer control register:
	 * LS: minimum time (ms) for which the link
	 *  status from PHY should be ok before transmitting
	 *  the LPI pattern.
	 * TW: minimum time (us) for which the core waits
	 *  after it has stopped transmitting the LPI pattern.
	 */
	dwmac1000_set_eee_timer( iMacID, 1000, 1000 );

	dwmac1000_set_eee_pls_enable( iMacID, pdFALSE );
	dwmac1000_ctrl_ane( iMacID, pdTRUE, pdFALSE, pdFALSE );

	dwmac1000_rane( iMacID, pdTRUE );
	vTaskDelay( 4000 );

	for( phyaddr = 0; phyaddr < 10; phyaddr++ )
	{
		phyIDs[ phyaddr ] = stmmac_mdio_read( iMacID, phyaddr, 2 );
	}
	dwmac1000_rgsmii( iMacID, &xStats );
}
