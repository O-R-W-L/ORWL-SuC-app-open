/**===========================================================================
 * @file nfc_commands.h
 *
 * @brief This file contains the NFC command interface
 * data structures and macros used for ORWL-KeyFob communication.
 * At core, all the API's are using MIFARE-DESFIRE framework which internally
 * uses ISO7816 command sets
 *
 *
 * @author ravikiran@design-shift.com
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

#ifndef NFC_COMMANDS_H
#define NFC_COMMANDS_H

/* ORWL Master Applet Identifier definitions */
#define	commandsMASTER_APPLET_LEN		(0x08)	/**< 8 bytes for master applet identification */
#define commandsMASTER_APPLET_BYTE0		(0xA0)	/**< Applet ID byte-0 */
#define commandsMASTER_APPLET_BYTE1		(0x00)	/**< Applet ID byte-1 */
#define commandsMASTER_APPLET_BYTE2		(0x00)	/**< Applet ID byte-2 */
#define commandsMASTER_APPLET_BYTE3		(0x01)	/**< Applet ID byte-3 */
#define commandsMASTER_APPLET_BYTE4		(0x51)	/**< Applet ID byte-4 */
#define commandsMASTER_APPLET_BYTE5		(0x00)	/**< Applet ID byte-5 */
#define commandsMASTER_APPLET_BYTE6		(0x00)	/**< Applet ID byte-6 */
#define commandsMASTER_APPLET_BYTE7		(0x00)	/**< Applet ID byte-7 */

/* ORWL USER Applet Identifier definitions */
#define	commandsUSER_APPLET_LEN			(0x06)	/**< 6 bytes for user applet identification */
#define commandsUSER_APPLET_BYTE0		(0xA0)	/**< Applet ID byte-0 */
#define commandsUSER_APPLET_BYTE1		(0x00)	/**< Applet ID byte-1 */
#define commandsUSER_APPLET_BYTE2		(0x00)	/**< Applet ID byte-2 */
#define commandsUSER_APPLET_BYTE3		(0x07)	/**< Applet ID byte-3 */
#define commandsUSER_APPLET_BYTE4		(0x38)	/**< Applet ID byte-4 */
#define commandsUSER_APPLET_BYTE5		(0x01)	/**< Applet ID byte-5 */

/* ORWL command selection macros */
#define commandsSelect_FILE_BY_NAME		(0x04)	/**< parameter to select the file by Name */
#define commandsSelect_FILE_BY_EID		(0x00)	/**< parameter to select the file by EFID */
#define commandsIGNORE_STORED_FCI		(0x0C)	/**< 0x0C ignore FCI and 0x00 uses stored FCI */
#define commandsSELECT_FID			NULL	/**< This filed is valid if select by FID is used.
 	 	 	 	 	 	 	    Invalid when select by name command is used
 	 	 	 	 	 	 	    ORWL uses later configurations. */

/* RAW Read Buffer configurations */
#define commandsRAW_ACCESS_BUF_LEN		(0xFF)	/**< Buffer size required for the read routine */
#define commandsRAW_ACCESS_CMD_LEN		(0x01)	/**< Buffer size required for the read routine */

#define commandsNFC_TAG_SECRET_SIZE		(32)	/**< Length of the secret key */
#define commandsMAX_DATA_SIZE			(32)	/**< Max Data Length */

#define commandsTRNG_NUM_OF_WORDS		(8)	/**< 32 byte number */
#define commandsCVM_ACTUAL_SIZE			(6)	/**< Size of CVM pin */
#define commandsCONFIRMSSK_SIZE			(8)	/**< Shared Secret Key Data Size for SSK verification*/
#define commandsKEYFOB_NAME_LENGTH		(19)	/**< Fixed Key Fob NAME Length */
#define commandsCVM_PAD_LEN			(2)	/**< 2 bytes are added to make 8 byte align */
#define commandsINIT_VECTOR_SIZE		(8)	/**< Initializing vector size */
#define commandsSHA_HASH_LENGTH			(20)	/**< SHA1 Hash length */

extern uint8_t gucIV[commandsINIT_VECTOR_SIZE];
extern uint8_t gucCVMPIN[commandsCVM_ACTUAL_SIZE+commandsCVM_PAD_LEN];

/* ORWL custom commands definition enum
 */
typedef enum
{
	eCMD_CONFIRM_SECRET_KEY		= 0x10,		/**< Command to check whether the secret keys generated
								on ORWL and KeyFob are proper */
	eCMD_GET_PUB_KEY		= 0x11,		/**< Command to read the peer's public key */
	eCMD_WRITE_PUB_KEY		= 0x12,		/**< Command to write the public key */
	eCMD_VERIFY_CVM			= 0x13,		/**< Command for CVM verification */
	eCMD_ASSOCIATION_REQUEST	= 0x14,		/**< Command request for KeyFob-ORWL association */
	eCMD_WRITE_DATA			= 0x15,		/**< Command request to write data */
	eCMD_READ_DATA			= 0x16,		/**< Command request to read data */
	eCMD_SAVE_KEYS			= 0x17,		/**< Command to save the keys */
	eCMD_SEND_SEED			= 0x18,		/**< Command to send ORWL seed */
	eCMD_GET_SEED			= 0x19,		/**< Command to read Peer's seed */
	eCMD_GET_SERIAL			= 0x20,		/**< Command to get the serial number */
	eCMD_GET_NAME			= 0x22,		/**< Command to get the peer name */
	eCMD_SAVE_KEYFOB_NAME		= 0x2C		/**< Command to store the key Fob name */

} eCmdORWLCommands_t;

/**
 * @brief This function selects the ORWL Master Applet.
 *
 * @param None
 *
 * @return status of master applet selection
 *  returns PH_NFCLIB_STATUS_SUCCESS on success
 *  returns error codes on error in selection
 */
uint32_t ulNfcCmdSelectMasterApplet( void );

/**
 * @brief This function selects the ORWL USER Applet.
 *
 * @param None
 *
 * @return status of user applet selection
 *  	return PH_NFCLIB_STATUS_SUCCESS on success
 *  	returns error codes on error in selection
 */
uint32_t ulNfcCmdSelectUserApplet( void );

/**
 * @brief This function reads RAW data from KeyFob.

 * @param pucReadBuf Pointer to buffer with contents read from the peer
 * @param pulReadLen Pointer to parameter to indicate number of bytes read
 *                   from peer
 *
 * @return Status of data read
 * returns PH_NFCLIB_STATUS_SUCCESS on success
 * returns error codes on error
 */
uint32_t ulNfcCmdReadRawData( uint8_t *pucReadBuf, uint32_t *pulReadLen );

/**
 * @brief This function reads Peer's Public Key data.

 * @param pucReadBuf Pointer to buffer with contents read from the peer
 * @param pulReadLen Pointer to parameter to indicate number of bytes read
 *                   from peer
 *
 * @return Status of Public Key read
 * returns PH_NFCLIB_STATUS_SUCCESS on success
 * returns error codes on error
 */
uint32_t ulNfcCmdReadPublicKey( uint8_t *pucReadBuf, uint32_t *pulReadLen );

/**
 * @brief This function reads Peer's challenge.

 * @param pucReadBuf Pointer to buffer with contents read from the peer
 * @param pulReadLen Pointer to parameter to indicate number of bytes read
 *                   from peer
 *
 * @return uint32_t
 * returns PH_NFCLIB_STATUS_SUCCESS on success
 * returns error codes on error
 */
uint32_t ulNfcCmdGetKeyFobChallenge( uint8_t *pucReadBuf, uint32_t *pulReadLen );

/**
 * @brief This function reads Peer's name.

 * @param pucReadBuf Pointer to buffer with contents read from the peer
 * @param pulReadLen Pointer to parameter to indicate number of bytes read
 *                   from peer
 *
 * @return uint32_t
 * returns PH_NFCLIB_STATUS_SUCCESS on success
 * returns error codes on error
 */
uint32_t ulNfcCmdGetName( uint8_t *pucReadBuf, uint32_t *pulReadLen );

/**
 * @brief This function reads Peer's serial number.

 * @param pucReadBuf Pointer to buffer with contents read from the peer
 * @param pulReadLen Pointer to parameter to indicate number of bytes read
 *                   from peer
 *
 * @return PH_NFCLIB_STATUS_SUCCESS on success
 * returns error codes on error
 */
uint32_t ulNfcCmdGetSerial( uint8_t *pucReadBuf, uint32_t *pulReadLen );

/**
 * @brief This function send request for key association.
 *
 * @param None
 *
 * @return Status of Key Associate
 *
 * returns PH_NFCLIB_STATUS_SUCCESS on success
 * returns error codes on error
 */
uint32_t ulNfcCmdKeyAssociate( void );

/**
 * @brief This function writes data to KeyFob.

 * @param pucWriteBuf Buffer contains data to write to the peer
 * @param ulWriteLen parameter to indicate number of bytes to write
 *
 * @return Status of data write
 * returns PH_NFCLIB_STATUS_SUCCESS on success
 * returns error codes on error
 */
uint32_t ulNfcCmdWriteRawData( uint8_t *pucWriteBuf, uint32_t ulWriteLen );

/**
 * @brief This function writes Public Key to KeyFob.

 * @param pucWriteBuf Buffer contains data to write to the peer
 * @param ulWriteLen parameter to indicate number of bytes to write
 *
 * @return Status of data write
 * returns PH_NFCLIB_STATUS_SUCCESS on success
 * returns error codes on error
 */
uint32_t ulNfcCmdWritePublicKey( uint8_t *pucWriteBuf, uint32_t ulWriteLen );

/**
 * @brief This function send the keyfob name to KeyFob.

 * @param pucWriteBuf Buffer contains name to write to the peer
 * @param ulWriteLen parameter to indicate number of bytes to write
 *
 * @return Status of data write
 * returns PH_NFCLIB_STATUS_SUCCESS on success
 * returns error codes on error
 */
uint32_t ulNfcCmdSaveKeyFobName( uint8_t *pucWriteBuf, uint32_t ulWriteLen );

/**
 * @brief This function send the Challenge to KeyFob.

 * @param pucWriteBuf Buffer contains challenge to write to the peer
 * @param ulWriteLen  parameter to indicate number of bytes to write
 *
 * @return Status of data write
 * returns PH_NFCLIB_STATUS_SUCCESS on success
 * returns error codes on error
 */
uint32_t ulNfcCmdSendORWLChallenge( uint8_t *pucWriteBuf, uint32_t ulWriteLen );

/**
 * @brief This function verifies the CVM data.
 *
 * Function takes the pin and does 3DES encryption using the shared secret key.
 * Send the encrypted data to KeyFob.
 *
 * @param pucKey        Pointer to shared secret key
 *
 * @return Status of CVM verification
 * returns PH_NFCLIB_STATUS_SUCCESS on success
 * returns error codes on error in selection
 */
uint32_t ulNfcCmdVerifyCVM( uint8_t *pucKey );

/**
 * @brief This function is to confirm the generated Shared secret key is proper.
 *
 * @param pucData Pointer to predefined encrypted data
 * @parma ulLen   Length of the data
 *
 * @return
 * returns PH_NFCLIB_STATUS_SUCCESS on success
 * returns error codes on error in selection
 */
uint32_t ulNfcCmdConfirmSSK( uint8_t *pucData , uint32_t ulLen );

/**
 * @brief This function is to instruct keyFob to save all keys.
 *
 * @param void
 *
 * @return
 * returns PH_NFCLIB_STATUS_SUCCESS on success
 * returns error codes on error in selection
 */
uint32_t ulNfcCmdSaveKey( void );
#endif /* NFC_COMMANDS_H */
