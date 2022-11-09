#include "../core/platform.h"
#include "CH58x_common.h"

//#define I2C_DEBUG

void platform_set_vbus_lvl_enable(FSC_U8 port,
                                  VBUS_LVL level,
                                  FSC_BOOL enable,
                                  FSC_BOOL disableOthers)
{

}

FSC_BOOL platform_get_vbus_lvl_enable(FSC_U8 port, VBUS_LVL level)
{

    return FALSE;
}

void platform_set_vbus_discharge(FSC_U8 port, FSC_BOOL enable)
{
    /* Enable/Disable the discharge path */
    /* TODO - Implement as needed on platforms that support this feature. */
}

FSC_BOOL platform_get_device_irq_state(FSC_U8 port)
{
    /* Return the state of the device interrupt signal. */
	PWR_PeriphClkCfg(DISABLE, BIT_SLP_CLK_UART0);
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeIN_PU);
    return GPIOB_ReadPortPin(GPIO_Pin_7) == 0? TRUE:FALSE;
}

FSC_BOOL platform_i2c_write(FSC_U8 SlaveAddress,
                            FSC_U8 RegAddrLength,
                            FSC_U8 DataLength,
                            FSC_U8 PacketSize,
                            FSC_U8 IncSize,
                            FSC_U32 RegisterAddress,
                            FSC_U8* Data)
{
	  int i=0;

	  while( I2C_GetFlagStatus( I2C_FLAG_BUSY ) != RESET );
	  I2C_GenerateSTART( ENABLE );
	  while( !I2C_CheckEvent( I2C_EVENT_MASTER_MODE_SELECT ) );
	  I2C_Send7bitAddress( SlaveAddress, I2C_Direction_Transmitter );
	  while( !I2C_CheckEvent( I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );

	  while( I2C_GetFlagStatus( I2C_FLAG_TXE ) ==  RESET );
	  I2C_SendData( RegisterAddress );

	  while( i<PacketSize )
	  {
	    if( I2C_GetFlagStatus( I2C_FLAG_TXE ) !=  RESET )
	    {
	      I2C_SendData( Data[i] );
	      i++;
	    }
	  }
	  while( !I2C_CheckEvent( I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );
	  I2C_GenerateSTOP( ENABLE );

#ifdef I2C_DEBUG
	  printf("write %02x %d byte: %02x\n", PacketSize, RegisterAddress, Data[0]);
#endif
	  return TRUE;
}

FSC_BOOL platform_i2c_read( FSC_U8 SlaveAddress,
                            FSC_U8 RegAddrLength,
                            FSC_U8 DataLength,
                            FSC_U8 PacketSize,
                            FSC_U8 IncSize,
                            FSC_U32 RegisterAddress,
                            FSC_U8* Data)
{
	  int i=0;

	  while( I2C_GetFlagStatus( I2C_FLAG_BUSY ) != RESET );

	  I2C_GenerateSTART( ENABLE );
	  while( !I2C_CheckEvent( I2C_EVENT_MASTER_MODE_SELECT ) );

	  I2C_Send7bitAddress( SlaveAddress, I2C_Direction_Transmitter );
	  while( !I2C_CheckEvent( I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );

	  while( I2C_GetFlagStatus( I2C_FLAG_TXE ) ==  RESET );
	  I2C_SendData( RegisterAddress );
	  while( !I2C_CheckEvent( I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );

	  if(PacketSize > 1)
		  I2C_AcknowledgeConfig(ENABLE);
	  else
		  I2C_AcknowledgeConfig(DISABLE);

	  I2C_GenerateSTART( ENABLE );
	  while( !I2C_CheckEvent( I2C_EVENT_MASTER_MODE_SELECT ) );

	  I2C_Send7bitAddress( SlaveAddress, I2C_Direction_Receiver );
	  while( !I2C_CheckEvent( I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED ) );

	  while( i<PacketSize)
	  {

		    if(I2C_GetFlagStatus(I2C_FLAG_RXNE) != RESET)
		    {

		        Data[i] = I2C_ReceiveData();
		        i++;

		        if(i == PacketSize - 1)
		        	I2C_AcknowledgeConfig(DISABLE);
		   }
	  }

	  I2C_GenerateSTOP( ENABLE );
#ifdef I2C_DEBUG
	 	printf("read %02x %d byte: %02x\n", PacketSize, RegisterAddress, Data[0]);
#endif
	  return TRUE;

}

/*****************************************************************************
* Function:        platform_delay_10us
* Input:           delayCount - Number of 10us delays to wait
* Return:          None
* Description:     Perform a software delay in intervals of 10us.
******************************************************************************/
void platform_delay_10us(FSC_U32 delayCount)
{
	while(delayCount--)
		mDelayuS(10);
}

void platform_set_pps_voltage(FSC_U8 port, FSC_U32 mv)
{

}

FSC_U16 platform_get_pps_voltage(FSC_U8 port)
{
    return 0;
}

void platform_set_pps_current(FSC_U8 port, FSC_U32 ma)
{
}

FSC_U16 platform_get_pps_current(FSC_U8 port)
{
    return 0;
}

FSC_U32 platform_get_system_time(void)
{
    return TMOS_GetSystemClock()*10/16;
}

