/*******************************************************************************
 * @file     main.c
 * @author   USB PD Firmware Team
 *
 * Copyright 2018 ON Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by ON Semiconductor under
 * limited terms and conditions. The terms and conditions pertaining to the
 * software and/or documentation are available at
 * http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf
 * ("ON Semiconductor Standard Terms and Conditions of Sale, Section 8 Software").
 *
 * DO NOT USE THIS SOFTWARE AND/OR DOCUMENTATION UNLESS YOU HAVE CAREFULLY
 * READ AND YOU AGREE TO THE LIMITED TERMS AND CONDITIONS. BY USING THIS
 * SOFTWARE AND/OR DOCUMENTATION, YOU AGREE TO THE LIMITED TERMS AND CONDITIONS.
 ******************************************************************************/
#include <stdio.h> 

#include "../core/core.h"
#include "../core/modules/dpm.h"


#ifdef FSC_DEBUG
#include "usbd_main.h"
#include "usbd_desc.h"
#include "usbd_hid.h"
#endif /* FSC_DEBUG */

#include "../core/FSCTypes.h"

/*******************************************************************************
 * Prototype of functions declared in the file
 ******************************************************************************/

volatile FSC_BOOL haveINTReady = FALSE;

#ifdef FSC_DEBUG
volatile FSC_BOOL haveUSBInMsg = FALSE;

void platform_set_debug(FSC_U8 port, FSC_BOOL enable);
#endif /* FSC_DEBUG */

/*******************************************************************************
 * Static variables
 ******************************************************************************/
static DevicePolicyPtr_t dpm;
static Port_t ports[NUM_PORTS];           /* Array of ports */
/**
 * @brief Program entry point and container of main loop
 * @param None
 * @param None
 */
void FusbInitial( void )
{

    DPM_Init(&dpm);

    ports[0].dpm = dpm;
    ports[0].PortID = 0;
    ports[0].irqCount = 0;
    core_initialize(&ports[0], FUSB300SlaveAddr);

    DPM_AddPort(dpm, &ports[0]);
}

void FusbLoop(void){
    /* Disable the timer interrupt */
	static PolicyState_t policy = -1;
	static ConnectionState conn = -1;
	static FSC_U32 timer_value;

	if(haveINTReady){
		core_state_machine(&ports[0]);
		if(ports[0].PolicyState != policy || ports[0].ConnState != conn ){
			printf("Connection %d Policy %d\n", ports[0].ConnState , ports[0].PolicyState);
			conn = ports[0].ConnState;
			policy = ports[0].PolicyState;
		}

		haveINTReady = FALSE;
	}

    /*
     * It is possible for the state machine to go into idle mode with
     * the interrupt pin still low and as a result the edge-sensitive
     * IRQ won't get triggered.  Check here before returning to wait
     * on the IRQ.
     */
    if(platform_get_device_irq_state(ports[0].PortID))
    {
        haveINTReady = TRUE;
    }
    else
    {
        /* If needed, enable timer interrupt before idling */
        timer_value = core_get_next_timeout(&ports[0]);

        if (timer_value > 0)
        {
            if (timer_value == 1)
            {
                /* A value of 1 indicates that a timer has expired
                 * or is about to expire and needs further processing.
                 */
                haveINTReady = TRUE;
            }
        }
    }
}

void FusbSetRequestVolatage(int output){
	int requestVolatage = 100;

	//find minimal voltage fix output
	 for (int i = 0; i < ports[0].SrcCapsHeaderReceived.NumDataObjects; i++)
	 {
	        if (ports[0].SrcCapsReceived[i].PDO.SupplyType == pdoTypeFixed)
	        {
            	requestVolatage = ports[0].SrcCapsReceived[i].FPDOSupply.Voltage;
	            if ( ports[0].SrcCapsReceived[i].FPDOSupply.Voltage * 50 > output + 800)
	            {
	                break;
	            }
	        }
	}

	if(requestVolatage != ports[0].snk_caps[0].FPDOSink.Voltage){
		ports[0].snk_caps[0].FPDOSink.Voltage = requestVolatage;
		ports[0].PolicyState = peSinkEvaluateCaps;
	}
	haveINTReady = TRUE;
	int timeout = 200;

	while(timeout--){
		FusbLoop();
		platform_delay_10us(100);
	}
}


