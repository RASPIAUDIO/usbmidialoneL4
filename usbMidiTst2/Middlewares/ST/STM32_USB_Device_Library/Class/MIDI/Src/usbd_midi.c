/**
  ******************************************************************************
  * @file    usbd_midi.c
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   This file provides the Audio core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                AUDIO Class  Description
  *          ===================================================================
 *           This driver manages the Audio Class 1.0 following the "USB Device Class Definition for
  *           Audio Devices V1.0 Mar 18, 98".
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Standard AC Interface Descriptor management
  *             - 1 Audio Streaming Interface (with single channel, PCM, Stereo mode)
  *             - 1 Audio Streaming Endpoint
  *             - 1 Audio Terminal Input (1 channel)
  *             - Audio Class-Specific AC Interfaces
  *             - Audio Class-Specific AS Interfaces
  *             - AudioControl Requests: only SET_CUR and GET_CUR requests are supported (for Mute)
  *             - Audio Feature Unit (limited to Mute control)
  *             - Audio Synchronization type: Asynchronous
  *             - Single fixed audio sampling rate (configurable in usbd_conf.h file)
  *          The current audio class version supports the following audio features:
  *             - Pulse Coded Modulation (PCM) format
  *             - sampling rate: 48KHz. 
  *             - Bit resolution: 16
  *             - Number of channels: 2
  *             - No volume control
  *             - Mute/Unmute capability
  *             - Asynchronous Endpoints 
  *          
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *           
  *      
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_midi.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_MIDI 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_MIDI_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_MIDI_Private_Defines
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup USBD_MIDI_Private_Macros
  * @{
  */ 

                                         
/**
  * @}
  */ 




/** @defgroup USBD_MIDI_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_MIDI_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_MIDI_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_MIDI_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req);

static uint8_t  *USBD_MIDI_GetCfgDesc (uint16_t *length);

static uint8_t  *USBD_MIDI_GetDeviceQualifierDesc (uint16_t *length);

static uint8_t  USBD_MIDI_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_MIDI_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_MIDI_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_MIDI_EP0_TxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_MIDI_SOF (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_MIDI_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_MIDI_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static void USBD_MIDI_SetupTransfer(USBD_HandleTypeDef *pdev);

static void USBD_MIDI_FlushTX(USBD_HandleTypeDef *pdev);

/**
  * @}
  */ 

/** @defgroup USBD_MIDI_Private_Variables
  * @{
  */ 

USBD_ClassTypeDef  USBD_MIDI = 
{
  USBD_MIDI_Init,
  USBD_MIDI_DeInit,
  USBD_MIDI_Setup,
  USBD_MIDI_EP0_TxReady,  
  USBD_MIDI_EP0_RxReady,
  USBD_MIDI_DataIn,
  USBD_MIDI_DataOut,
  USBD_MIDI_SOF,
  USBD_MIDI_IsoINIncomplete,
  USBD_MIDI_IsoOutIncomplete,      
  USBD_MIDI_GetCfgDesc,
  USBD_MIDI_GetCfgDesc, 
  USBD_MIDI_GetCfgDesc,
  USBD_MIDI_GetDeviceQualifierDesc,
};

/* USB AUDIO device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MIDI_CfgDesc[USB_MIDI_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration 1 */
  0x09,                                 /* bLength */
  USB_DESC_TYPE_CONFIGURATION,          /* bDescriptorType */
  LOBYTE(USB_MIDI_CONFIG_DESC_SIZ),    /* wTotalLength  109 bytes*/
  HIBYTE(USB_MIDI_CONFIG_DESC_SIZ),      
  0x02,                                 /* bNumInterfaces */
  0x01,                                 /* bConfigurationValue */
  0x00,                                 /* iConfiguration */
  0xC0,                                 /* bmAttributes  BUS Powred*/
  0x32,                                 /* bMaxPower = 100 mA*/
  /* 09 byte*/
  
  /* USB Speaker Standard interface descriptor */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType */
  0x00,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOCONTROL,          /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/
  
  /* USB Speaker Class-specific AC Interface Descriptor */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_HEADER,                 /* bDescriptorSubtype */
  0x00,          /* 1.00 */             /* bcdADC */
  0x01,
  0x09,                                 /* wTotalLength = 9*/
  0x00,
  0x01,                                 /* bInCollection */
  0x01,                                 /* baInterfaceNr */
  /* 09 byte*/
  
  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwith */
  /* Interface 1, Alternate Setting 0                                             */
  AUDIO_INTERFACE_DESC_SIZE,  /* bLength */
  USB_DESC_TYPE_INTERFACE,        /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x02,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_MIDISTREAMING,        /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/
  
  /* Removed - 9bytes */
  
  /* USB Speaker Audio Streaming Interface Descriptor */
  AUDIO_STREAMING_INTERFACE_DESC_SIZE,  /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  MIDI_STREAMING_GENERAL,               /* bDescriptorSubtype */
  0x00,          /* 1.00 */             /* bcdMSC */
  0x01,                               
  LOBYTE(MIDI_DESCRIPTOR_SIZE),         /* wTotalLength = 65 */
  HIBYTE(MIDI_DESCRIPTOR_SIZE),
  /* ^^^^^ Total size of the remaining descriptors including this one */
  /* 07 byte*/
  
  /* MIDI IN descriptor */
  MIDI_IN_JACK_DESC_SIZE,             /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,    /* bDescriptorType */
  MIDI_IN_JACK_SUBTYPE,               /* bDescriptorSubtype */
  MIDI_JACK_EMBEDDED,                 /* bJackType */
  MIDI_IN_JACK_ID,                    /* bJackID */
  0,                                  /* iJack */
  /* 06 byte */
  
  /* MIDI IN descriptor */
  MIDI_IN_JACK_DESC_SIZE,             /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,    /* bDescriptorType */
  MIDI_IN_JACK_SUBTYPE,               /* bDescriptorSubtype */
  MIDI_JACK_EXTERNAL,                 /* bJackType */
  MIDI_IN_JACK_ID_EXTERNAL,           /* bJackID */
  0,
  /* 06 byte */
  
  /* MIDI OUT descriptor */
  MIDI_OUT_JACK_DESC_SIZE(1),         /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,    /* bDescriptorType */
  MIDI_OUT_JACK_SUBTYPE,              /* bDescriptorSubtype */
  MIDI_JACK_EMBEDDED,                 /* bJackType */
  MIDI_OUT_JACK_ID,                   /* bJackID */
  1,                                  /* bNrInputPins */
  MIDI_IN_JACK_ID_EXTERNAL,           /* BaSourceID(1) */
  0x01,                               /* BaSourcePin(1) */
  0,                                  /* iJack */
  /* 09 byte */
  
  /* MIDI OUT descriptor */
  MIDI_OUT_JACK_DESC_SIZE(1),         /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,    /* bDescriptorType */
  MIDI_OUT_JACK_SUBTYPE,              /* bDescriptorSubtype */
  MIDI_JACK_EXTERNAL,                 /* bJackType */
  MIDI_OUT_JACK_ID_EXTERNAL,          /* bJackID */
  1,                                  /* bNrInputPins */
  MIDI_IN_JACK_ID,                    /* BaSourceID(1) */
  0x01,                               /* BaSourcePin(1) */
  0,                                  /* iJack */
  /* 09 byte */
  
  /* OUT endpoint */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,  /* bLength */
  USB_DESC_TYPE_ENDPOINT,             /* bDescriptorType */
  MIDI_OUT_EP,                        /* bEndpointAddress */
  0x02,                               /* bmAttribute = BULK */
  LOBYTE(MIDI_OUT_PACKET),
  HIBYTE(MIDI_OUT_PACKET),
  0,
  0,
  0,
  /* 09 byte */
  
  /* OUT CS endpoint */
  MIDI_STREAMING_ENDPOINT_DESC_SIZE,
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,
  MIDI_STREAMING_GENERAL,
  1,
  MIDI_IN_JACK_ID,
  /* 05 byte */
  
  /* IN endpoint */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,  /* bLength */
  USB_DESC_TYPE_ENDPOINT,             /* bDescriptorType */
  MIDI_IN_EP,                         /* bEndpointAddress */
  0x02,                               /* bmAttribute = BULK */
  LOBYTE(MIDI_IN_PACKET),
  HIBYTE(MIDI_IN_PACKET),
  0,
  0,
  0,
  /* 09 byte */
  
  /* IN CS endpoint */
  MIDI_STREAMING_ENDPOINT_DESC_SIZE,
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,
  MIDI_STREAMING_GENERAL,
  1,
  MIDI_OUT_JACK_ID,
  /* 05 byte */
} ;

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MIDI_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END=
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */ 

/** @defgroup USBD_MIDI_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_MIDI_Init
  *         Initialize the AUDIO interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_MIDI_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx)
{
  USBD_MIDI_HandleTypeDef   *haudio;
  
  /* Open EP OUT */
  USBD_LL_OpenEP(pdev,
                 MIDI_OUT_EP,
                 USBD_EP_TYPE_BULK,
                 MIDI_OUT_PACKET);
                
  /* Open EP IN */
  USBD_LL_OpenEP(pdev,
                 MIDI_IN_EP,
                 USBD_EP_TYPE_BULK,
                 MIDI_IN_PACKET);
  
  /* Allocate Audio structure */
  pdev->pClassData = USBD_malloc(sizeof (USBD_MIDI_HandleTypeDef));
  
  if(pdev->pClassData == NULL)
  {
    return USBD_FAIL; 
  }
  else
  {
    haudio = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
    haudio->alt_setting = 0;
    haudio->wr_ptr = 0; 
    haudio->rd_ptr = 0;  
    haudio->tx_state = 0;
    haudio->tx_connection = MIDI_TX_UNKNOWN;
    haudio->tx_timeout = 0;
    
    /* Initialize the Audio output Hardware layer */
    if (((USBD_MIDI_ItfTypeDef *)pdev->pUserData)->Init(0) != USBD_OK)
    {
      return USBD_FAIL;
    }
    
    /* Prepare Out endpoint to receive 1st packet */ 
    USBD_LL_PrepareReceive(pdev,
                           MIDI_OUT_EP,
                           (uint8_t*)haudio->rx_buffer,                        
                           MIDI_OUT_PACKET);      
  }
  return USBD_OK;
}

/**
  * @brief  USBD_MIDI_Init
  *         DeInitialize the AUDIO layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_MIDI_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx)
{
  
  /* Close EP OUT */
  USBD_LL_CloseEP(pdev,
              MIDI_OUT_EP);
              
  /* Close EP IN */            
  USBD_LL_CloseEP(pdev,
              MIDI_IN_EP);

  /* DeInit  physical Interface components */
  if(pdev->pClassData != NULL)
  {
   ((USBD_MIDI_ItfTypeDef *)pdev->pUserData)->DeInit(0);
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }
  
  return USBD_OK;
}

/**
  * @brief  USBD_MIDI_Setup
  *         Handle the AUDIO specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_MIDI_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req)
{
  USBD_MIDI_HandleTypeDef   *haudio;
  uint8_t ret = USBD_OK;
  haudio = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :  
    switch (req->bRequest)
    {
    default:
      USBD_CtlError (pdev, req);
      ret = USBD_FAIL; 
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&(haudio->alt_setting),
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      if ((uint8_t)(req->wValue) <= USBD_MAX_NUM_INTERFACES)
      {
        haudio->alt_setting = (uint8_t)(req->wValue);
      }
      else
      {
        /* Call the error management function (command will be nacked */
        USBD_CtlError (pdev, req);
      }
      break;      
      
    default:
      USBD_CtlError (pdev, req);
      ret = USBD_FAIL;     
    }
  }
  return ret;
}


/**
  * @brief  USBD_MIDI_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_MIDI_GetCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_MIDI_CfgDesc);
  return USBD_MIDI_CfgDesc;
}

/**
  * @brief  USBD_MIDI_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_MIDI_DataIn (USBD_HandleTypeDef *pdev, 
                              uint8_t epnum)
{
  USBD_MIDI_HandleTypeDef   *haudio;
  USBD_MIDI_ItfTypeDef      *haudio_itf;
  haudio_itf = (USBD_MIDI_ItfTypeDef*)pdev->pUserData;
  haudio = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
  
  if (haudio != NULL && (epnum & 0x7F) == (MIDI_IN_EP & 0x7F))
  {
      haudio->tx_state = 0;
      haudio->tx_timeout = 0;
      if(haudio->tx_connection != MIDI_TX_CONNECTED){
        haudio->tx_connection = MIDI_TX_CONNECTED;
        haudio_itf->StatusTX(MIDI_TX_CONNECTED);
      }
      USBD_MIDI_SetupTransfer(pdev);
  }
  return USBD_OK;
}

/**
  * @brief  USBD_MIDI_EP0_RxReady
  *         handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_MIDI_EP0_RxReady (USBD_HandleTypeDef *pdev)
{
  return USBD_OK;
}
/**
  * @brief  USBD_MIDI_EP0_TxReady
  *         handle EP0 TRx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_MIDI_EP0_TxReady (USBD_HandleTypeDef *pdev)
{
  /* Only OUT control data are processed */
  return USBD_OK;
}
/**
  * @brief  USBD_MIDI_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_MIDI_SOF (USBD_HandleTypeDef *pdev)
{
  USBD_MIDI_HandleTypeDef   *haudio;
  USBD_MIDI_ItfTypeDef      *haudio_itf;
  haudio_itf = (USBD_MIDI_ItfTypeDef*)pdev->pUserData;
  haudio = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
  
  if (haudio != NULL && haudio->tx_state != 0)
  {
      haudio->tx_timeout++;
      if(haudio->tx_timeout > 10)  USBD_MIDI_FlushTX(pdev);

        if(haudio->tx_connection != MIDI_TX_DISCONNECTED){
          haudio->tx_connection = MIDI_TX_DISCONNECTED;
          haudio_itf->StatusTX(MIDI_TX_DISCONNECTED);
      }
  }
  return USBD_OK;
}

static void USBD_MIDI_FlushTX(USBD_HandleTypeDef *pdev){
  USBD_MIDI_HandleTypeDef   *haudio;
  haudio = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
  USBD_LL_FlushEP(pdev,MIDI_IN_EP);
  haudio->rd_ptr = haudio->wr_ptr;
  haudio->tx_state = 0;
  
  /* F0 USB specific - stop TX EP */
  PCD_HandleTypeDef* hpcd = (PCD_HandleTypeDef*)(pdev->pData);
  PCD_SET_EP_TX_STATUS(hpcd->Instance, MIDI_IN_EP & 0x7F, USB_EP_TX_NAK);
  //USBD_LL_Transmit(pdev, MIDI_IN_EP, (uint8_t*)haudio->tx_buffer_rd, 0);
}

static void USBD_MIDI_SetupTransfer(USBD_HandleTypeDef *pdev){
  USBD_MIDI_HandleTypeDef   *haudio;
  haudio = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
  if(haudio && haudio->tx_state == 0 && haudio->rd_ptr != haudio->wr_ptr){
    uint16_t i = 0;
    uint16_t new_rd_ptr = 0;
    for(i = 0; i < MIDI_TOTAL_BUF_SIZE;++i){
      haudio->tx_buffer_rd[i] = haudio->tx_buffer_wr[haudio->rd_ptr];
    
      new_rd_ptr = haudio->rd_ptr+1;
      
      if(new_rd_ptr >= MIDI_TOTAL_BUF_SIZE){ 
        new_rd_ptr = 0;
      }
      haudio->rd_ptr = new_rd_ptr;
      if(haudio->rd_ptr == haudio->wr_ptr){
        ++i;
        break;
      }
    }
    
    haudio->tx_state = 1;
    USBD_LL_Transmit(pdev, MIDI_IN_EP, (uint8_t*)haudio->tx_buffer_rd, i*4);
  }
}

/**
  * @brief  USBD_MIDI_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_MIDI_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  return USBD_OK;
}
/**
  * @brief  USBD_MIDI_IsoOutIncomplete
  *         handle data ISO OUT Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_MIDI_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  return USBD_OK;
}
/**
  * @brief  USBD_MIDI_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_MIDI_DataOut (USBD_HandleTypeDef *pdev, 
                              uint8_t epnum)
{
  USBD_MIDI_HandleTypeDef   *haudio;
  USBD_MIDI_ItfTypeDef      *haudio_itf;
  haudio_itf = (USBD_MIDI_ItfTypeDef*)pdev->pUserData;
  haudio = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
  
  if (epnum == MIDI_OUT_EP)
  {
    
    uint8_t message_count = USBD_LL_GetRxDataSize (pdev, epnum);
    
    if((message_count & 0x3) == 0){
      uint8_t i;
      message_count >>= 2;
      for(i = 0;i < message_count;++i){
        haudio_itf->EventOUT(haudio->rx_buffer[i]);
      }
    }
    
    /* Prepare Out endpoint to receive next audio packet */
    USBD_LL_PrepareReceive(pdev,
                           MIDI_OUT_EP,
                           (uint8_t*)haudio->rx_buffer, 
                           MIDI_OUT_PACKET);  
      
  }
  
  return USBD_OK;
}

/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_MIDI_GetDeviceQualifierDesc (uint16_t *length)
{
  *length = sizeof (USBD_MIDI_DeviceQualifierDesc);
  return USBD_MIDI_DeviceQualifierDesc;
}

/**
* @brief  USBD_MIDI_RegisterInterface
* @param  fops: Audio interface callback
* @retval status
*/
uint8_t  USBD_MIDI_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                        USBD_MIDI_ItfTypeDef *fops)
{
  if(fops != NULL)
  {
    pdev->pUserData= fops;
  }
  return 0;
}

uint8_t  USBD_MIDI_SendEvent(USBD_HandleTypeDef   *pdev, USBD_MIDI_MessageTypeDef event){
  USBD_MIDI_HandleTypeDef   *haudio;
  haudio = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
  if(haudio){
    uint16_t new_wr_ptr = haudio->wr_ptr + 1;
    if(new_wr_ptr >= MIDI_TOTAL_BUF_SIZE) new_wr_ptr = 0;
    if(new_wr_ptr != haudio->rd_ptr){
      MIDI_SET_USB_CIN(event, MIDI_GET_EVENT_ID(event) >> 4);
      haudio->tx_buffer_wr[haudio->wr_ptr] = event;
      haudio->wr_ptr = new_wr_ptr;
      
      __DSB();
      USBD_MIDI_SetupTransfer(pdev);
      
      return USBD_OK;
    }
  }
  return USBD_BUSY;
}
/**
  * @}
  */ 


/**
  * @}
  */ 


/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
