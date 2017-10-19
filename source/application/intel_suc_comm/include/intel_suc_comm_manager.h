/**===========================================================================
 * @file intel_suc_comm_manager.h
 *
 * @brief It is Intel SuC working functions, structures for realizing the UART
 * communication between Intel and SuC.
 *
 * @author gupta@design-shift.com
 *
 ============================================================================
 *
 * Copyright © Design SHIFT, 2017-2018
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright.
 *     * Neither the name of the [ORWL] nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY DESIGN SHIFT ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL DESIGN SHIFT BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ============================================================================
 *
 */

#ifndef __INTEL_SUC_COM_H__
#define __INTEL_SUC_COM_H__
#include <Intel_SuC_Comm.h>   /**< Common definition file between Intel and SuC development*/


/* Create event flags for the data buffer */

#define intelSESSION_RX_ACK		(1<<0)  /**< Acknowledgement for receive session */
#define intelSESSION_TX			(1<<1)  /**< Transmit session */
#define intelSESSION_TX_SUCCESS		(1<<2)  /**< Success of transmit session */
#define intelSESSION_TX_FAIL		(1<<3)  /**< Failure of transmit session */
#define intelSESSION_RX_ERROR		(1<<4)  /**< Receive error */
#define intelSESSION_RX_NACK		(1<<5)  /**< No acknowledgement for receive session */
#define intelSESSION_TX_RX_COM_FAIL	(1<<6)  /**< Event bit for tx and rx failure */
#define intelSESSION_AUTH_FAIL		(1<<7)  /**< Event bit for authentication failure */
#define intelSESSION_INVALID_COMMAND	(1<<8)  /**< Event bit for invalid command from Intel */

/** Maximum possible packet size - 255+ len + chksum + 0xFE + 0xDF + pktype */
#define intelSUC_BUFF_RX_SIZE		(255 + 5)
#define intelHW_VERSION			(0x2)/**< hwVer - PCBA HW Version for SuC Config */
#define intelSUC_SW_MAJOR_RELEASE_NO	(VERSION_MAJOR)/**< swVerMajor-SuC SW Major Release number */
#define intelSUC_SW_MINOR_RELEASE_NO	(VERSION_MINOR)/**< swVerMinor - SuC SW Minor Release number.*/
#define intelPRE_AMBLE_SIZE		(5)  /**< Max size of pre amble bit */
#define intelPIN_DATA_SIZE		(6)  /**< Size of Pin */
#define intelSEND_TIMEOUT		(-1) /**< send timeout error */
#define intelSEND_SUCCESS		(0)  /**< send success */
#define intelSEND_FAIL			(1)  /**< send failure */
#define intelDATE_TIME_ARRAY_SIZE	(20) /**< size of the array to store date and time */
#define intelDELAY_MS			(10)  /**< delay */
#define intelMAX_KEYFOB_NAME_LEN	(20) /**< Maximum length of keyfob name */
#define intelMAX_TAMPER_LOG_SIZE	(8)  /**< Maximum size of tamper log */
#define intelMAX_DATE_TIME_SIZE		(20) /**< Maximum size of date & time */

typedef enum
{
	eINIT_STATE = 0x01,	/**< Initial state */
	eSOF_STATE,		/**< Start of frame state */
	ePACKET_TYPE_STATE,	/**< It describes the type of packets*/
	eLENGTH_STATE ,		/**< Length state */
	eCHECKSUM_STATE,	/**< Checksum state */
	ePAYLOAD_STATE,		/**< Payload state */
	ePAYLOAD_COMPLETE,	/**< Payload complete state */
	eACK_NACK_STATE		/**< Acknowledgement and not acknowledgement state */
} eORWLDataLinkPacketReceiveState;

typedef union
{
	/* Maximum Data size for raw buffer data reception */
	uint8_t ucRawData[255] ;

	/* Overloading structures */
	BiosSucAction_t action ;
	BiosSuc1B_t actWith1Byte ;
	BiosSucActionWithData_t actWithData ;

}uDataBufferMapping;

typedef struct
{
    eORWLDataLinkPacketReceiveState eState ;	/**<enum for device state **/
    uint8_t ucPtr ;				/**<Index to the buffer */
    uint8_t ucLen ;				/**<Length of buffer */
    uint8_t ucChksum ;				/**<checksum for error checking */
    uDataBufferMapping xBuff ;			/**<variable of uDataBufferMapping */
}xUartBuffer_t;

/** Structure for uart rx buffer */
typedef struct
{
   uint16_t usInPtr;
   uint16_t usOutPtr;
   uint16_t usByteFree;
   uint8_t ucBuff[intelSUC_BUFF_RX_SIZE];
} xSplRxUartBuffer_t;

typedef int32_t (*tpfnCommand)(void);

typedef struct
{
    uint8_t ucPktType;		/**< type of packet */
    tpfnCommand cmd_read;	/**< command for reading data */
    tpfnCommand cmd_write;	/**< command for writing data */
}xIntelSucCommHandling;

/* Variables of structure */
extern EventGroupHandle_t xUartTxRXSync ;
extern xUartBuffer_t xTxBuffer ;

/* Function declaration */

/**
 * @brief for sending packet.
 *
 * This function is used for sending packet. This is reentrant function
 * and any task can call the same.
 *
 * @param pucMsg is the pointer to the msg to be sent.
 * @param pucLen is the length of msg.
 * @param pxTimeout is the time to wait for semaphore.
 *
 * @return error code
 */
int32_t lIntelSendPacket(uint8_t *pucMsg, uint8_t *pucLen, TickType_t * pxTimeout );

/**
 * @brief Interrupt service routine
 *
 * This isr is used to check if there is some data received
 * or rx error interrupt received by reading interrupt status reg.
 *
 * @return void
 */
void vUartTxRx( void );

/**
 * @brief For processing the receive data
 *
 * This function is used for checking the received data is right or not,
 * if right than process that according to state.
 *
 * @return void
 */
void vIntelProcessRxPacket( void );

/**
 * @brief For transmitting the data
 *
 * This function is is used for transmitting the data on UART.
 *
 * @param pucMsg is pointer to data to be transmitted
 * @param ucLen is the size of data
 *
 * @return size of the transmitted data
 */
uint8_t ucIntelTxUartData (uint8_t *pucMsg, uint8_t ucLen);

/**
 * @brief Used for initialization of UART
 *
 * This function is used for initialization of packets and Initialize the
 * data link layer of IntelSuC Communication. i.e. UART
 *
 * @return error code
 */
int32_t lIntelSucComInit( void );

#endif /*__INTEL_SUC_COM_H__ */
