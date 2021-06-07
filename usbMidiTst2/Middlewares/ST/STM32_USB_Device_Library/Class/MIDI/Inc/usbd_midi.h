/**
  ******************************************************************************
  * @file    usbd_midi.h
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   header file for the usbd_midi.c file.
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
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_MIDI_H
#define __USB_MIDI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USBD_MIDI
  * @brief This file is the Header file for usbd_midi.c
  * @{
  */ 


/** @defgroup USBD_MIDI_Exported_Defines
  * @{
  */ 
#define MIDI_OUT_EP                                  0x01
#define MIDI_IN_EP                                   0x81

#define USB_MIDI_CONFIG_DESC_SIZ                     (84+17)
#define AUDIO_INTERFACE_DESC_SIZE                     9
#define USB_MIDI_DESC_SIZ                            0x09
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE             0x09
#define MIDI_STREAMING_ENDPOINT_DESC_SIZE            0x05

#define MIDI_DESCRIPTOR_TYPE                         0x21
#define MIDI_DESCRIPTOR_SIZE                          65
#define USB_DEVICE_CLASS_AUDIO                        0x01
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01
#define AUDIO_SUBCLASS_MIDISTREAMING                 0x03
#define AUDIO_PROTOCOL_UNDEFINED                      0x00

/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE               0x24
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                0x25

#define MIDI_STREAMING_GENERAL                        0x01


#define MIDI_IN_JACK_DESC_SIZE                        0x06
#define MIDI_IN_JACK_SUBTYPE                          0x02

#define MIDI_OUT_JACK_DESC_SIZE(pins)                 (0x07 + 2*pins)
#define MIDI_OUT_JACK_SUBTYPE                         0x03

#define MIDI_JACK_EMBEDDED                            0x01
#define MIDI_JACK_EXTERNAL                            0x02

#define MIDI_IN_JACK_ID                               0x01
#define MIDI_IN_JACK_ID_EXTERNAL                      0x02
#define MIDI_OUT_JACK_ID                              0x03
#define MIDI_OUT_JACK_ID_EXTERNAL                     0x04

/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                          0x01
#define AUDIO_CONTROL_INPUT_TERMINAL                  0x02
#define AUDIO_CONTROL_OUTPUT_TERMINAL                 0x03
#define AUDIO_CONTROL_FEATURE_UNIT                    0x06

#define AUDIO_INPUT_TERMINAL_DESC_SIZE                0x0C
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE           0x07

#define AUDIO_CONTROL_MUTE                            0x0001

#define AUDIO_FORMAT_TYPE_I                           0x01
#define AUDIO_FORMAT_TYPE_III                         0x03

#define AUDIO_ENDPOINT_GENERAL                        0x01

#define AUDIO_REQ_GET_CUR                             0x81
#define AUDIO_REQ_SET_CUR                             0x01

#define AUDIO_OUT_STREAMING_CTRL                      0x02


#define AUDIO_OUT_PACKET                              (uint32_t)(((USBD_AUDIO_FREQ * 2 * 2) /1000)) 
#define AUDIO_DEFAULT_VOLUME                          70
    
/*
  Total size of the audio transfer buffer in words (word = MIDI event)
  16 x 32-bit = 64 bytes (maximum packet size)
 */
#define MIDI_TOTAL_BUF_SIZE                          ((uint32_t)(16))

#define MIDI_OUT_PACKET                               64
#define MIDI_IN_PACKET                                64  
    
/**
  * @}
  */ 


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */

typedef uint32_t USBD_MIDI_MessageTypeDef;

#define MIDI_GET_EVENT_ID(event)      ((event >> 8) & 0xF0)
#define MIDI_GET_MIDI_CHANNEL(event)  ((event >> 8) & 0x0F)

#define MIDI_GET_KEY(event)           ((event >> 16) & 0xFF)
#define MIDI_GET_VELOCITY(event)      ((event >> 24) & 0xFF)

#define MIDI_EVENT_NOTE_OFF           (0x80)
#define MIDI_EVENT_NOTE_ON            (0x90)
#define MIDI_EVENT_CC                 (0xB0)

#define MIDI_MAKE_EVENT(channel,event,p1,p2)  (((event & 0xF0) << 8) | ((channel &0xF) << 8) | (p1 << 16) | (p2 << 24))

#define MIDI_SET_USB_CIN(event, cin)  do { event = (event & 0xFFFFFFF0) | cin; } while(0)

typedef enum {
  MIDI_TX_DISCONNECTED = 0,
  MIDI_TX_CONNECTED = 1,
  MIDI_TX_UNKNOWN = 2,
}USBD_MIDI_TX_ConnectionTypeDef;

typedef struct
{
  __IO uint32_t             alt_setting; 
  USBD_MIDI_MessageTypeDef  rx_buffer[MIDI_TOTAL_BUF_SIZE];
  USBD_MIDI_MessageTypeDef  tx_buffer_rd[MIDI_TOTAL_BUF_SIZE];
  USBD_MIDI_MessageTypeDef  tx_buffer_wr[MIDI_TOTAL_BUF_SIZE];
  __IO uint16_t             rd_ptr;  
  uint16_t                  wr_ptr;  
  __IO uint8_t              tx_state;
  __IO uint8_t              tx_timeout;
  __IO USBD_MIDI_TX_ConnectionTypeDef tx_connection;
}
USBD_MIDI_HandleTypeDef; 


typedef struct
{
    int8_t  (*Init)         (uint32_t options);
    int8_t  (*DeInit)       (uint32_t options);
    int8_t  (*EventOUT)     (USBD_MIDI_MessageTypeDef event);
    int8_t  (*StatusTX)     (USBD_MIDI_TX_ConnectionTypeDef new_status);
}USBD_MIDI_ItfTypeDef;
/**
  * @}
  */ 



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */ 

extern USBD_ClassTypeDef  USBD_MIDI;
#define USBD_MIDI_CLASS    &USBD_MIDI
/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */ 
uint8_t  USBD_MIDI_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                        USBD_MIDI_ItfTypeDef *fops);
                                        
uint8_t  USBD_MIDI_SendEvent(USBD_HandleTypeDef   *pdev,USBD_MIDI_MessageTypeDef event);
/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif  /* __USB_MIDI_H */
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
