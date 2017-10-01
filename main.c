/*
	FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
	All rights reserved

	VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

	This file is part of the FreeRTOS distribution.

	FreeRTOS is free software; you can redistribute it and/or modify it under
	the terms of the GNU General Public License (version 2) as published by the
	Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

	***************************************************************************
	>>!   NOTE: The modification to the GPL is included to allow you to     !<<
	>>!   distribute a combined work that includes FreeRTOS without being   !<<
	>>!   obliged to provide the source code for proprietary components     !<<
	>>!   outside of the FreeRTOS kernel.                                   !<<
	***************************************************************************

	FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE.  Full license text is available on the following
	link: http://www.freertos.org/a00114.html

	***************************************************************************
	 *                                                                       *
	 *    FreeRTOS provides completely free yet professionally developed,    *
	 *    robust, strictly quality controlled, supported, and cross          *
	 *    platform software that is more than just the market leader, it     *
	 *    is the industry's de facto standard.                               *
	 *                                                                       *
	 *    Help yourself get started quickly while simultaneously helping     *
	 *    to support the FreeRTOS project by purchasing a FreeRTOS           *
	 *    tutorial book, reference manual, or both:                          *
	 *    http://www.FreeRTOS.org/Documentation                              *
	 *                                                                       *
	***************************************************************************

	http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
	the FAQ page "My application does not run, what could be wrong?".  Have you
	defined configASSERT()?

	http://www.FreeRTOS.org/support - In return for receiving this top quality
	embedded software for free we request you assist our global community by
	participating in the support forum.

	http://www.FreeRTOS.org/training - Investing in training allows your team to
	be as productive as possible as early as possible.  Now you can receive
	FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
	Ltd, and the world's leading authority on the world's leading RTOS.

	http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
	including FreeRTOS+Trace - an indispensable productivity tool, a DOS
	compatible FAT file system, and our tiny thread aware UDP/IP stack.

	http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
	Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

	http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
	Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
	licenses offer ticketed support, indemnification and commercial middleware.

	http://www.SafeRTOS.com - High Integrity Systems also provide a safety
	engineered and independently SIL3 certified version for use in safety and
	mission critical applications that require provable dependability.

	1 tab == 4 spaces!
*/


/* Standard includes. */
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <strings.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Standard demo includes. */
#include "partest.h"
#include "TimerDemo.h"
#include "QueueOverwrite.h"
#include "EventGroupsDemo.h"
#include "IntSemTest.h"

/* Altera library includes. */
#include "alt_timers.h"
#include "alt_clock_manager.h"
#include "alt_interrupt.h"
#include "alt_globaltmr.h"
#include "alt_address_space.h"
#include "mmu_support.h"
#include "cache_support.h"

#include "serial.h"

#include "cyclone_dma.h"
#include "cyclone_emac.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"
#include "FreeRTOS_tcp_server.h"
#include "NetworkInterface.h"

/* Demo application includes. */
#include "hr_gettime.h"

#include "UDPLoggingPrintf.h"

/* mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is used to select between two demo
 * applications, as described at the top of this file.
 *
 * When mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is set to 1 the simple blinky example
 * will be run.
 *
 * When mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is set to 0 the comprehensive test
 * and demo application will be run.
 */
#define mainCREATE_SIMPLE_BLINKY_DEMO_ONLY	1

#define mainCOMMAND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define mainCOMMAND_TASK_STACK_SIZE		( 1024 )

/* Define names that will be used for DNS, LLMNR and NBNS searches. */
#define mainHOST_NAME					"sam4e"
#define mainDEVICE_NICK_NAME			"sam4expro"

/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

/*
 * See the comments at the top of this file and above the
 * mainSELECTED_APPLICATION definition.
 */
#if ( mainCREATE_SIMPLE_BLINKY_DEMO_ONLY == 1 )
	extern void main_blinky( void );
#else
	extern void main_full( void );
#endif /* #if mainCREATE_SIMPLE_BLINKY_DEMO_ONLY == 1 */

/* Prototypes for the standard FreeRTOS callback/hook functions implemented
within this file. */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vApplicationTickHook( void );

/*-----------------------------------------------------------*/

static void prvCommandTask( void *pvParameters );

/* configAPPLICATION_ALLOCATED_HEAP is set to 1 in FreeRTOSConfig.h so the
application can define the array used as the FreeRTOS heap.  This is done so the
heap can be forced into fast internal RAM - useful because the stacks used by
the tasks come from this space. */
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ] __attribute__ ( ( section( ".oc_ram" ) ) );

/* FreeRTOS uses its own interrupt handler code.  This code cannot use the array
of handlers defined by the Altera drivers because the array is declared static,
and so not accessible outside of the dirver's source file.  Instead declare an
array for use by the FreeRTOS handler.  See:
http://www.freertos.org/Using-FreeRTOS-on-Cortex-A-Embedded-Processors.html. */
static INT_DISPATCH_t xISRHandlers[ ALT_INT_PROVISION_INT_COUNT ];

/* The default IP and MAC address used by the demo.  The address configuration
defined here will be used if ipconfigUSE_DHCP is 0, or if ipconfigUSE_DHCP is
1 but a DHCP server could not be contacted.  See the online documentation for
more information. */
static const uint8_t ucIPAddress[ 4 ] = { configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3 };
static const uint8_t ucNetMask[ 4 ] = { configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3 };
static const uint8_t ucGatewayAddress[ 4 ] = { configGATEWAY_ADDR0, configGATEWAY_ADDR1, configGATEWAY_ADDR2, configGATEWAY_ADDR3 };
static const uint8_t ucDNSServerAddress[ 4 ] = { configDNS_SERVER_ADDR0, configDNS_SERVER_ADDR1, configDNS_SERVER_ADDR2, configDNS_SERVER_ADDR3 };

/* Default MAC address configuration.  The demo creates a virtual network
connection that uses this MAC address by accessing the raw Ethernet data
to and from a real network connection on the host PC.  See the
configNETWORK_INTERFACE_TO_USE definition for information on how to configure
the real network connection to use. */

/* 00:11:22:33:44:49 */
const uint8_t ucMACAddress[ 6 ] = { configMAC_ADDR0, configMAC_ADDR1, configMAC_ADDR2, configMAC_ADDR3, configMAC_ADDR4, configMAC_ADDR5 };

/* Use by the pseudo random number generator. */
static UBaseType_t ulNextRand;

static SemaphoreHandle_t xServerSemaphore;

int verboseLevel;

/* Handle of the task that runs the FTP and HTTP servers. */
static TaskHandle_t xCommandTaskHandle = NULL;

#define RECEIVE_TASK_STACK_SIZE		640
#define RECEIVE_TASK_PRIORITY		2

/*-----------------------------------------------------------*/

int main( void )
{
	/* Configure the hardware ready to run the demo. */
	prvSetupHardware();

	xSerialPortInitMinimal( 115200, 128 );

	/* Initialise the network interface.

	***NOTE*** Tasks that use the network are created in the network event hook
	when the network is connected and ready for use (see the definition of
	vApplicationIPNetworkEventHook() below).  The address values passed in here
	are used if ipconfigUSE_DHCP is set to 0, or if ipconfigUSE_DHCP is set to 1
	but a DHCP server cannot be	contacted. */

	FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );

	xTaskCreate( prvCommandTask, "Command", mainCOMMAND_TASK_STACK_SIZE, NULL, mainCOMMAND_TASK_PRIORITY, &xCommandTaskHandle );

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was either insufficient FreeRTOS heap memory available for the idle
	and/or timer tasks to be created, or vTaskStartScheduler() was called from
	User mode.  See the memory management section on the FreeRTOS web site for
	more details on the FreeRTOS heap http://www.freertos.org/a00111.html.  The
	mode from which main() is called is set in the C start up code and must be
	a privileged mode (not user mode). */
	for( ;; );

	/* Don't expect to reach here. */
	return 0;
}
/*-----------------------------------------------------------*/

	static void prvCommandTask( void *pvParameters )
	{
		xSocket_t xSocket;
		BaseType_t xHadSocket = pdFALSE;
		BaseType_t xLastCount = 0;
		char cLastBuffer[ 16 ];
		char cBuffer[ 128 ];

		xServerSemaphore = xSemaphoreCreateBinary();
		configASSERT( xServerSemaphore != NULL );

		/* Wait until the network is up before creating the servers.  The
		notification is given from the network event hook. */
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

		/* The priority of this task can be raised now the disk has been
		initialised. */
		vTaskPrioritySet( NULL, mainCOMMAND_TASK_PRIORITY );

//		xTelnetCreate( &xTelnet, 23 );

		for( ;; )
		{
		TickType_t xReceiveTimeOut = pdMS_TO_TICKS( 200 );

			xSemaphoreTake( xServerSemaphore, xReceiveTimeOut );

			xSocket = xLoggingGetSocket();

			if( xSocket == NULL)
			{
				vTaskDelay( 100 );
			}
			else
			{
			BaseType_t xCount;
			struct freertos_sockaddr xSourceAddress;
			socklen_t xSourceAddressLength = sizeof( xSourceAddress );

				if( xHadSocket == pdFALSE )
				{
					xHadSocket = pdTRUE;
					FreeRTOS_printf( ( "prvCommandTask started\n" ) );
					/* xServerSemaphore will be given to when there is data for xSocket
					and also as soon as there is USB/CDC data. */
					FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_SET_SEMAPHORE, ( void * ) &xServerSemaphore, sizeof( xServerSemaphore ) );
					//FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );
				}
				xCount = 0;// xTelnetRecv( &xTelnet, &xSourceAddress, cBuffer, sizeof( cBuffer ) - 1 );
				if( xCount == 0 )
				{
					xCount = FreeRTOS_recvfrom( xSocket, ( void * )cBuffer, sizeof( cBuffer )-1, FREERTOS_MSG_DONTWAIT,
						&xSourceAddress, &xSourceAddressLength );
				}

				if( ( xCount >= 3 ) && ( xCount <= 5 ) && ( cBuffer[ 0 ] == 0x1b ) && ( cBuffer[ 1 ] == 0x5b ) && ( cBuffer[ 2 ] == 0x41 ) )
				{
					xCount = xLastCount;
					memcpy( cBuffer, cLastBuffer, sizeof cLastBuffer );
				}
				else if( xCount > 0 )
				{
					xLastCount = xCount;
					memcpy( cLastBuffer, cBuffer, sizeof cLastBuffer );
				}

				if( ( xCount > 0 ) && ( cBuffer[ 0 ] >= 32 ) && ( cBuffer[ 0 ] < 0x7F ) )
				{
					cBuffer[ xCount ] = '\0';
					FreeRTOS_printf( ( ">> %s\n", cBuffer ) );
					if( strncmp( cBuffer, "ver", 3 ) == 0 )
					{
					int level;

						if( sscanf( cBuffer + 3, "%d", &level ) == 1 )
						{
							verboseLevel = level;
						}
						lUDPLoggingPrintf( "Verbose level %d\n", verboseLevel );
					}

					#if( USE_IPERF != 0 )
		
					{
						if( strncmp( cBuffer, "iperf", 5 ) == 0 )
						{
							vIPerfInstall();
						}
					}
					#endif

					if( strncmp( cBuffer, "netstat", 7 ) == 0 )
					{
						FreeRTOS_netstat();
					}
					#if( USE_LOG_EVENT != 0 )
					{
						if( strncmp( cBuffer, "event", 4 ) == 0 )
						{
							eventLogDump();
						}
					}
					#endif /* USE_LOG_EVENT */
				}	/* if( xCount > 0 ) */
			}	/* if( xSocket != NULL) */
		}
	}

static void prvSetupHardware( void )
{
extern uint8_t __cs3_interrupt_vector;
uint32_t ulSCTLR, ulVectorTable = ( uint32_t ) &__cs3_interrupt_vector;
const uint32_t ulVBit = 13U;

	alt_int_global_init();
	alt_int_cpu_binary_point_set( 0 );

	/* Clear SCTLR.V for low vectors and map the vector table to the beginning
	of the code. */
	__asm( "MRC p15, 0, %0, c1, c0, 0" : "=r" ( ulSCTLR ) );
	ulSCTLR &= ~( 1 << ulVBit );
	__asm( "MCR p15, 0, %0, c1, c0, 0" : : "r" ( ulSCTLR ) );
	__asm( "MCR p15, 0, %0, c12, c0, 0" : : "r" ( ulVectorTable ) );

	cache_init();
	mmu_init();

	/* GPIO for LEDs.  ParTest is a historic name which used to stand for
	parallel port test. */
	vParTestInitialise();
}

/*-----------------------------------------------------------*/

/* Called by FreeRTOS+TCP when the network connects or disconnects.  Disconnect
events are only received if implemented in the MAC driver. */

/* Called by FreeRTOS+TCP when the network connects or disconnects.  Disconnect
events are only received if implemented in the MAC driver. */
void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
static BaseType_t xTasksAlreadyCreated = pdFALSE;

	/* If the network has just come up...*/
	if( eNetworkEvent == eNetworkUp )
	{
		/* Create the tasks that use the IP stack if they have not already been
		created. */
		if( xTasksAlreadyCreated == pdFALSE )
		{
			/* Tasks that use the TCP/IP stack can be created here. */

			#if( mainCREATE_TCP_ECHO_TASKS_SINGLE == 1 )
			{
				/* See http://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_Echo_Clients.html */
				vStartTCPEchoClientTasks_SingleTasks( configMINIMAL_STACK_SIZE * 4, tskIDLE_PRIORITY + 1 );
			}
			#endif

			#if( mainCREATE_SIMPLE_TCP_ECHO_SERVER == 1 )
			{
				/* See http://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_Echo_Server.html */
				vStartSimpleTCPServerTasks( configMINIMAL_STACK_SIZE * 4, tskIDLE_PRIORITY + 1 );
			}
			#endif


			#if( ( mainCREATE_FTP_SERVER == 1 ) || ( mainCREATE_HTTP_SERVER == 1 ) )
			{
				/* See TBD.
				Let the server work task now it can now create the servers. */
				xTaskNotifyGive( xServerWorkTaskHandle );
			}
			#endif
			xTaskNotifyGive( xCommandTaskHandle );

			/* Start a new task to fetch logging lines and send them out. */
			vUDPLoggingTaskCreate();

			xTasksAlreadyCreated = pdTRUE;
		}

		{
		uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
		char cBuffer[ 16 ];
			/* Print out the network configuration, which may have come from a DHCP
			server. */
			FreeRTOS_GetAddressConfiguration( &ulIPAddress, &ulNetMask, &ulGatewayAddress, &ulDNSServerAddress );
			FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );
			FreeRTOS_printf( ( "IP Address: %s\n", cBuffer ) );

			FreeRTOS_inet_ntoa( ulNetMask, cBuffer );
			FreeRTOS_printf( ( "Subnet Mask: %s\n", cBuffer ) );

			FreeRTOS_inet_ntoa( ulGatewayAddress, cBuffer );
			FreeRTOS_printf( ( "Gateway Address: %s\n", cBuffer ) );

			FreeRTOS_inet_ntoa( ulDNSServerAddress, cBuffer );
			FreeRTOS_printf( ( "DNS Server Address: %s\n", cBuffer ) );
		}
	}
}
/*-----------------------------------------------------------*/

#if( ipconfigUSE_DHCP_HOOK != 0 )
	eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress )
	{
	eDHCPCallbackAnswer_t xResult = eDHCPContinue;
	static BaseType_t xUseDHCP = pdTRUE;

		switch( eDHCPPhase )
		{
		case eDHCPPhasePreDiscover:		/* Driver is about to ask for a DHCP offer. */
			/* Returning eDHCPContinue. */
			break;
		case eDHCPPhasePreRequest:		/* Driver is about to request DHCP an IP address. */
			if( xUseDHCP == pdFALSE )
			{
				xResult = eDHCPStopNoChanges;
			}
			else
			{
				xResult = eDHCPContinue;
			}
			break;
		}
		lUDPLoggingPrintf( "DHCP %s: use = %d: %s\n",
			( eDHCPPhase == eDHCPPhasePreDiscover ) ? "pre-discover" : "pre-request",
			xUseDHCP,
			xResult == eDHCPContinue ? "Continue" : "Stop" );
		return xResult;
	}
#endif	/* ipconfigUSE_DHCP_HOOK */
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	vAssertCalled( __FILE__, __LINE__ );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

UBaseType_t uxRand( void )
{
const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

	/* Utility function to generate a pseudo random number. */

	ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
	return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
}
/*-----------------------------------------------------------*/

static void prvSRand( UBaseType_t ulSeed )
{
	/* Utility function to seed the pseudo random number generator. */
	ulNextRand = ulSeed;
}
/*-----------------------------------------------------------*/

void vApplicationPingReplyHook( ePingReplyStatus_t eStatus, uint16_t usIdentifier )
{
}
/*-----------------------------------------------------------*/

const char *pcApplicationHostnameHook( void )
{
	/* Assign the name "FreeRTOS" to this network node.  This function will be
	called during the DHCP: the machine will be registered with an IP address
	plus this name. */
	return mainHOST_NAME;
}
/*-----------------------------------------------------------*/

BaseType_t xApplicationDNSQueryHook( const char *pcName )
{
BaseType_t xReturn;

	/* Determine if a name lookup is for this node.  Two names are given
	to this node: that returned by pcApplicationHostnameHook() and that set
	by mainDEVICE_NICK_NAME. */
	if( strcasecmp( pcName, pcApplicationHostnameHook() ) == 0 )
	{
		xReturn = pdPASS;
	}
	else if( strcasecmp( pcName, mainDEVICE_NICK_NAME ) == 0 )
	{
		xReturn = pdPASS;
	}
	else
	{
		xReturn = pdFAIL;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeHeapSpace;

	/* This is just a trivial example of an idle hook.  It is called on each
	cycle of the idle task.  It must *NOT* attempt to block.  In this case the
	idle task just queries the amount of FreeRTOS heap that remains.  See the
	memory management section on the http://www.FreeRTOS.org web site for memory
	management options.  If there is a lot of heap memory free then the
	configTOTAL_HEAP_SIZE value in FreeRTOSConfig.h can be reduced to free up
	RAM. */
	xFreeHeapSpace = xPortGetFreeHeapSize();

	/* Remove compiler warning about xFreeHeapSpace being set but never used. */
	( void ) xFreeHeapSpace;
}
/*-----------------------------------------------------------*/

void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
volatile unsigned long ul = 0;

	( void ) pcFile;
	( void ) ulLine;

	taskENTER_CRITICAL();
	{
		/* Set ul to a non-zero value using the debugger to step out of this
		function. */
		while( ul == 0 )
		{
			portNOP();
		}
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	#if( mainCREATE_SIMPLE_BLINKY_DEMO_ONLY == 0 )
	{
		/* The full demo includes a software timer demo/test that requires
		prodding periodically from the tick interrupt. */
		vTimerPeriodicISRTests();

		/* Call the periodic queue overwrite from ISR demo. */
		vQueueOverwritePeriodicISRDemo();

		/* Call the periodic event group from ISR demo. */
		vPeriodicEventGroupsProcessing();

		/* Call the periodic test that uses mutexes form an interrupt. */
		vInterruptSemaphorePeriodicTest();
	}
	#endif
}
/*-----------------------------------------------------------*/

void vConfigureTickInterrupt( void )
{
alt_freq_t ulTempFrequency;
const alt_freq_t ulMicroSecondsPerSecond = 1000000UL;
void FreeRTOS_Tick_Handler( void );

	/* Interrupts are disabled when this function is called. */

	/* Initialise the general purpose timer modules. */
	alt_gpt_all_tmr_init();

	/* ALT_CLK_MPU_PERIPH = mpu_periph_clk */
	alt_clk_freq_get( ALT_CLK_MPU_PERIPH, &ulTempFrequency );

	/* Use the local private timer. */
	alt_gpt_counter_set( ALT_GPT_CPU_PRIVATE_TMR, ulTempFrequency / configTICK_RATE_HZ );

	/* Sanity check. */
	configASSERT( alt_gpt_time_microsecs_get( ALT_GPT_CPU_PRIVATE_TMR ) == ( ulMicroSecondsPerSecond / configTICK_RATE_HZ ) );

	/* Set to periodic mode. */
	alt_gpt_mode_set( ALT_GPT_CPU_PRIVATE_TMR, ALT_GPT_RESTART_MODE_PERIODIC );

	/* The timer can be started here as interrupts are disabled. */
	alt_gpt_tmr_start( ALT_GPT_CPU_PRIVATE_TMR );

	/* Register the standard FreeRTOS Cortex-A tick handler as the timer's
	interrupt handler.  The handler clears the interrupt using the
	configCLEAR_TICK_INTERRUPT() macro, which is defined in FreeRTOSConfig.h. */
	vRegisterIRQHandler( ALT_INT_INTERRUPT_PPI_TIMER_PRIVATE, ( alt_int_callback_t ) FreeRTOS_Tick_Handler, NULL );

	/* This tick interrupt must run at the lowest priority. */
	alt_int_dist_priority_set( ALT_INT_INTERRUPT_PPI_TIMER_PRIVATE, portLOWEST_USABLE_INTERRUPT_PRIORITY << portPRIORITY_SHIFT );

	/* Ensure the interrupt is forwarded to the CPU. */
	alt_int_dist_enable( ALT_INT_INTERRUPT_PPI_TIMER_PRIVATE );

	/* Finally, enable the interrupt. */
	alt_gpt_int_clear_pending( ALT_GPT_CPU_PRIVATE_TMR );
	alt_gpt_int_enable( ALT_GPT_CPU_PRIVATE_TMR );

}
/*-----------------------------------------------------------*/

void vRegisterIRQHandler( uint32_t ulID, alt_int_callback_t pxHandlerFunction, void *pvContext )
{
	if( ulID < ALT_INT_PROVISION_INT_COUNT )
	{
		xISRHandlers[ ulID ].pxISR = pxHandlerFunction;
		xISRHandlers[ ulID ].pvContext = pvContext;
	}
}
/*-----------------------------------------------------------*/

void vApplicationIRQHandler( uint32_t ulICCIAR )
{
uint32_t ulInterruptID;
void *pvContext;
alt_int_callback_t pxISR;

	/* Re-enable interrupts. */
	__asm ( "cpsie i" );

	/* The ID of the interrupt is obtained by bitwise anding the ICCIAR value
	with 0x3FF. */
	ulInterruptID = ulICCIAR & 0x3FFUL;

	if( ulInterruptID < ALT_INT_PROVISION_INT_COUNT )
	{
		/* Call the function installed in the array of installed handler
		functions. */
		pxISR = xISRHandlers[ ulInterruptID ].pxISR;
		pvContext = xISRHandlers[ ulInterruptID ].pvContext;
		pxISR( ulICCIAR, pvContext );
	}
}

extern void vOutputChar( const char cChar, const TickType_t xTicksToWait  );
void vOutputChar( const char cChar, const TickType_t xTicksToWait  )
{
	/* printf-stdarg.c needs this function. */
}

#define KILO_BYTE						( 1024u )
#define MEGA_BYTE						( KILO_BYTE * KILO_BYTE )

#define BOOT_ROM_ORIGIN					( 0xfffd0000u )
#define BOOT_ROM_LENGTH					( 64u * KILO_BYTE )

#define ON_CHIP_RAM_ORIGIN				( 0xffff0000u )
#define ON_CHIP_RAM_LENGTH				( 64u * KILO_BYTE )

#define SD_RAM_ORIGIN					( 0x00100000u )
#define SD_RAM_LENGTH					( 1023u * MEGA_BYTE )

#define IN_RANGE( ADDR, START, LEN )	( ( ADDR >= START ) && ( ADDR <= START + ( LEN - 1 ) ) )

//  boot_rom (rx) : ORIGIN = 0xfffd0000, LENGTH = 64K
//  oc_ram (rwx) : ORIGIN = 0xffff0000, LENGTH = 64K
//  ram (rwx) : ORIGIN = 0x100000, LENGTH = 1023M

extern BaseType_t xApplicationMemoryPermissions( uint32_t aAddress );
BaseType_t xApplicationMemoryPermissions( uint32_t aAddress )
{
BaseType_t xReturn;

	if( IN_RANGE( aAddress, BOOT_ROM_ORIGIN, BOOT_ROM_LENGTH ) )
	{
		xReturn = 1;
	}
	else if( IN_RANGE( aAddress, SD_RAM_ORIGIN, SD_RAM_LENGTH ) )
	{
		xReturn = 3;
	}
	else if( IN_RANGE( aAddress, ON_CHIP_RAM_ORIGIN, ON_CHIP_RAM_LENGTH ) )
	{
		xReturn = 3;
	}
	else
	{
		xReturn = 0;
	}

	return xReturn;
}

int _read(int handle, void *buf, unsigned len)
{
	return 0;
}

int _write(int handle, void *buf, unsigned len)
{
	return len;
}

int _close(int file)
{
	return -1;
}

extern int _end;

caddr_t _sbrk(int incr)
{
	static unsigned char *heap = NULL;
	unsigned char *prev_heap;

	if (heap == NULL) {
		heap = (unsigned char *)&_end;
	}
	prev_heap = heap;

	heap += incr;

	return (caddr_t) prev_heap;
}

int _lseek(int file, int ptr, int dir)
{
	return 0;
}

//	void __attribute__ ( ( interrupt ) ) __cs3_isr_dabort( void )
//	{
//		while(1);
//	}

void __cs3_isr_dabort( void )
{
	vAssertCalled( __FILE__, __LINE__ );
}
