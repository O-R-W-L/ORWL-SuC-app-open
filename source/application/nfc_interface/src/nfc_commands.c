/**===========================================================================
 * @file nfc_commands.c
 *
 * @brief This file contains the internal implementation all ORWL commands to
 * communicate with remote Tag/Card. All ISO1816 command communication with the
 * peer is initiated at this layer. Application has to decide the calling
 * sequence and call the right API's.
 *
 * @author viplav.roy@design-shift.com
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
 *
 ============================================================================
 *
 */

/* nfc interface header */
#include <nfc_interface.h>

/* nfc commands header */
#include <nfc_commands.h>
#include <nfc_common.h>

/* error messages header */
#include <errors.h>
#include <events.h>

/**
 * Key for encrypting/decrypting the message
 */
uint8_t gucPlain[commandsCONFIRMSSK_SIZE] = {0x4f, 0x52, 0x57, 0x4c, 0x4b, 0x45, 0x59, 0x46};

/* For encryption of data 8 bytes are needed so appending 2 bytes of zero */
uint8_t gucCVMPIN[commandsCVM_ACTUAL_SIZE+commandsCVM_PAD_LEN] = {0x1,0x2,0x3,0x4,0x5,0x6,0x0,0x0};

uint32_t ulNfcCmdSelectMasterApplet( void )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* Master Applet identifier definition */
	uint8_t ucMasterApplet[commandsMASTER_APPLET_LEN] = {
			commandsMASTER_APPLET_BYTE0, commandsMASTER_APPLET_BYTE1,
			commandsMASTER_APPLET_BYTE2, commandsMASTER_APPLET_BYTE3,
			commandsMASTER_APPLET_BYTE4, commandsMASTER_APPLET_BYTE5,
			commandsMASTER_APPLET_BYTE6, commandsMASTER_APPLET_BYTE7};

	/* Populate the DESFIRE structure with appropriate command */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand =
			MFDF_Iso_SelectFile;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.Select_File.bSelector =
			commandsSelect_FILE_BY_NAME;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.Select_File.bOption =
			commandsIGNORE_STORED_FCI;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.Select_File.pFid =
			commandsSELECT_FID;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.Select_File.pDFname =
			ucMasterApplet;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.Select_File.bDFnameLen =
			commandsMASTER_APPLET_LEN;

	/* initiate transfer */
	return phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsMASTER_APPLET_LEN );
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdSelectUserApplet( void )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* User Applet identifier definition */
	uint8_t ucUserApplet[commandsUSER_APPLET_LEN] = {
	commandsUSER_APPLET_BYTE0, commandsUSER_APPLET_BYTE1,
	commandsUSER_APPLET_BYTE2, commandsUSER_APPLET_BYTE3,
	commandsUSER_APPLET_BYTE4, commandsUSER_APPLET_BYTE5 };

	/* Populate the DESFIRE structure with appropriate command */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand =
			MFDF_Iso_SelectFile;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.Select_File.bSelector =
			commandsSelect_FILE_BY_NAME;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.Select_File.bOption =
			commandsIGNORE_STORED_FCI;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.Select_File.pFid =
			commandsSELECT_FID;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.Select_File.pDFname =
			ucUserApplet;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.Select_File.bDFnameLen =
			commandsUSER_APPLET_LEN;

	/* initiate transfer */
	return phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsUSER_APPLET_LEN );
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdReadRawData( uint8_t *pucReadBuf, uint32_t *pulReadLen )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* command buffer */
	uint8_t ucCmdBuf;

	/* status byte */
	uint32_t ulStatus;

	/* set if more than 256 Bytes of data expected */
	uint16_t usMoreDataAvailable = 0;

	/* validate input buffers */
	if( pucReadBuf == NULL )
	{
		debugERROR_PRINT(" Invalid parameters Received \n");
		return COMMON_ERR_NULL_PTR;
	}

	/* Send Raw Read command */
	ucCmdBuf = eCMD_READ_DATA;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand = MFDF_Raw_Comm;

	/* Length of Data in the command buffer */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.bBufLen = 0;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.pBuf = &ucCmdBuf;

	/* send data */
	ulStatus = phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsRAW_ACCESS_CMD_LEN );
	if( !ulStatus )
	{
		debugPRINT_NFC("MFDF_Read_Raw_Data command sent \n");
	}
	else
	{
		debugERROR_PRINT("Failed to send MFDF_Read_Raw_Data command "
						"ulStatus = 0x%x \n",ulStatus);
		return ulStatus;
	}

	/*The data received over the above command can be retrieved by calling the receive */
	/* wNumberofBytes first as input tells the max supported recieve size and then as out tells the actual number of data bytes received */
	/* wNumberofBytes will vary depending upon the card layout*/
	ulStatus = phNfcLib_Receive(pucReadBuf, pulReadLen, &usMoreDataAvailable);
#if DBG_DATA
	uint32_t counter0 = 0;
	debugPRINT_NFC("*pulReadLen = %d \n",*pulReadLen);
	for (counter0 = 0; counter0 < *pulReadLen; counter0++)
	{
		debugPRINT_NFC(" ucReadBuf[%d] = 0x%x\n",counter0, pucReadBuf[counter0]);
	}
#endif
	return ulStatus;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdReadPublicKey( uint8_t *pucReadBuf, uint32_t *pulReadLen )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* command buffer */
	uint8_t ucCmdBuf;

	/* status byte */
	uint32_t ulStatus;

	/* set if more than 256 Bytes of data expected */
	uint16_t usMoreDataAvailable = 0;

	/* validate input buffers */
	if( pucReadBuf == NULL )
	{
		debugERROR_PRINT(" Invalid parameters Received \n");
		return COMMON_ERR_NULL_PTR;
	}

	/* Send Raw Read command */
	ucCmdBuf = eCMD_GET_PUB_KEY;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand = MFDF_Raw_Comm;

	/* Length of Data in the command buffer */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.bBufLen = 0;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.pBuf = &ucCmdBuf;

	/* send data */
	ulStatus = phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsRAW_ACCESS_CMD_LEN );
	if( !ulStatus )
	{
		debugPRINT_NFC("MFDF_Read_Public_key command sent \n");
	}
	else
	{
		debugERROR_PRINT("Failed to send MFDF_Read_Public_key command "
					"ulStatus = 0x%x \n",ulStatus);
		return ulStatus;
	}

	ulStatus = phNfcLib_Receive(pucReadBuf, pulReadLen, &usMoreDataAvailable);
#if DBG_DATA
	uint32_t counter0 = 0;
	debugPRINT_NFC("ulReadLen = %d \n",*pulReadLen);
	for (counter0 = 0; counter0 < *pulReadLen; counter0++)
	{
		debugPRINT_NFC(" ucReadBuf[%d] = 0x%x\n",counter0, pucReadBuf[counter0]);
	}
#endif
	return ulStatus;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdGetKeyFobChallenge( uint8_t *pucReadBuf, uint32_t *pulReadLen )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* command buffer */
	uint8_t ucCmdBuf;

	/* status byte */
	uint32_t ulStatus;

	/* set if more than 256 Bytes of data expected */
	uint16_t usMoreDataAvailable = 0;

	/* validate input buffers */
	if( pucReadBuf == NULL )
	{
		debugERROR_PRINT(" Invalid parameters Received \n");
		return COMMON_ERR_NULL_PTR;
	}

	/* Send Raw Read command */
	ucCmdBuf = eCMD_GET_SEED;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand = MFDF_Raw_Comm;

	/* Length of Data in the command buffer */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.bBufLen = 0;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.pBuf = &ucCmdBuf;

	/* send data */
	ulStatus = phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsRAW_ACCESS_CMD_LEN );
	if( !ulStatus )
	{
		debugPRINT_NFC("MFDF_Get_Peer_Challenge command sent \n");
	}
	else
	{
		debugERROR_PRINT("Failed to send MFDF_Get_Peer_Challenge command"
					"ulStatus = 0x%x \n",ulStatus);
		return ulStatus;
	}

	ulStatus = phNfcLib_Receive(pucReadBuf, pulReadLen, &usMoreDataAvailable);
#if DBG_DATA
	uint32_t counter0 = 0;
	debugPRINT_NFC("ulReadLen = %d \n",*pulReadLen);
	for (counter0 = 0; counter0 < *pulReadLen; counter0++)
	{
		debugPRINT_NFC(" ucReadBuf[%d] = 0x%x\n",counter0, pucReadBuf[counter0]);
	}
#endif
	return ulStatus;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdGetName( uint8_t *pucReadBuf, uint32_t *pulReadLen )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* command buffer */
	uint8_t ucCmdBuf;

	/* status byte */
	uint32_t ulStatus;

	/* set if more than 256 Bytes of data expected */
	uint16_t usMoreDataAvailable = 0;

	/* validate input buffers */
	if( pucReadBuf == NULL )
	{
		debugERROR_PRINT(" Invalid parameters Received \n");
		return COMMON_ERR_NULL_PTR;
	}

	/* Send Raw Read command */
	ucCmdBuf = eCMD_GET_NAME;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand = MFDF_Raw_Comm;

	/* Length of Data in the command buffer */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.bBufLen = 0;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.pBuf = &ucCmdBuf;

	/* send data */
	ulStatus = phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsRAW_ACCESS_CMD_LEN );
	if( !ulStatus )
	{
		debugPRINT_NFC("MFDF_Get_Peer_Name command sent \n");
	}
	else
	{
		debugERROR_PRINT("Failed to send MFDF_Get_Peer_Name command"
					"ulStatus = 0x%x \n",ulStatus);
		return ulStatus;
	}

	ulStatus = phNfcLib_Receive(pucReadBuf, pulReadLen, &usMoreDataAvailable);
#if DBG_RECEIVED_DATA
	uint32_t counter0 = 0;
	debugPRINT_NFC("ulReadLen = %d \n",*pulReadLen);
	for (counter0 = 0; counter0 < *pulReadLen; counter0++)
	{
		debugPRINT_NFC(" ucReadBuf[%d] = 0x%x\n",counter0, pucReadBuf[counter0]);
	}
#endif
	return ulStatus;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdGetSerial( uint8_t *pucReadBuf, uint32_t *pulReadLen )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* command buffer */
	uint8_t ucCmdBuf;

	/* status byte */
	uint32_t ulStatus;

	/* set if more than 256 Bytes of data expected */
	uint16_t usMoreDataAvailable = 0;

	/* validate input buffers */
	if((pucReadBuf == NULL) || (pulReadLen == NULL))
	{
		debugERROR_PRINT(" Invalid parameters Received \n");
		return COMMON_ERR_INVAL;
	}

	/* Send Raw Read command */
	ucCmdBuf = eCMD_GET_SERIAL;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand = MFDF_Raw_Comm;

	/* Length of Data in the command buffer */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.bBufLen = 0;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.pBuf = &ucCmdBuf;

	/* send data */
	ulStatus = phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsRAW_ACCESS_CMD_LEN );
	if( !ulStatus )
	{
		debugPRINT_NFC("MFDF_GET_SERIAL command sent \n");
	}
	else
	{
		debugERROR_PRINT("Failed to send MFDF_GET_SERIAL command "
						"ulStatus = 0x%x \n",ulStatus);
		return ulStatus;
	}
	ulStatus = phNfcLib_Receive(pucReadBuf, pulReadLen, &usMoreDataAvailable);
	return ulStatus;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdKeyAssociate( void )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* command buffer */
	uint8_t ucCmdBuf;

	/* status byte */
	uint32_t ulStatus;

	/* Send Key association command */
	ucCmdBuf = eCMD_ASSOCIATION_REQUEST;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand = MFDF_Raw_Comm;

	/* Length of Data in the command buffer */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.bBufLen = 0;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.pBuf = &ucCmdBuf;

	/* send data */
	ulStatus = phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsRAW_ACCESS_CMD_LEN );
	if( !ulStatus )
	{
		debugPRINT_NFC("MFDF_Key_Association command sent \n");
	}
	else
	{
		debugERROR_PRINT("Failed to send MFDF_Key_Association command "
					"ulStatus = 0x%x \n",ulStatus);
	}
	return ulStatus;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdWriteRawData( uint8_t *pucWriteBuf, uint32_t ulWriteLen )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* command buffer */
	uint8_t ucCmdBuf[commandsRAW_ACCESS_BUF_LEN]={0};

	/* status byte */
	uint32_t ulStatus;

	uint8_t ucCount = 0 ;

	/* validate input buffers */
	if( pucWriteBuf == NULL )
	{
		debugERROR_PRINT(" Invalid parameters Received \n");
		return COMMON_ERR_NULL_PTR;
	}

	/* Send Write command */
	ucCmdBuf[ucCount] = eCMD_WRITE_DATA;
	for(ucCount = 0; ucCount < ulWriteLen; ucCount++)
	{
		ucCmdBuf[ucCount+1] = pucWriteBuf[ucCount];
	}

	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand = MFDF_Raw_Comm;
	/* Length of Data in the command buffer */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.bBufLen = ulWriteLen;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.pBuf = ucCmdBuf;

	/* send data */
	ulStatus = phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsRAW_ACCESS_CMD_LEN );
	if( !ulStatus )
	{
		debugPRINT_NFC("MFDF_Write_Data command sent \n");
	}
	else
	{
		debugERROR_PRINT("Failed to send MFDF_Write_Data command "
					"ulStatus = 0x%x \n",ulStatus);
	}
	return ulStatus;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdWritePublicKey( uint8_t *pucWriteBuf, uint32_t ulWriteLen )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* command buffer */
	uint8_t ucCmdBuf[commandsRAW_ACCESS_BUF_LEN]={0};

	/* status byte */
	uint32_t ulStatus;

	uint8_t ucCount = 0 ;

	/* validate input buffers */
	if( pucWriteBuf == NULL )
	{
		debugERROR_PRINT(" Invalid parameters Received \n");
		return COMMON_ERR_NULL_PTR;
	}

	/* Send Write command */
	ucCmdBuf[ucCount] = eCMD_WRITE_PUB_KEY;
	for(ucCount = 0; ucCount < ulWriteLen; ucCount++)
	{
		ucCmdBuf[ucCount+1] = pucWriteBuf[ucCount];
	}

	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand = MFDF_Raw_Comm;
	/* Length of Data in the command buffer */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.bBufLen = ulWriteLen;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.pBuf = ucCmdBuf;

	/* send data */
	ulStatus = phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsRAW_ACCESS_CMD_LEN );
	if( !ulStatus )
	{
		debugPRINT_NFC("MFDF_Write_Public_Key command sent \n");
	}
	else
	{
		debugERROR_PRINT("Failed to send MFDF_Write_Public_Key command "
					"ulStatus = 0x%x \n",ulStatus);
	}
	return ulStatus;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdSaveKeyFobName( uint8_t *pucWriteBuf, uint32_t ulWriteLen )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* command buffer */
	uint8_t ucCmdBuf[commandsRAW_ACCESS_BUF_LEN]={0};

	/* status byte */
	uint32_t ulStatus;

	uint8_t ucCount = 0 ;

	/* validate input buffers */
	if( pucWriteBuf == NULL )
	{
		debugERROR_PRINT(" Invalid parameters Received \n");
		return COMMON_ERR_NULL_PTR;
	}

	/* Send Write command */
	ucCmdBuf[ucCount] = eCMD_SAVE_KEYFOB_NAME;
	for(ucCount = 0; ucCount < ulWriteLen; ucCount++)
	{
		ucCmdBuf[ucCount+1] = pucWriteBuf[ucCount];
	}

	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand = MFDF_Raw_Comm;
	/* Length of Data in the command buffer */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.bBufLen = ulWriteLen;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.pBuf = ucCmdBuf;

	/* send data */
	ulStatus = phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsRAW_ACCESS_CMD_LEN );
	if( !ulStatus )
	{
		debugPRINT_NFC("MFDF_Save_KeyFob_Name command sent \n");
	}
	else
	{
		debugERROR_PRINT("Failed to send MFDF_Save_KeyFob_Name command "
					"ulStatus = 0x%x \n",ulStatus);
	}
	return ulStatus;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdSendORWLChallenge( uint8_t *pucWriteBuf, uint32_t ulWriteLen )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* command buffer */
	uint8_t ucCmdBuf[commandsRAW_ACCESS_BUF_LEN]={0};

	/* status byte */
	uint32_t ulStatus;

	uint8_t ucCount = 0 ;

	/* validate input buffers */
	if( pucWriteBuf == NULL )
	{
		debugERROR_PRINT(" Invalid parameters Received \n");
		return COMMON_ERR_NULL_PTR;
	}

	/* Send Write command */
	ucCmdBuf[ucCount] = eCMD_SEND_SEED;
	for(ucCount = 0; ucCount < ulWriteLen; ucCount++)
	{
		ucCmdBuf[ucCount+1] = pucWriteBuf[ucCount];
	}

	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand = MFDF_Raw_Comm;
	/* Length of Data in the command buffer */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.bBufLen = ulWriteLen;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.pBuf = ucCmdBuf;

	/* send data */
	ulStatus = phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsRAW_ACCESS_CMD_LEN );
	if( !ulStatus )
	{
		debugPRINT_NFC("MFDF_Send_Challenge command sent \n");
	}
	else
	{
		debugERROR_PRINT("Failed to send MFDF_Send_Challenge command "
					"ulStatus = 0x%x \n",ulStatus);
	}
	return ulStatus;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdVerifyCVM( uint8_t *pucKey )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* command buffer */
	uint8_t ucCmdBuf[commandsRAW_ACCESS_BUF_LEN]={0};

	/* TODO: Make ucCipherdata empty, Values are for verification */
	uint8_t ucCipherdata[8]={0x96,0xc6,0x93,0x56,0x34,0xe4,0x02,0xd6};

	/* status byte */
	uint32_t ulStatus;

	uint8_t ucCount = 0 ;

	ulStatus = ucl_3des_cbc(ucCipherdata, gucCVMPIN, pucKey, gucIV,
			sizeof(gucCVMPIN), nfccommonUCL_CIPHER_ENCRYPT);
	if( ulStatus )
	{
		debugERROR_PRINT(" Failed to encrypt the message ");
		return ulStatus;
	}

	/* Send Write command */
	ucCmdBuf[ucCount] = eCMD_VERIFY_CVM;
	for(ucCount = 0; ucCount < sizeof(gucCVMPIN); ucCount++)
	{
		ucCmdBuf[ucCount+1] = ucCipherdata[ucCount];
	}

#if DBG_DATA
	for(ucCount=0;ucCount<9;ucCount++)
	{
		debugPRINT_NFC(" ucCmdBuf[%d] = 0x%02x",ucCount,ucCmdBuf[ucCount]);
	}
#endif

	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand = MFDF_Raw_Comm;
	/* Length of Data in the command buffer */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.bBufLen = sizeof(gucCVMPIN);
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.pBuf = ucCmdBuf;

	/* send data */
	ulStatus = phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsRAW_ACCESS_CMD_LEN );
	if( !ulStatus )
	{
		debugPRINT_NFC("MFDF_Write_CVM_VERIFY command sent \n");
	}
	else
	{
		debugERROR_PRINT("Failed to send MFDF_Write_CVM_VERIFY command "
					"ulStatus = 0x%x \n",ulStatus);
	}
	return ulStatus;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdConfirmSSK( uint8_t *pucData , uint32_t ulLen )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* command buffer */
	uint8_t ucCmdBuf[commandsRAW_ACCESS_BUF_LEN]={0};

	/* status byte */
	uint32_t ulStatus;

	uint8_t ucCount = 0 ;

	/* Send Write command */
	ucCmdBuf[ucCount] = eCMD_CONFIRM_SECRET_KEY;
	for(ucCount = 0; ucCount < ulLen; ucCount++)
	{
		ucCmdBuf[ucCount+1] = pucData[ucCount];
	}
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand = MFDF_Raw_Comm;
	/* Length of Data in the command buffer */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.bBufLen = ulLen;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.pBuf = ucCmdBuf;

	/* send data */
	ulStatus = phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsRAW_ACCESS_CMD_LEN );
	if( !ulStatus )
	{
		debugPRINT_NFC("MFDF_Write_SSK_VERIFY command sent \n");
	}
	else
	{
		debugERROR_PRINT("Failed to send MFDF_Write_SSK_VERIFY command "
					"ulStatus = 0x%x \n",ulStatus);
	}
	return ulStatus;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCmdSaveKey( void )
{
	/* Input buffer to hold DESFIRE data and command */
	phNfcLib_Transmit_t xphNfcLib_TransmitInput;

	/* command buffer */
	uint8_t ucCmdBuf;

	/* status byte */
	uint32_t ulStatus;

	/* Send Key association command */
	ucCmdBuf = eCMD_SAVE_KEYS;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.bCommand = MFDF_Raw_Comm;

	/* Length of Data in the command buffer */
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.bBufLen = 0;
	xphNfcLib_TransmitInput.phNfcLib_MifareDesfire.Operation.RawComm.pBuf = &ucCmdBuf;

	/* send data */
	ulStatus = phNfcLib_Transmit( &xphNfcLib_TransmitInput, commandsRAW_ACCESS_CMD_LEN );
	if( !ulStatus )
	{
		debugPRINT_NFC("MFDF_Save_Key command sent \n");
	}
	else
	{
		debugERROR_PRINT("Failed to send MFDF_Save_Key command "
					"ulStatus = 0x%x \n",ulStatus);
	}
	return ulStatus;
}
/*----------------------------------------------------------------------------*/
