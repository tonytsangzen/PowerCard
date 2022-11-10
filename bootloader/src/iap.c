/********************************** (C) COPYRIGHT *******************************
 * File Name          : iap.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2022/03/15
 * Description        : USB IAP����
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
#include "iap.h"
#include "crc32.h"

#undef pSetupReqPak     /* ����������ͷ�ļ���ͻ  */
#define pSetupReqPak          ((PUSB_SETUP_REQ)EP0_Databuf)

void myDevEP2_IN_Deal(uint8_t s);
void myDevEP2_OUT_Deal(uint8_t l);

#define DevEP0SIZE  64
// �豸������
const uint8_t MyDevDescr[] =
{
	0x12,0x01,0x10,0x01,0xFF,0x00,0x00,DevEP0SIZE,
	0x86,0x1A,0x23,0x75,0x63,0x02,0x00,0x02,
    0x00,0x01
};
// ����������
const uint8_t MyCfgDescr[] =
{
	0x09,0x02,0x27,0x00,0x01,0x01,0x00,0x80,0xf0,              
	0x09,0x04,0x00,0x00,0x03,0xff,0x01,0x02,0x00,
    0x07,0x05,0x82,0x02,0x20,0x00,0x00,                        //�����ϴ��˵�
    0x07,0x05,0x02,0x02,0x20,0x00,0x00,                        //�����´��˵�
    0x07,0x05,0x81,0x03,0x08,0x00,0x01
};
// ����������
const uint8_t MyLangDescr[] =
{ 0x04, 0x03, 0x09, 0x04 };
// ������Ϣ
const uint8_t MyManuInfo[] =
{ 0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0 };
// ��Ʒ��Ϣ
const uint8_t MyProdInfo[] =
{ 0x0C, 0x03, 'C', 0, 'H', 0, '5', 0, '8', 0, 'x', 0 };

/*��Ʒ������*/
const uint8_t StrDesc[28] =
{
  0x1C,0x03,0x55,0x00,0x53,0x00,0x42,0x00,
  0x32,0x00,0x2E,0x00,0x30,0x00,0x2D,0x00,
  0x53,0x00,0x65,0x00,0x72,0x00,0x69,0x00,
  0x61,0x00,0x6C,0x00
};

const uint8_t Return1[2] = {0x31,0x00};
const uint8_t Return2[2] = {0xC3,0x00};
const uint8_t Return3[2] = {0x9F,0xEE};

/**********************************************************/
uint8_t DevConfig;
uint8_t SetupReqCode;
uint16_t SetupReqLen;
const uint8_t *pDescr;

/******** �û��Զ������˵�RAM ****************************************/
__attribute__((aligned(4)))   uint8_t EP0_Databuf[64 + 64 + 64]; //ep0(64)+ep4_out(64)+ep4_in(64)
__attribute__((aligned(4)))   uint8_t EP1_Databuf[64 + 64]; //ep1_out(64)+ep1_in(64)
__attribute__((aligned(4)))   uint8_t EP2_Databuf[64 + 64]; //ep2_out(64)+ep2_in(64)
__attribute__((aligned(4)))   uint8_t EP3_Databuf[64 + 64]; //ep3_out(64)+ep3_in(64)

__attribute__((aligned(4)))   uint8_t g_write_buf[256 + 64]; //ÿ����256�ֽ���дflash�������ٶ�
volatile uint16_t g_buf_write_ptr = 0;
volatile uint32_t g_flash_write_ptr = 0;
uint32_t g_tcnt;
__attribute__((aligned(4))) iap_cmd_t g_iap_cmd;

/*********************************************************************
 * @fn      USB_DevTransProcess
 *
 * @brief   IAP USB��ѭ��,�����ram�����У������ٶ�.
 *
 * @param   None.
 *
 * @return  None.
 */
__attribute__((section(".highcode")))
void USB_DevTransProcess(void)
{
    uint8_t len, chtype;
    uint8_t intflag, errflag = 0;

    intflag = R8_USB_INT_FG;
    if (intflag & RB_UIF_TRANSFER)
    {
        if (intflag & RB_U_IS_NAK)
        {
        }
        else
        {
        	// �����������ƺͶ˵��
            switch (R8_USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
            {
            case UIS_TOKEN_IN:
            {
                switch (SetupReqCode)
                {
                case USB_GET_DESCRIPTOR:
                    len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen;   // ���δ��䳤��
                    my_memcpy(EP0_Databuf, pDescr, len);  /* �����ϴ����� */
                    SetupReqLen -= len;
                    pDescr += len;
                    R8_UEP0_T_LEN = len;
                    R8_UEP0_CTRL ^= RB_UEP_T_TOG;                          // ��ת
                    break;
                case USB_SET_ADDRESS:
                    R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT) | SetupReqLen;
                    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                    break;
                default:
                    R8_UEP0_T_LEN = 0;            // ״̬�׶�����жϻ�����ǿ���ϴ�0�������ݰ��������ƴ���
                    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                    break;
                }
            }
            break;

            case UIS_TOKEN_OUT:
                len = R8_USB_RX_LEN;
                break;
			
			 case UIS_TOKEN_OUT | 1 :
       		 {
       		   if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
       		   {                       // ��ͬ�������ݰ�������
       		     len = R8_USB_RX_LEN;
       		     //DevEP1_OUT_Deal( len );
       		   }
       		 }
       		   break;

       		 case UIS_TOKEN_IN | 1 :
       		   R8_UEP1_CTRL = ( R8_UEP1_CTRL & ~MASK_UEP_T_RES ) | UEP_T_RES_NAK;
       		   break;

       		 case UIS_TOKEN_OUT | 2 :
       		 {
       		   if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
       		   {                       // ��ͬ�������ݰ�������
       		     len = R8_USB_RX_LEN;
       		     myDevEP2_OUT_Deal( len );
       		   }
       		 }
       		   break;

       		 case UIS_TOKEN_IN | 2 :
       		   R8_UEP2_CTRL = ( R8_UEP2_CTRL & ~MASK_UEP_T_RES ) | UEP_T_RES_NAK;
       		   break;

       		 case UIS_TOKEN_OUT | 3 :
       		 {
       		   if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
       		   {                       // ��ͬ�������ݰ�������
       		     len = R8_USB_RX_LEN;
       		     //DevEP3_OUT_Deal( len );
       		   }
       		 }
       		   break;

			case UIS_TOKEN_IN | 3 :
        	  R8_UEP3_CTRL = ( R8_UEP3_CTRL & ~MASK_UEP_T_RES ) | UEP_T_RES_NAK;
        	  break;

        	case UIS_TOKEN_OUT | 4 :
        	{
        	  if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
        	  {
        	    R8_UEP4_CTRL ^= RB_UEP_R_TOG;
        	    len = R8_USB_RX_LEN;
        	    //DevEP4_OUT_Deal( len );
        	  }
        	}
			break;

        	case UIS_TOKEN_IN | 4 :
        	  R8_UEP4_CTRL ^= RB_UEP_T_TOG;
        	  R8_UEP4_CTRL = ( R8_UEP4_CTRL & ~MASK_UEP_T_RES ) | UEP_T_RES_NAK;
        	  break;

            default:
                break;
            }
            R8_USB_INT_FG = RB_UIF_TRANSFER;

        }
        if (R8_USB_INT_ST & RB_UIS_SETUP_ACT)                   // Setup������
        {
            R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
            SetupReqLen = pSetupReqPak->wLength;
            SetupReqCode = pSetupReqPak->bRequest;
            chtype = pSetupReqPak->bRequestType;

            len = 0;
            errflag = 0;
            if ((pSetupReqPak->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD)
            {
				if( pSetupReqPak->bRequestType == 0xC0 )
    		    {
    		      if(SetupReqCode==0x5F)
    		      {
    		        pDescr = Return1;
    		        len = sizeof(Return1);
    		      }
    		      else if(SetupReqCode==0x95)
    		      {
    		        if((pSetupReqPak->wValue)==0x18)
    		        {
    		          pDescr = Return2;
    		          len = sizeof(Return2);
    		        }
    		        else if((pSetupReqPak->wValue)==0x06)
    		        {
    		          pDescr = Return3;
    		          len = sizeof(Return3);
    		        }
    		      }
    		      else
    		      {
    		        errflag = 0xFF;
    		      }
    		      memcpy(pEP0_DataBuf,pDescr,len);
    		    }
    		    else
    		    {
    		      len = 0;
    		    }
            }
            else /* ��׼���� */
            {
                switch (SetupReqCode)
                {
                case USB_GET_DESCRIPTOR:
                {
                    switch (((pSetupReqPak->wValue) >> 8))
                    {
                    case USB_DESCR_TYP_DEVICE:
                    {
                        pDescr = MyDevDescr;
                        len = MyDevDescr[0];
                    }
                    break;

                    case USB_DESCR_TYP_CONFIG:
                    {
                        pDescr = MyCfgDescr;
                        len = MyCfgDescr[2];
                    }
                    break;

                    case USB_DESCR_TYP_STRING:
                    {
                        switch ((pSetupReqPak->wValue) & 0xff)
                        {
                        case 1:
                            pDescr = MyManuInfo;
                            len = MyManuInfo[0];
                            break;
                        case 2:
                            pDescr = MyProdInfo;
                            len = MyProdInfo[0];
                            break;
                        case 0:
                            pDescr = MyLangDescr;
                            len = MyLangDescr[0];
                            break;
                        default:
                            errflag = 0xFF;                        // ��֧�ֵ��ַ���������
                            break;
                        }
                    }
                    break;

                    default:
                        errflag = 0xff;
                        break;
                    }
                    if (SetupReqLen > len)
                    {
                        SetupReqLen = len;      //ʵ�����ϴ��ܳ���
                    }
                    len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                    my_memcpy(EP0_Databuf, pDescr, len);
                    pDescr += len;
                }
                break;

                case USB_SET_ADDRESS:
                    SetupReqLen = (pSetupReqPak->wValue) & 0xff;
                    break;

                case USB_GET_CONFIGURATION:
                    EP0_Databuf[0] = DevConfig;
                    if (SetupReqLen > 1)
                    {
                        SetupReqLen = 1;
                    }
                    break;

                case USB_SET_CONFIGURATION:
                    DevConfig = (pSetupReqPak->wValue) & 0xff;
                    break;

                case USB_CLEAR_FEATURE:
                {
                    if ((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)    // �˵�
                    {
                        switch ((pSetupReqPak->wIndex) & 0xff)
                        {
                        case 0x82:
                            R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                            break;
                        case 0x02:
                            R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                            break;
                        case 0x81:
                            R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                            break;
                        case 0x01:
                            R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                            break;
                        default:
                            errflag = 0xFF;                            // ��֧�ֵĶ˵�
                            break;
                        }
                    }
                    else
                    {
                        errflag = 0xFF;
                    }
                }
                break;

                case USB_GET_INTERFACE:
                    EP0_Databuf[0] = 0x00;
                    if (SetupReqLen > 1)
                    {
                        SetupReqLen = 1;
                    }
                    break;

                case USB_GET_STATUS:
                    EP0_Databuf[0] = 0x00;
                    EP0_Databuf[1] = 0x00;
                    if (SetupReqLen > 2)
                    {
                        SetupReqLen = 2;
                    }
                    break;

                default:
                    errflag = 0xff;
                    break;
                }
            }
            if (errflag == 0xff)        // �����֧��
            {
//                  SetupReqCode = 0xFF;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;    // STALL
            }
            else
            {
                if (chtype & 0x80)     // �ϴ�
                {
                    len = (SetupReqLen > DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                    SetupReqLen -= len;
                }
                else
                {
                    len = 0;        // �´�
                }
                R8_UEP0_T_LEN = len;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;    // Ĭ�����ݰ���DATA1
            }
        }
        R8_USB_INT_FG = RB_UIF_TRANSFER;
    }
    else if (intflag & RB_UIF_BUS_RST)
    {
        R8_USB_DEV_AD = 0;
        R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
        R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
        R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
        R8_USB_INT_FG = RB_UIF_BUS_RST;
    }
    else if (intflag & RB_UIF_SUSPEND)
    {
        // ����
        R8_USB_INT_FG = RB_UIF_SUSPEND;
    }
    else
    {
        R8_USB_INT_FG = intflag;
    }
}

/*********************************************************************
 * @fn      myDevEP2_OUT_Deal
 *
 * @brief   IAP USB���ݴ�����������ram�����������ٶ�.
 *
 * @param   None.
 *
 * @return  None.
 */
 int xmodemReceive(char ch);

#pragma pack(1)
typedef struct {
	uint32_t smagic;
	uint8_t major;
	uint8_t minor;
	uint8_t flag;
	uint8_t resv;
	uint32_t loadaddr;
	uint32_t length;
	uint32_t crc;
	uint32_t resv0;
	uint32_t resv1;
	uint32_t emagic;
}fw_info_t;
#pragma pack()

fw_info_t fwInfo = {0};


__attribute__((section(".highcode")))
int program(unsigned char*buf, int size){
	uint32_t paddr = fwInfo.loadaddr + fwInfo.length;
   //if we reach block boundary. do erase first
   if((paddr % EEPROM_BLOCK_SIZE) == 0){
		while(FLASH_ROM_ERASE(paddr, size));
   }

   return FLASH_ROM_WRITE(paddr, (PUINT32)buf, size);
}

__attribute__((section(".highcode")))
int program_process(unsigned char* buf, int size){
	/*init firmware info*/
	if(fwInfo.smagic == 0){
		fwInfo.smagic = 0xfeedbeef;
		//ToDO: get version form fw?
		fwInfo.major = 0;
		fwInfo.minor = 0;

		fwInfo.loadaddr = APP_CODE_START_ADDR; 
		fwInfo.length = 0;
	}
	if(program(buf, size) == 0){
		fwInfo.length += size;
		return 0;
	}
   	return -1;
}

__attribute__((section(".highcode")))
int  program_end(void){
	fwInfo.emagic = 0xdeadbeef;		
	fwInfo.crc = crc32((char*)fwInfo.loadaddr, fwInfo.length);
	if(program((unsigned char*)&fwInfo, sizeof(fw_info_t)) == 0){
		g_tcnt = 3000000 - 20;
		return 0;
	}
	return -1;
 }

__attribute__((section(".highcode")))
void check_and_run(void){
	fw_info_t *pinfo = (fw_info_t*)APP_CODE_START_ADDR;
	for(int i = 0; i < (FLASH_ROM_MAX_SIZE - APP_CODE_START_ADDR)/sizeof(fw_info_t); i++){
		if(pinfo->smagic == 0xfeedbeef && pinfo->emagic == 0xdeadbeef){
			if(pinfo->crc == crc32((char*)pinfo->loadaddr, pinfo->length)){
				void (*p)(void) = (void (*)(void))APP_CODE_START_ADDR;
				p();
			}
		}
		pinfo++;
	}
}
__attribute__((section(".highcode")))
void myDevEP2_OUT_Deal(uint8_t l)
{
    /* �û����Զ��� */
	g_tcnt = 0;
	for(int i = 0; i < l; i++)
		xmodemReceive(EP2_Databuf[i]);
	return;
}

/*******************************************************************************
 * Function Name  : myDevEP2_IN_Deal
 * Description    : �˵�2�����ϴ�
 * Input          : l: �ϴ����ݳ���(<64B)
 * Return         : None
 *******************************************************************************/
__attribute__((section(".highcode")))
void myDevEP2_IN_Deal(uint8_t s)
{
    EP2_Databuf[64] = s;
    R8_UEP2_T_LEN = 1;
    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK; //enable send
}

/*********************************************************************
 * @fn      my_memcpy
 *
 * @brief   ���ݿ�������,�����ram�����У������ٶ�
 *
 * @param   None.
 *
 * @return  None.
 */
__attribute__((section(".highcode")))
void my_memcpy(void *dst, const void *src, uint32_t l)
{
    uint32_t len = l;
    PUINT8 pdst = (PUINT8) dst;
    PUINT8 psrc = (PUINT8) src;
    while (len)
    {
        *pdst++ = *psrc++;
        len--;
    }
}
