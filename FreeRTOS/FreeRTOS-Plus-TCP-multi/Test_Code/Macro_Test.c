/*
 * FreeRTOS+TCP Labs Build 150825 (C) 2014 Real Time Engineers ltd.
 * Authors include Hein Tibosch and Richard Barry
 *
 *******************************************************************************
 ***** NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ***
 ***                                                                         ***
 ***                                                                         ***
 ***   FREERTOS+TCP IS STILL IN THE LAB:                                     ***
 ***                                                                         ***
 ***   This product is functional and is already being used in commercial    ***
 ***   products.  Be aware however that we are still refining its design,    ***
 ***   the source code does not yet fully conform to the strict coding and   ***
 ***   style standards mandated by Real Time Engineers ltd., and the         ***
 ***   documentation and testing is not necessarily complete.                ***
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
 * - Open source licensing -
 * While FreeRTOS+TCP is in the lab it is provided only under version two of the
 * GNU General Public License (GPL) (which is different to the standard FreeRTOS
 * license).  FreeRTOS+TCP is free to download, use and distribute under the
 * terms of that license provided the copyright notice and this text are not
 * altered or removed from the source files.  The GPL V2 text is available on
 * the gnu.org web site, and on the following
 * URL: http://www.FreeRTOS.org/gpl-2.0.txt.  Active early adopters may, and
 * solely at the discretion of Real Time Engineers Ltd., be offered versions
 * under a license other then the GPL.
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
 * http://www.FreeRTOS.org/udp
 *
 */

/*
 * Some routines used for internal testing:
 * Macro's and CRC calculations
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_ARP.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_DHCP.h"
#include "NetworkInterface.h"
#include "NetworkBufferManagement.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)	(BaseType_t)(sizeof(x)/sizeof(x)[0])
#endif

#if( ipconfigINCLUDE_TEST_CODE == 1 )

#ifndef __WIN32__
#warning * * * * * * * * * * * * * * * * * * * *
#warning        Testing code is enabled
#warning * * * * * * * * * * * * * * * * * * * *
#endif

const unsigned portCHAR testMsg[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x45, 0x00,
	0x00, 0x49, 0x06, 0xd5, 0x00, 0x00, 0x80, 0x06, 0xad, 0x5d, 0xc0, 0xa8, 0x02, 0x64, 0xc0, 0xa8,
	0x02, 0xc8, 0x00, 0x17, 0x83, 0xc0, 0x6c, 0xfd, 0x31, 0xb4, 0x06, 0x47, 0x20, 0x0d, 0x50, 0x18,
	0x2d, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x34, 0x33, 0x35, 0x38, 0x2e, 0x36, 0x33, 0x36, 0x2e, 0x31,
	0x33, 0x32, 0x20, 0x5b, 0x31, 0x5d, 0x3a, 0x20, 0x55, 0x44, 0x50, 0x20, 0x50, 0x6f, 0x72, 0x74,
	0x20, 0x20, 0x20, 0x20, 0x36, 0x38, 0x0a
};

//static portINLINE int32_t  FreeRTOS_max_int32  (int32_t  a, int32_t  b) { return a >= b ? a : b; }
//static portINLINE uint32_t FreeRTOS_max_uint32 (uint32_t a, uint32_t b) { return a >= b ? a : b; }
//static portINLINE int32_t  FreeRTOS_min_int32  (int32_t  a, int32_t  b) { return a <= b ? a : b; }
//static portINLINE uint32_t FreeRTOS_min_uint32 (uint32_t a, uint32_t b) { return a <= b ? a : b; }
//static portINLINE uint32_t FreeRTOS_round_up   (uint32_t a, uint32_t d) { return d * ( ( a + d - 1 ) / d); }
//static portINLINE uint32_t FreeRTOS_round_down (uint32_t a, uint32_t d) { return d * ( a / d); }

static portINLINE BaseType_t xSequenceLessThan (uint32_t a, uint32_t b)
{
	return ((b - a - 1) & 0x80000000U) == 0;
}

static portINLINE BaseType_t xSequenceLessThanOrEqual (uint32_t a, uint32_t b)
{
	return ((b - a) & 0x80000000U) == 0;
}

static portINLINE BaseType_t xSequenceGreaterThan (uint32_t a, uint32_t b)
{
	return ((a - b - 1) & 0x80000000U) == 0;
}

static portINLINE BaseType_t xSequenceGreaterThanOrEqual (uint32_t a, uint32_t b)
{
	return ((a - b) & 0x80000000U) == 0;
}

typedef union _xUnion32 {
	uint32_t u32;
	uint16_t u16[2];
	uint8_t u8[4];
} xUnion32;

typedef union _xUnionPtr {
	uint32_t *u32ptr;
	uint16_t *u16ptr;
	uint8_t *u8ptr;
} xUnionPtr;


static BaseType_t incOne( BaseType_t *plCount )
{
	(*plCount)++;
	return *plCount > 0;
}

static BaseType_t expressionShortCuts( )
{
	BaseType_t index;
	BaseType_t subtract[2] = { 0, 0 };
	BaseType_t lCount = 0;
	BaseType_t lSum;
	for( index = 0; index < 10; index++) {
		if( ( index & 1 ) && incOne( &lCount ) )
		{
			subtract[0]++;
		}
		if( ( index & 1 ) || incOne( &lCount ) )
		{
			if( ( index & 1 ) == 0 )
			{
				subtract[1]++;
			}
		}
	}
	lSum = lCount - subtract[0] - subtract[1];
	FreeRTOS_debug_printf( ( "expressionShortCuts: %ld - %ld - %ld = %ld\n",
		lCount, subtract[0], subtract[1], lSum ) );
	/* Function should return 1 */
	return lSum == 0 && subtract[0] == 5 && subtract[1] == 5;
}

void prvTCPInternalTesting ()
{
	BaseType_t count;
	uint32_t ulBig    = 0xA0923102L;
	uint32_t ulSmall  = 0x20923102L;
	int32_t lBig      = 2L;
	int32_t lSmall    = -10L;
	uint32_t ulAmount = 500;
	uint32_t ulDelta  = 12;
	uint32_t ulRndUp  = 504;
	uint32_t ulRndDn  = 492;

	uint16_t usNetVal;
	uint32_t ulNetVal;

	xUnion32 xUn32;
	portCHAR cResults[12];
	uint16_t sum;
	uint16_t usHostVal;
	uint32_t ulHostVal;

	xUn32.u32 = 0x12345678;

	usNetVal = 0x1234;
	ulNetVal = 0x12345678;
#if( ipconfigBYTE_ORDER == FREERTOS_LITTLE_ENDIAN )
	usHostVal = 0x3412;
	ulHostVal = 0x78563412;
	cResults[0] =
		(xUn32.u8[0] == 0x78) &&
		(xUn32.u8[1] == 0x56) &&
		(xUn32.u8[2] == 0x34) &&
		(xUn32.u8[3] == 0x12);
#else
	usHostVal = 0x1234;
	ulHostVal = 0x12345678;
	cResults[0] =
		(xUn32.u8[0] == 0x12) &&
		(xUn32.u8[1] == 0x34) &&
		(xUn32.u8[2] == 0x56) &&
		(xUn32.u8[3] == 0x78);
#endif

	cResults[1] =
		FreeRTOS_max_uint32( ulBig, ulSmall ) == ulBig &&
		FreeRTOS_min_uint32( ulBig, ulSmall ) == ulSmall;
	cResults[2] =
		FreeRTOS_max_int32( lBig, lSmall ) == lBig &&
		FreeRTOS_min_int32( lBig, lSmall ) == lSmall;
	cResults[3] =
		FreeRTOS_round_up( ulAmount, ulDelta ) == ulRndUp &&
		FreeRTOS_round_down( ulAmount, ulDelta ) == ulRndDn;
	cResults[4] =
		FreeRTOS_htons( usHostVal ) == usNetVal  &&
		FreeRTOS_htonl( ulNetVal  ) == ulHostVal &&
		FreeRTOS_htons( usHostVal ) == usNetVal  &&
		FreeRTOS_htonl( ulNetVal  ) == ulHostVal;
//	uint32_t ulPrev =
	cResults[5] =
		xSequenceLessThan (0x45901272ul, 0x45901272ul) == pdFALSE &&
		xSequenceLessThan (0x45901272ul, 0x45901271ul) == pdFALSE &&
		xSequenceLessThan (0xFFFFFF90ul, 0x00000549ul) == pdTRUE &&
		xSequenceLessThan (0x00000549ul, 0xFFFFFF90ul) == pdFALSE;
	cResults[6] =
		xSequenceLessThanOrEqual (0x45901272ul, 0x45901272ul) == pdTRUE &&
		xSequenceLessThanOrEqual (0x45901272ul, 0x45901271ul) == pdFALSE &&
		xSequenceLessThanOrEqual (0xFFFFFF90ul, 0x00000549ul) == pdTRUE &&
		xSequenceLessThanOrEqual (0x00000549ul, 0xFFFFFF90ul) == pdFALSE;
	cResults[7] =
		xSequenceGreaterThanOrEqual (0x45901272ul, 0x45901272ul) == pdTRUE &&
		xSequenceGreaterThanOrEqual (0x45901272ul, 0x45901271ul) == pdTRUE &&
		xSequenceGreaterThanOrEqual (0xFFFFFF90ul, 0x00000549ul) == pdFALSE &&
		xSequenceGreaterThanOrEqual (0x00000549ul, 0xFFFFFF90ul) == pdTRUE;
	cResults[8] =
		xSequenceGreaterThan (0x45901272ul, 0x45901272ul) == pdFALSE &&
		xSequenceGreaterThan (0x45901272ul, 0x45901271ul) == pdTRUE &&
		xSequenceGreaterThan (0xFFFFFF90ul, 0x00000549ul) == pdFALSE &&
		xSequenceGreaterThan (0x00000549ul, 0xFFFFFF90ul) == pdTRUE;
	sum = usGenerateProtocolChecksum( testMsg, pdTRUE );
	cResults[9] = FreeRTOS_ntohs( sum ) == 0x11dd;
	if( !cResults[9] )
		FreeRTOS_debug_printf( ( "Checksum = %04X\n", FreeRTOS_ntohs( sum ) ) );
	cResults[10] =
		( ipconfigRAND32( ) != ipconfigRAND32( ) && ipconfigRAND32( ) != ipconfigRAND32( ) );

	cResults[11] = expressionShortCuts( );
	{
		BaseType_t x;
		count = 0;
		for( x = 0; x < ARRAY_SIZE(cResults); x++ )
		{
			if( cResults[ x ] )
			{
				count++;
			}
		}
	}
	FreeRTOS_debug_printf( ( "FreeRTOS+TCP Macro_Test: %d%d%d%d  %d%d%d%d %d%d%d%d: %ld/%ld OK\n",
		cResults[0], cResults[1], cResults[2], cResults[3],
		cResults[4], cResults[5], cResults[6], cResults[7],
		cResults[8], cResults[9], cResults[10], cResults[11],
		count, ARRAY_SIZE(cResults) ) );
}

#endif  /*  ipconfigINCLUDE_TEST_CODE == 1  */
