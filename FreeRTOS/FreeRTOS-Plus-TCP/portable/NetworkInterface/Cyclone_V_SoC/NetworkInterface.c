/*
 * FreeRTOS+TCP Labs Build 200417 (C) 2016 Real Time Engineers ltd.
 * Authors include Hein Tibosch and Richard Barry
 *
 *******************************************************************************
 ***** NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ***
 ***                                                                         ***
 ***                                                                         ***
 ***   FREERTOS+TCP IS STILL IN THE LAB (mainly because the FTP and HTTP     ***
 ***   demos have a dependency on FreeRTOS+FAT, which is only in the Labs    ***
 ***   download):                                                            ***
 ***                                                                         ***
 ***   FreeRTOS+TCP is functional and has been used in commercial products   ***
 ***   for some time.  Be aware however that we are still refining its       ***
 ***   design, the source code does not yet quite conform to the strict      ***
 ***   coding and style standards mandated by Real Time Engineers ltd., and  ***
 ***   the documentation and testing is not necessarily complete.            ***
 ***                                                                         ***
 ***   PLEASE REPORT EXPERIENCES USING THE SUPPORT RESOURCES FOUND ON THE    ***
 ***   URL: http://www.FreeRTOS.org/contact  Active early adopters may, at   ***
 ***   the sole discretion of Real Time Engineers Ltd., be offered versions  ***
 ***   under a license other than that described below.                      ***
 ***                                                                         ***
 ***                                                                         ***
 ***** NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ***
 *******************************************************************************
 *
 * FreeRTOS+TCP can be used under two different free open source licenses.  The
 * license that applies is dependent on the processor on which FreeRTOS+TCP is
 * executed, as follows:
 *
 * If FreeRTOS+TCP is executed on one of the processors listed under the Special
 * License Arrangements heading of the FreeRTOS+TCP license information web
 * page, then it can be used under the terms of the FreeRTOS Open Source
 * License.  If FreeRTOS+TCP is used on any other processor, then it can be used
 * under the terms of the GNU General Public License V2.  Links to the relevant
 * licenses follow:
 *
 * The FreeRTOS+TCP License Information Page: http://www.FreeRTOS.org/tcp_license
 * The FreeRTOS Open Source License: http://www.FreeRTOS.org/license
 * The GNU General Public License Version 2: http://www.FreeRTOS.org/gpl-2.0.txt
 *
 * FreeRTOS+TCP is distributed in the hope that it will be useful.  You cannot
 * use FreeRTOS+TCP unless you agree that you use the software 'as is'.
 * FreeRTOS+TCP is provided WITHOUT ANY WARRANTY; without even the implied
 * warranties of NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. Real Time Engineers Ltd. disclaims all conditions and terms, be they
 * implied, expressed, or statutory.
 *
 * 1 tab == 4 spaces!
 *
 * http://www.FreeRTOS.org
 * http://www.FreeRTOS.org/plus
 * http://www.FreeRTOS.org/labs
 *
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
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

/* Provided memory configured as uncached. */
#include "uncached_memory.h"

#define niBMSR_LINK_STATUS         0x0004ul

#ifndef	PHY_LS_HIGH_CHECK_TIME_MS
	/* Check if the LinkSStatus in the PHY is still high after 15 seconds of not
	receiving packets. */
	#define PHY_LS_HIGH_CHECK_TIME_MS	15000
#endif

#ifndef	PHY_LS_LOW_CHECK_TIME_MS
	/* Check if the LinkSStatus in the PHY is still low every second. */
	#define PHY_LS_LOW_CHECK_TIME_MS	1000
#endif

/* The size of each buffer when BufferAllocation_1 is used:
http://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/Embedded_Ethernet_Buffer_Management.html */
#define niBUFFER_1_PACKET_SIZE		1536

/* Naming and numbering of PHY registers. */
#define PHY_REG_01_BMSR			0x01	/* Basic mode status register */

#ifndef iptraceEMAC_TASK_STARTING
	#define iptraceEMAC_TASK_STARTING()	do { } while( 0 )
#endif

/* Default the size of the stack used by the EMAC deferred handler task to twice
the size of the stack used by the idle task - but allow this to be overridden in
FreeRTOSConfig.h as configMINIMAL_STACK_SIZE is a user definable constant. */
#ifndef configEMAC_TASK_STACK_SIZE
	#define configEMAC_TASK_STACK_SIZE ( 2 * configMINIMAL_STACK_SIZE )
#endif

#define EMAC_IF_RX_EVENT	1
#define EMAC_IF_TX_EVENT	2
#define EMAC_IF_ERR_EVENT	4
#define EMAC_IF_ALL_EVENT	7

/*-----------------------------------------------------------*/

/*
 * Look for the link to be up every few milliseconds until either xMaxTime time
 * has passed or a link is found.
 */
static BaseType_t prvGMACWaitLS( TickType_t xMaxTime );

/*
 * A deferred interrupt handler for all MAC/DMA interrupt sources.
 */
static void prvEMACHandlerTask( void *pvParameters );

/*-----------------------------------------------------------*/

extern int phy_detected;

/* A copy of PHY register 1: 'PHY_REG_01_BMSR' */
static uint32_t ulPHYLinkStatus = 0;

#if( ipconfigUSE_LLMNR == 1 )
	//static const uint8_t xLLMNR_MACAddress[] = { 0x01, 0x00, 0x5E, 0x00, 0x00, 0xFC };
#endif

/* ucMACAddress as it appears in main.c */
extern const uint8_t ucMACAddress[ 6 ];

/* Holds the handle of the task used as a deferred interrupt processor.  The
handle is used so direct notifications can be sent to the task for all EMAC/DMA
related interrupts. */
TaskHandle_t xEMACTaskHandle = NULL;

static volatile UBaseType_t uxISREvents;

EMACInterface_t xEMACif;

static int iMacID = 1;

__attribute__ ( ( aligned( 32 ) ) ) gmac_tx_descriptor_t txDescriptors[ GMAC_TX_BUFFERS ];
__attribute__ ( ( aligned( 32 ) ) ) gmac_rx_descriptor_t rxDescriptors[ GMAC_RX_BUFFERS ];

/*-----------------------------------------------------------*/

BaseType_t xNetworkInterfaceInitialise( void )
{
const TickType_t xWaitLinkDelay = pdMS_TO_TICKS( 7000UL ), xWaitRelinkDelay = pdMS_TO_TICKS( 1000UL );
BaseType_t xLinkStatus;

	/* Guard against the init function being called more than once. */
	if( xEMACTaskHandle == NULL )
	{
		dwmac1000_sys_init( iMacID );

		gmac_tx_descriptor_init( iMacID, txDescriptors, ( uint8_t *) NULL, GMAC_TX_BUFFERS );
		gmac_rx_descriptor_init( iMacID, rxDescriptors, ( uint8_t *) NULL, GMAC_RX_BUFFERS );

		prvGMACWaitLS( xWaitLinkDelay );

		/* The deferred interrupt handler task is created at the highest
		possible priority to ensure the interrupt handler can return directly
		to it.  The task's handle is stored in xEMACTaskHandle so interrupts can
		notify the task when there is something to process. */
		xTaskCreate( prvEMACHandlerTask, "EMAC", configEMAC_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &xEMACTaskHandle );
	}
	else
	{
		/* Initialisation was already performed, just wait for the link. */
		prvGMACWaitLS( xWaitRelinkDelay );
	}

	/* Only return pdTRUE when the Link Status of the PHY is high, otherwise the
	DHCP process and all other communication will fail. */
	xLinkStatus = xGetPhyLinkStatus();

	return ( xLinkStatus != pdFALSE );
}
/*-----------------------------------------------------------*/

BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxBuffer, BaseType_t bReleaseAfterSend )
{
	if( ( ulPHYLinkStatus & niBMSR_LINK_STATUS ) != 0 )
	{
		iptraceNETWORK_INTERFACE_TRANSMIT();
	}
	else if( bReleaseAfterSend != pdFALSE )
	{
		/* No link. */
		vReleaseNetworkBufferAndDescriptor( pxBuffer );
	}

	return pdTRUE;
}
/*-----------------------------------------------------------*/

static inline uint32_t ulReadMDIO( uint32_t ulRegister )
{
uint32_t ulValue = gmac_mdio_read( iMacID, ulUsePHYAddress, ulRegister);

	return ulValue;
}
/*-----------------------------------------------------------*/

static inline void ulWriteMDIO( uint32_t ulRegister, uint32_t ulValue )
{
	gmac_mdio_write( iMacID, ulUsePHYAddress, ulRegister, ( uint16_t ) ulValue );
}
/*-----------------------------------------------------------*/

static BaseType_t prvGMACWaitLS( TickType_t xMaxTime )
{
TickType_t xStartTime, xEndTime;
const TickType_t xShortDelay = pdMS_TO_TICKS( 20UL );
BaseType_t xReturn;

	xStartTime = xTaskGetTickCount();

	for( ;; )
	{
		xEndTime = xTaskGetTickCount();

		if( xEndTime - xStartTime > xMaxTime )
		{
			xReturn = pdFALSE;
			break;
		}
		ulPHYLinkStatus = ulReadMDIO( PHY_REG_01_BMSR );

		if( ( ulPHYLinkStatus & niBMSR_LINK_STATUS ) != 0 )
		{
			xReturn = pdTRUE;
			break;
		}

		vTaskDelay( xShortDelay );
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

void vNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{
static uint8_t ucNetworkPackets[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS * niBUFFER_1_PACKET_SIZE ] __attribute__ ( ( aligned( 32 ) ) );
uint8_t *ucRAMBuffer = ucNetworkPackets;
uint32_t ul;

	for( ul = 0; ul < ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS; ul++ )
	{
		pxNetworkBuffers[ ul ].pucEthernetBuffer = ucRAMBuffer + ipBUFFER_PADDING;
		*( ( unsigned * ) ucRAMBuffer ) = ( unsigned ) ( &( pxNetworkBuffers[ ul ] ) );
		ucRAMBuffer += niBUFFER_1_PACKET_SIZE;
	}
}
/*-----------------------------------------------------------*/

BaseType_t xGetPhyLinkStatus( void )
{
BaseType_t xReturn;

	if( ( ulPHYLinkStatus & niBMSR_LINK_STATUS ) == 0 )
	{
		xReturn = pdFALSE;
	}
	else
	{
		xReturn = pdTRUE;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

static void prvEMACHandlerTask( void *pvParameters )
{
UBaseType_t uxCurrentCount;
const TickType_t ulMaxBlockTime = pdMS_TO_TICKS( 100UL );
UBaseType_t uxLastMinBufferCount = 0;
UBaseType_t uxCurrentBufferCount = 0;
uint32_t ulStatus;

	/* Remove compiler warnings about unused parameters. */
	( void ) pvParameters;

	/* A possibility to set some additional task properties like calling
	portTASK_USES_FLOATING_POINT() */
	iptraceEMAC_TASK_STARTING();

	for( ;; )
	{
		uxCurrentBufferCount = uxGetMinimumFreeNetworkBuffers();
		if( uxLastMinBufferCount != uxCurrentBufferCount )
		{
			/* The logging produced below may be helpful
			while tuning +TCP: see how many buffers are in use. */
			uxLastMinBufferCount = uxCurrentBufferCount;
			FreeRTOS_printf( ( "Network buffers: %lu lowest %lu\n",
				uxGetNumberOfFreeNetworkBuffers(), uxCurrentBufferCount ) );
		}

		#if( ipconfigCHECK_IP_QUEUE_SPACE != 0 )
		{
		static UBaseType_t uxLastMinQueueSpace = 0;

			uxCurrentCount = uxGetMinimumIPQueueSpace();
			if( uxLastMinQueueSpace != uxCurrentCount )
			{
				/* The logging produced below may be helpful
				while tuning +TCP: see how many buffers are in use. */
				uxLastMinQueueSpace = uxCurrentCount;
				FreeRTOS_printf( ( "Queue space: lowest %lu\n", uxCurrentCount ) );
			}
		}
		#endif /* ipconfigCHECK_IP_QUEUE_SPACE */

		if( ( uxISREvents & EMAC_IF_ALL_EVENT ) == 0 )
		{
			/* No events to process now, wait for the next. */
			ulTaskNotifyTake( pdFALSE, ulMaxBlockTime );
		}

		if( ( uxISREvents & EMAC_IF_RX_EVENT ) != 0 )
		{
			uxISREvents &= ~EMAC_IF_RX_EVENT;
			gmac_check_rx( &xEMACif );
		}

		if( ( uxISREvents & EMAC_IF_TX_EVENT ) != 0 )
		{
			uxISREvents &= ~EMAC_IF_TX_EVENT;
			gmac_check_tx( &xEMACif );
		}

		if( ( uxISREvents & EMAC_IF_ERR_EVENT ) != 0 )
		{
			uxISREvents &= ~EMAC_IF_ERR_EVENT;
			gmac_check_errors( &xEMACif );
		}

		ulStatus = ulReadMDIO( PHY_REG_01_BMSR );

		if( ulPHYLinkStatus != ulStatus )
		{
		volatile BaseType_t xStartNego = 0;

			/* Test if the Link Status has become high. */
			if( ( ( ulPHYLinkStatus ^ ulStatus ) & niBMSR_LINK_STATUS ) && ( ( ulStatus & niBMSR_LINK_STATUS ) != 0 ) )
			{
				xStartNego = pdTRUE;
			}

			if( ulPHYLinkStatus != ulStatus )
			{
				ulPHYLinkStatus = ulStatus;
				FreeRTOS_printf( ( "prvEMACHandlerTask: PHY LS now %d Start nego %d\n", ( ulPHYLinkStatus & niBMSR_LINK_STATUS ) != 0, xStartNego ) );
				if( xStartNego )
				{
				uint32_t ulLinkSpeed;
					ulLinkSpeed = Phy_Setup( &xEMACif );
					if( ulLinkSpeed )
					{
						//
					}

					/* Setting the operating speed of the MAC needs a delay. */
					vTaskDelay( pdMS_TO_TICKS( 25UL ) );
				}
			}
		}
	}
}
/*-----------------------------------------------------------*/
