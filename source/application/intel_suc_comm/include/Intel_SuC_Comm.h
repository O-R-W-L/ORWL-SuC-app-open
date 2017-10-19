 /**===========================================================================
 * @file intel_suc_comm.h
 *
 * @brief: This file contains common data types, structures, macros used for communication
 * between Intel BIOS and SuC over UART. All data is assumed to be unpacked data
 * in little endian format.
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
 *
 *
 * <b> REVISION HISTORY </b>
 * @n ================ @n
 * @Version 1.0 : 22-Feb-2017 : Author : Gupta
 *          1.1 : 02-May-2017 : Author : Gupta
 *                Updated ACK/NACK status
 *                Updated Macros for KeyManagement Attribute
 *          1.2 : Updated table and added new entry for Public and HLOS updates
 *          1.3 : Updated return values for keyFOB failure
 *          1.4 : Updated for the lenght of serial number for ORWL as per new scheme
 *          1.5 : Updated in camp for the testing and realization - Proxi Update, Pin on Boot
 *          1.6 : Updated with new field call sucSerialNum for tracking
 *			1.7 : Updated with new command for updating error status from Intel to SuC
 *          1.8 : Updated with two new commands for RTC management
 *          1.9 : Updated with Macros for masking WiFi Enable and Disable option.
 * $RevLog$
 *
 *============================================================================
 */
#ifndef __INTEL_SUC_COMM_H__
#define __INTEL_SUC_COMM_H__
#ifdef __cplusplus
extern "C" {
#endif


#define UART_SOF				0xFEDF 	 	/**< Start of Frame for UART Communication. Design SHIFT Bluetooth 16bit UUID*/
#define UART_SOF_0				0xFE 	 	/**< Start of Frame for UART Communication. Design SHIFT Bluetooth 16bit UUID*/
#define UART_SOF_1				0xDF 	 	/**< Start of Frame for UART Communication. Design SHIFT Bluetooth 16bit UUID*/
#define UART_MAX_DATA			255			/**< Maximum data length that is possible to exchange at datalink layer*/

#define UART_TYP_ENC			0x01		/**< UART Communication is encrypted. */
#define UART_TYP_ENC_NO			0x02		/**< UART Communication is un-encrypted. */
#define UART_TYP_ACK_NACK		0x03		/**< UART Packet is ACK/NACK. */

#define UART_TRANS_NACK			0x02		/**< UART Received packet has checksum error. */
#define UART_TRANS_ACK			0x01		/**< UART Received packet checksum is good. */


/** Intel SuC communication "Data" packet purpose
*/
#define CMD_READ				0x01		/**< Command for reading the data packet type. */
#define CMD_WRITE				0x02		/**< Command for Write/Updating the data packet type. */
#define RESP_READ				0x03		/**< Response for read request of a peer. */
#define RESP_WRITE				0x04		/**< Response for Write/Updating action for a peer. */

/** Intel SuC communication "Data" packet content
*/
#define ORWL_DEV_STATE			0x01		/**< Get device state of the SuC/ORWL at BIOS Boot or Any time. */
#define	ORWL_PROX_CONFIG		0x02		/**< ORWL Configuration for the Proximity protection feature. */
#define ORWL_DEV_PROD_CONFIG		0x03		/**< ORWL Production Configuration to identify the device. */
#define ORWL_KEY_MGT			0x04		/**< ORWL KeyFOB Management for adding or Deleting keys.  */
#define ORWL_KEY_FOR_DAT		0x05		/**< ORWL paired keyFOB devices data.  */
#define SUC_ROT_REGEN			0x06		/**< Regenerate Root of Trust. NVSRAM Erasure on SUC+SSD PW will be lost. */
#define ORWL_SEC_PIN_UPDATE		0x07		/**< This will initiate the NVSRAM Erasure. SSD data will be lost.  */
#define INTEL_DEV_ACT			0x09		/**< Intel sub-system ACPI state to go.   */
#define INT_SSD_MGMT_INTEL		0x0A		/**< Intel sub-system SSD Management.   */
#define INT_SSD_MGMT_SUC		0x0B		/**< Intel sub-system SSD Management to be updated for ROT Re-Generation request from SuC.   */
#define ORWL_TAMPER_LOG			0x0C		/**< Get the log history for the tamper.   */
#define ORWL_TAMPER_MGT			0x0D		/**< Request to perform Tamper reset .   */
#define ORWL_TAMPER_METR		0x0E		/**< Request to provide ORWL Tamper Metric on occurences and reset. - FUTURE SUPPORT   */
#define ORWL_SEC_STATE			0x0F		/**< Get the security status of ORWL */
#define ORWL_SEC_CHECK_LEVEL0		0x10		/**< Request re-Authentication on SuC for Level 1 type NFC+BLE.  */
#define ORWL_SEC_CHECK_LEVEL1		0x11		/**< Request re-Authentication on SuC for Level 2 type NFC+BLE+PIN Entry.  */
#define INTEL_HLOS_STATUS		0x12		/**< Update on OS Installation. State Transition.  */
#define COREBOOT_PUBLIC_KEY		0x13		/**< For any BIOS update in future, it should use this key for performing ECDSA signature verification.*/
#define DATA_ERROR_STATAUS		0x14		/**< Coreboot when received un-aligned or out of range data, this will be sent for OLED Update.*/
#define GET_RTC_TIME			0x15		/**< Get RTC Time of SUC for managing.*/
#define SET_RTC_TIME			0x16		/**< Set RTC Time of SUC for managing.*/

/** ORWL Product Dev State for respective Intel BIOS Behavior
*/

#define ORWL_DEV_STATE_PROD_PCBA	0x01	/**< ORWL is in production mode. For BIOS is production state. No security HandShake Required. */
#define ORWL_DEV_STATE_PROD_FA		0x02	/**< ORWL is in production mode. For BIOS is production state. No security HandShake Required. */
#define ORWL_DEV_STATE_PROD_TAMPER	0x03	/**< ORWL is in production tampered state. BIOS will never boot until tamper cleared */
#define ORWL_DEV_STATE_ROT		0x04	/**< ORWL is in ROT mode. Maps to "NONTamperedProduction State. BIOS just to load OS. */
#define ORWL_DEV_STATE_HLOS_OOB		0x05	/**< ORWL is in HLOS Installed mode. BIOS will never boot until ORWL enters User mode. */
#define ORWL_DEV_STATE_USER		0x06	/**< ORWL is in NotTamperedUser mode. All security aspects of the system are active. */
#define ORWL_DEV_STATE_TAMPERED		0x07	/**< ORWL is in NotTamperedUser mode. All security aspects of the system are active. */
#define ORWL_DEV_STATE_ENTER_BIOS	0x08	/**< ORWL is in NotTamperedUser mode and user placed the KeyFOB and pressed PWR BTN. */

#define ORWL_MAX_SUC_SERNUM			33		/**< Considering 32 Character as max and one null byte terminator */

#define SUC_WRITE_STATUS_SUCCESS	0x00	/**< SuC successfully executed the write operation submitted by BIOS. */
#define	SUC_WRITE_STATUS_FAIL_INVALID 	0x01	/**< SuC failed due to invalid options selection. */
#define	SUC_WRITE_STATUS_FAIL_MEM	0x02	/**< SuC failed to memory write issues. */
#define	SUC_WRITE_STATUS_FAIL_UNKOWN 	0x03	/**< SuC failed to unknown reasons. */
#define SUC_WRITE_STATUS_FAIL_AUTH	0x04 	/**< SuC failed to security authentication failed. */
#define SUC_WRITE_STATUS_TIMEOUT	0x05	/**< User didn't re-authenticate */
#define SUC_WRITE_STATUS_EXCEED_KEYFOB	0x06	/**< Indicates that the keyfob adding limit reached.  */
#define SUC_WRITE_STATUS_RTC_INVALID	0x07	/**< User trying to set time below 1-Jan-2015.  */
#define SUC_WRITE_STATUS_RTC_FAIL	0x08	/**< RTC Set time fail.  */

/* Possible status for RTC read command */
#define ORWL_RTC_RD_STAT_SUCC		0x01		/**< Indicates the RTC read is success and good for usage. */
#define ORWL_RTC_RD_STAT_BUSY		0x02		/**< Indicates HW RTC busy for providing right data. Retry to be done. */
#define ORWL_RTC_RD_STAT_ERR		0x03		/**< Read Failed due to error. Should never happen, but in case this, Reboot the  system */

#define INTEL_WRITE_STATUS_SUCCESS	0x00	/**< Intel successfully executed the write operation submitted by SuC. */
#define INTEL_WRITE_STATUS_FAIL		0x01	/**< Intel failed to complete write operation submitted by SuC. */



/** ORWL Proximity Protection
*/
#define PROX_PROT_DIS			0x00
#define PROX_PROT_05M			0x01 		/**<  Proximity protection for 5Meters. */
#define PROX_PROT_10M			0x02		/**<  Proximity protection for 10Meters. */
#define PROX_PROT_15M			0x03		/**<  Proximity protection for 15Meters. */
#define PROX_PROT_20M			0x04		/**<  Proximity protection for 20Meters. */
#define PROX_PROT_25M			0x05		/**<  Proximity protection for 25Meters. */
#define PROX_PROT_30M			0x06		/**<  Proximity protection for 30Meters. */

#define PROX_PROT_MOD_NO_CG		0x01		/**< Proximity protection mode for Intel Subsystem to be unchanged in existing state. */
#define PROX_PROT_MOD_STD_BY		0x02		/**< Proximity protection mode for Intel Subsystem to be in standby. */
#define PROX_PROT_MOD_SHT_DWN		0x03		/**< Proximity protection mode for Intel Subsystem to shutdown. */

#define PROX_PROT_MOT_EN		0x01		/**< During Proximity Protection, if motion is detected on ORWL Intel-subsystem power should power off. */
#define PROX_PROT_MOT_DIS		0x02		/**< During Proximity Protection, no action to be performed for Intel-subsystem. */


#define PROX_PROT_IO_EN			0x01		/**< Proximity protection mode for IO Interfaces enable. */
#define PROX_PROT_IO_DIS		0x00		/**< Proximity protection mode for IO Interfaces disable. */

#define PROX_PROT_BOOTPIN_EN	0x01		/**< Enable boot pin */
#define PROX_PROT_BOOTPIN_DIS	0x00		/**< Disable boot pin */

#define PROX_PROT_BOOTPIN_MASK		(0x1)	/**< Bit zero of BOOTPIN configuration is used for PIN entry disable or enable */
#define PROX_PROT_BOOTPIN_WIFI_MASK (0x2)   /**< Bit one of BOOTPIN configuration is used for WiFi enable or disable by coreboot */

/** ORWL KeyFOB Management and usage
*/
#define ORWL_KEY_FOB_MGT_ADD		0x00		/**< Indicates for adding the new keyFOB with respective attributes. */
#define ORWL_KEY_FOB_MGT_DEL		0x01		/**< Indicates for deleting the keyFOB for selected named KeyFOB. If only one Key is there,delete option should be disabled */

#define ORWL_KEY_FOB_MAX_COUNT		0x0A		/**< Maximum no. of KeyFOB an ORWL can have. */

#define ORWL_KEY_FOB_ATTR_FREE		0x00		/**< KeyFOB slot is free and we can add new keyfob in the given slot. */
#define ORWL_KEY_FOB_ATTR_ADMIN		0x01		/**< KeyFOB is of privilege Admin. */
#define ORWL_KEY_FOB_ATTR_USER		0x02		/**< KeyFOB is of privilege User. */

#define TAMPER_MGT_CLEAR		0x01		/**< Clear the Tamper event. But don't delete the log history. */
#define TAMPER_MGT_ERASE		0x02		/**< Clear the Tamper event and clear all the history. */


#define SSD_SEC_ERASE_TRUE		0x01		/**< Perform secure erase before setting the shared password. */
#define SSD_SEC_ERASE_FALSE		0x00		/**< Don't perform secure erase, but just use the password as needed. */

#define ORWL_MAX_SSD_PASSWORD		33		/**< Considering 32 Character as max and one null byte terminator */
/** Intel Sub-system System State
*/
#define INTEL_DEV_STATE_S0 		0x01		/**< Intel is working and all security attributes are satisfied for NFC Auth&Proximity. */
#define INTEL_DEV_STATE_STD_BY		0x02		/**< Intel sub-system is about to enter stdby mode. SuC can ask system to enter standby mode on proximity fail selection */
#define INTEL_DEV_STATE_HIBER		0x03		/**< Intel sub-system is about to enter hibernate mode. SuC can ask system to enter hibernate mode on proximity fail selection */
#define INTEL_DEV_STATE_SHT_DWN		0x04		/**< Intel sub-system is about to shutdown. SuC can ask system to shutdown/proximity protection mode. */
#define INTEL_DEV_STATE_RST		0x06		/**< Intel sub-system is about to restart. Until valid write sucess message, restart shouldn't happen. */

/** Intel OS installation status when Core boot is in Non-Tampered Production State for OS Installation

*/
#define HLOS_INSTALL_SUCCESS		0x01		/**< Coreboot was able to validate the signature for the disk copy image and success in copying to SSD. */
#define HLOS_INSTALL_FAIL_COPY		0x02		/**< Coreboot was able to validate the signature for the disk copy image and failed to copy. */
#define HLOS_INSTALL_FAIL_SIG		0x03		/**< Coreboot was not able to validate the signature for image in USB Drive. */
#define HLOS_INSTALL_FAIL_NOT_VALID	0x04		/**< Inserted pen drive doesn't contain the valid files for OS installation. */
#define HLOS_INSTALL_FAIL_UNKNOWN	0x05		/**< Any other core boot error for USB Pen Drive usage for validating the content and files. */

/** If SuC sends some invalid data, Coreboot will update SuC to display information on Error in Intel
*   SuC communication
*/
#define DATA_IN_VALID				0x01 		/**< Data sent by SuC is out of range for Coreboot to use. */

/** @struct IntelSuCData_t
    @brief Data payload for UART Communication.

    This structure provides primary mapping of the UART data payload
    that is exchanged between Intel SuC.
*/
typedef struct urtIntelSuCData
{
	unsigned char sof[2] ;		/**<  UART Start of Frame.UART_SOF startOfFrame[0] = 0xFE; startOfFrame[1] = 0xDF;*/
	unsigned char typ ;			/**<  UART Communication is encrypted or not - UART_ENC_xxx. */
	unsigned char len ;			/**<  len+1= Length of the datapacket/size of the structure. */
	unsigned char ckSum ;		/**<  Simple checksum of the data payload. */
	unsigned char data[UART_MAX_DATA] ;	/**<  Maximum data for the datapayload. */
}IntelSuCData_t ;


/** @struct urtIntelSuCAckNack
    @brief ACK/NACK for UART Communication.

    This structure provides primary mapping of the UART ACK or NACK for the data exchange
*/
typedef struct urtIntelSuCAckNack
{
	unsigned char sof[2] ;		/**<  UART Start of Frame.UART_SOF startOfFrame[0] = 0xFE; startOfFrame[1] = 0xDF;*/
	unsigned char typ ;			/**<  UART Packet is ACK/NACK- UART_TYP_ACK_NACK. */
	unsigned char ackStatus ;	/**<  Status is ACK for success and NACK for Failure. */
}IntelSuCAckNack_t ;



/**
Intel SuC Commmand Response data structure exchanged from peer to peer

  Data Packet Type		|CMD_READ	|RESP_READ		|CMD_WRITE		|RESP_WRITE		|Direction of commn
============================================================================================================================================
ORWL_DEV_STATE			|BiosSucAction_t|OrwlDevState_t		|	NA		|		NA	| Intel -> SuC
ORWL_PROX_CONFIG		|BiosSucAction_t|UIBiosProxiProt_t	|UIBiosProxiProt_t	|SucAckWriteStat	| Intel -> SuC
ORWL_DEV_PROD_CONFIG		|BiosSucAction_t|UIBiosOrwlConf_t	|	NA		|		NA	| Intel -> SuC
ORWL_KEY_MGT			|NULL		|NULL			|OrwlKeyMgt_t		|SucAckWriteStat	| Intel -> SuC
ORWL_KEY_FOR_DAT		|BiosSucAction_t|OrwlKeyData_t		|	NA		|		NA	| Intel -> SuC
SUC_ROT_REGEN			|	NA	|	NA		|BiosSucAction_t	|SucAckWriteStat	| Intel -> SuC
ORWL_SEC_PIN_UPDATE		|	NA	|	NA		|OrwlPinUpdate_t	|SucAckWriteStat	| Intel -> SuC
INTEL_DEV_ACT			|	NA	|	NA		|IntelSubState_t	|SucAckWriteStat	| Intel -> SuC
INT_SSD_MGMT_INTEL		|BiosSucAction_t|BiosSsdMgt_t		|	NA		|		NA	| Intel -> SuC
ORWL_TAMPER_LOG			|BiosSucAction_t|OrwlTamperLog_t	|	NA		|		NA	| Intel -> SuC
ORWL_TAMPER_MGT			|	NA	|	NA		|TamperMgtAct_t		|SucAckWriteStat	| Intel -> SuC
ORWL_SEC_CHECK_LEVEL0		|BiosSucAction_t|SucAckWriteStat	|	NA		|		NA	| Intel -> SuC
ORWL_SEC_CHECK_LEVEL1		|BiosSucAction_t|SucAckWriteStat	|	NA		|		NA	| Intel -> SuC
INTEL_HLOS_STATUS		|	NA	|	NA		|HlosInstallStat_t	|SucAckWriteStat	| Intel -> SuC
COREBOOT_PUBLIC_KEY		|BiosSucAction_t	|PublicKeyOrwlEcc	|	NA				|		NA			| Intel -> SuC
DATA_ERROR_STATAUS		|NA					|NA					|OrwlDataVal_t		|SucAckWriteStat	| Intel -> SuC
======================================================================================================================
*/

/**Following are the datatypes for data payload that is possible.
*/

/** @struct BiosSucAction_t
    @brief Depending on the device state, BIOS or SuC can request other peer respectively for performing
           reading or writing exchanged data structures/configurations.
*/
typedef struct sucBiosAction
{
	unsigned char cmd ;			/**< command to perform */
	unsigned char dataPktTyp;	/**< Datapacket to be exchanged */
}BiosSucAction_t ;


/** @struct BiosSuc1B_t
    @brief Depending on the device state, BIOS or SuC can request other peer respectively for performing
           reading or writing of data length 1 byte.

    This structure is used for following communication datatypes OrwlDevState_t,SucAckWriteStat,
    IntelSubState_t,

*/
typedef struct biosSuc1B
{
	BiosSucAction_t action ; 	/**< Action Payload */
	unsigned char statUpdate;	/**< 1-Byte Data payload */
}BiosSuc1B_t ;




/** @struct BiosSucActionWithData_t
    @brief Depending on the device state, BIOS or SuC can request other peer respectively for performing
           reading or writing with data as needed.
*/
typedef struct BiosSucActionWithData_t
{
	BiosSucAction_t action ; 	/**< Action Payload */
	unsigned char data[UART_MAX_DATA-2];/**< Datapacket structure data payload holder */
}BiosSucActionWithData_t ;



/******************
	1-Bytes response codes that are possible in Intel-SuC Communication
*/

typedef char OrwlDevState_t ;	/**< Refer to Macro group ORWL_DEV_STATE_xxx for possible values */
typedef char SucAckWriteStat ;	/**< Write status update for BIOS for all write options. SUC_WRITE_STATUS_xxx */
typedef char IntelSubState_t ; 	/**< Refer to Macro group INTEL_DEV_STATE_xxx for possible values */
typedef char IntelAckWriteStat_t; /**< Write status update for SUC for all write options. INTEL_WRITE_STATUS_xxx */
typedef char OrwlSecState_t ;	/**< Security state of the ORWL device. SUC_USER_SUB_STATE*/
typedef char TamperMgtAct_t ;   /**< Tamper management for different options - TAMPER_MGT_xxx */
typedef char OrwlDataVal_t ;    /**< On receiving invalid data, coreboot will sends the information - DATA_IN_VALID */

/** @struct UIBiosProxiProt_t
    @brief BIOS UI configuration and management for the proximity
           protection of the ORWL.

    This structure provides primary mapping SuC Configuration for BLE Proximity protection
*/
typedef struct uiBiosProxiProt
{
	unsigned char range ;		/**< range - range for proximity protection.PROX_PROT_xx */
	unsigned char mode ;		/**< mode - On proximity protection kick-start, mode of Intel Subsytem required. PROX_PROT_MOD_xxx */
	unsigned char ioEnDis ;		/**< ioEnDis - Indicates that whether IO interfaces should be disabled or no. */
	unsigned char motionAct ;	/**< motionAct - During proximity protection, what should be the device behavior - PROX_PROT_MOT_xxx. */
	unsigned char AskPinOnBoot ;/**< AskPinOnBoot - Ask for PIN during boot */
}UIBiosProxiProt_t ;


/** @struct UIBiosOrwlConf_t
    @brief BIOS need to display ORWL Production information. This structure provides the data that are displayed
           exchanged between both the components.
*/
typedef struct uiBiosOrwlConf
{
	unsigned char srNum[ORWL_MAX_SUC_SERNUM] ; 	/**< srNum[30]- String for ORWL Serial Number */
	unsigned char swVerMajor ; 	/**< swVerMajor - SuC SW Major Release number */
	unsigned char swVerMinor ; 	/**< swVerMinor - SuC SW Minor Release number. SuC FW is String swVerMajor.swVerMinor */
	unsigned char hwVer ;		/**< hwVer - PCBA HW Version for SuC Config */
	unsigned char currTime[20] ;/**< currTime - Current SuC Time in format MM-DD-YYYY HH:MM:SS PCBA HW Version for SuC Config */
	unsigned char prodTime[20] ;/**< prodTime - Production of ORWL Time in format MM-DD-YYYY HH:MM:SS PCBA HW Version for SuC Config */
	unsigned char sucSerialNum[ORWL_MAX_SUC_SERNUM] ; /**< sucSerialNum - ORWL Main board serial number to track KeyManagement */
} UIBiosOrwlConf_t;

/** @struct OrwlKeyMgt_t
    @brief This is data structure exchanged between BIOS and SuC for adding or deleting the KeyFOB in the ORWL.
*/
typedef struct orwlKeyMgt
{
	unsigned char action ; 		/**< action - Indicates whether to add or delete the KeyFob - ORWL_KEY_FOB_MGT_xxx*/
	unsigned char keyFobNam[20] ;/**< keyFobNam - User configurable name for a KeyFOB. Max len of 19characters */
	unsigned char preAttr ;		/**< preAttr - Indicates what previliges keyFob need to have. ORWL_KEY_FOB_ATTR_xxx. */
} OrwlKeyMgt_t;



/** @struct KeyDataIndividual_t
    @brief User configurable KeyFOB data.
*/
typedef struct keyDataIndividual
{
	unsigned char keyFobNam[20] ;/**< keyFobNam - User configurable name for a KeyFOB. Max len of 19characters */
	unsigned char preAttr ;		/**< preAttr - Indicates what previliges keyFob need to have. There are two levels. ORWL_KEY_FOB_ATTR_xxx */

}KeyDataIndividual_t;


/** @struct OrwlKeyData_t
    @brief Data structure for getting the complete list of keyFob in ORWL. This will be requested when user wants to delete/Modify.
*/
typedef struct orwlKeyData
{
	KeyDataIndividual_t keyFob[ORWL_KEY_FOB_MAX_COUNT] ;
	unsigned char keyFobCount ; /**< keyFobCount - Indicates how many keyFOB are added*/
} OrwlKeyData_t;


/** @struct BiosSsdMgt_t
    @brief SSD has two passwords - User and Admin. This structures carries the required data.
*/
typedef struct biosSsdMgt
{
	unsigned char userPwd[ORWL_MAX_SSD_PASSWORD] ;	/**< userPwd - User password for SSD SED Drive for data protection.  */
	unsigned char adminPwd[ORWL_MAX_SSD_PASSWORD] ;	/**< adminPwd - Admin password for SSD management for secure erase and usage.  */
	unsigned char secEraseReq ;	/**< secEraseReq - Perform secure erase before setting the passwords or just update the password without loosing data - SSD_SEC_ERASE_xxx*/
} BiosSsdMgt_t;


/** @struct OrwlTamperLog_t
    @brief Formated string containing information on what is the reason for the tamper and its occurance time.
*/
typedef struct orwlTamperLog
{
	unsigned char formatLog[128] ;
} OrwlTamperLog_t;

/** @struct OrwlPinUpdate_t
    @brief 6-Digit security PIN in ASCII value for the digit. Index 0 corresponds to first digit to be entered.
*/
typedef struct orwlPinUpdate
{
	unsigned char pinData[6] ;
} OrwlPinUpdate_t;

/** @struct HlosInstallStat_t
    @brief Data payload indicating information on the OS Installation success and requesting SuC to generate the OS HASH for integrity verification

    Structure will provide update on the HLOS Installation for coreboot in Non-Tampered Production State for installing from
    USB pen drive. .
*/
typedef struct hlosInstallStat
{
	unsigned char serNum[30] ;		/**< serNum - Serial number for SSD .  */
	unsigned char statusUpdate ;	/**< statusUpdate - Status update for the USB Image booting. HLOS_INSTALL_XXX  .  */
} HlosInstallStat_t;

/** Structure to get/set RTC time communication between SUC and BIOS. Structure
 *  will provide standard date and time managed by system.
 */
typedef struct OrwlRTCTime
{
    unsigned char ucSecond;	    /**< second resolution */
    unsigned char ucMinute;	    /**< minute resolution */
    unsigned char ucHour;	    /**< hour resolution */
    unsigned char ucWeekday;	    /**< Weekday 0-sunday... 6-saturday */
    unsigned char ucDate;           /**< Date */
    unsigned char ucMonth;          /**< month */
    unsigned char ucYearOffset2000; /**< Year offset from year 2000*/
}OrwlRTCTime_t;

/** @struct OrwlRTCTimeStat_t
    @brief Date and time structure with read status update
    Provides the RTC Time with status of success or fail. On failure, Coreboot need to
    request the RTC time.
*/
typedef struct OrwlRTCTimeStat
{
    /** Read status update indicating whether it was success or fail */
    unsigned char ucReadRTCStat;
    OrwlRTCTime_t currTime;
}OrwlRTCTimeStat_t;

/** @struct publiKeyORWLEcc
    @brief Data payload indicating information on the OTP flashed public key of Maxim chip

*/
typedef struct publiKeyORWLEcc
{
	unsigned char key[32] ;		/**< key - 256bit public key .  */
} PublicKeyOrwlEcc;

#ifdef __cplusplus
extern "C" }
#endif
#endif /*__INTEL_SUC_COMM_H__*/


