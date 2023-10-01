/******************************************************************************
 * @file LoRaMacHelper.h
 * @author  Insight SiP
 * @version V1.0.0
 * @date    06-november-2018
 * @brief LoRaMacHelper declaration file.
 *
 * @attention
 *	THIS SOFTWARE IS PROVIDED BY INSIGHT SIP "AS IS" AND ANY EXPRESS
 *	OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *	OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *	DISCLAIMED. IN NO EVENT SHALL INSIGHT SIP OR CONTRIBUTORS BE
 *	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *	GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 *	OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#ifndef __LORAMACHELPER_H__
#define __LORAMACHELPER_H__

#include "stdint.h"
#include "mac/Commissioning.h"
#include "boards/mcu/board.h"
#include "mac/LoRaMac.h"
#include "mac/region/Region.h"
#include "mac/region/RegionAS923.h"

#define LORAWAN_CONFIRMED_MSG_ON 0			/**< LoRaWAN confirmed messages */
#define LORAWAN_CERTIF_PORT 224				/**< LoRaWAN certification port */
#define LORAWAN_APP_PORT 2					/**< LoRaWAN application port, do not use 224. It is reserved for certification */
#define LORAWAN_APP_DATA_MAX_SIZE 242		/**< LoRaWAN User application data buffer size*/
#define LORAWAN_DEFAULT_DATARATE DR_3		/**< LoRaWAN Default datarate*/
#define LORAWAN_DEFAULT_TX_POWER TX_POWER_0 /**< LoRaWAN Default tx power*/

typedef struct lmh_param_s
{
	bool adr_enable;			/**< Activation state of adaptative Datarate */
	int8_t tx_data_rate;		/**< Uplink datarate, if AdrEnable is off */
	bool enable_public_network; /**< Enable or disable a public network */
	uint8_t nb_trials;			/**< Number of trials for the join request. */
	int8_t tx_power;			/**< Uplink power */
	bool duty_cycle;			/**< Enable or disable the duty cycle control */
} lmh_param_t;

typedef enum
{
	LMH_RESET = 0,
	LMH_SET,
	LMH_ONGOING,
	LMH_FAILED
} lmh_join_status;

typedef enum
{
	LMH_ERROR = -1,
	LMH_SUCCESS = 0,
	LMH_BUSY = 1
} lmh_error_status;

typedef enum
{
	LMH_UNCONFIRMED_MSG = 0,
	LMH_CONFIRMED_MSG = !LMH_UNCONFIRMED_MSG
} lmh_confirm;

/**@brief Application Data structure
 */
typedef struct
{
	uint8_t *buffer;  /**< point to the LoRa App data buffer */
	uint8_t buffsize; /**< LoRa App data buffer size */
	uint8_t port;	  /**< Port on which the LoRa App is data is sent/ received */
	int16_t rssi;
	uint8_t snr;
} lmh_app_data_t;

/**@brief LoRaMac Helper Callbacks
 */
typedef struct lmh_callback_s
{
	/**@brief Get the current battery level
 * @retval value  	battery level ( 0: very low, 254: fully charged )
 */
	uint8_t (*BoardGetBatteryLevel)(void);

	/**@brief Gets the board 64 bits unique ID 
 * @param id	Pointer to an array that will contain the Unique ID
 */
	void (*BoardGetUniqueId)(uint8_t *id);

	/**@brief Returns a pseudo random seed generated using the MCU Unique ID
 * @retval seed Generated pseudo random seed
 */
	uint32_t (*BoardGetRandomSeed)(void);

	/**@brief Process Rx Data received from Lora network 
 * @param AppData 	Rx structure
 */
	void (*lmh_RxData)(lmh_app_data_t *appdata);

	/**@brief callback indicating EndNode has just joined
 */
	void (*lmh_has_joined)();

	/**@brief Confirms the class change 
 * @param Class A, B, or C
 */
	void (*lmh_ConfirmClass)(DeviceClass_t Class);

	/**@brief callback indicating EndNode has just joined failed
 */
	void (*lmh_has_joined_failed)();

	/**@brief callback indicating unconfirmed TX-RX cycle finished
	 * Only called when unconfirmed message is used
 */
	void (*lmh_unconf_finished)();

	/**@brief callback indicating result of confirmed TX
	 * Only called when confirmed message is used
 */
	void (*lmh_conf_result)(bool result);

} lmh_callback_t;

/**@brief LoRaWAN compliance tests support data
 */
typedef struct LoraMacHelper_ComplianceTest_s
{
	bool running;
	uint8_t state;
	bool is_tx_confirmed;
	uint8_t app_port;
	uint8_t data_buffer_size;
	uint8_t data_buffer[64];
	uint16_t downlink_counter;
	bool link_check;
	uint8_t demod_margin;
	uint8_t nb_gateways;
} lmh_compliance_test_t;

/**@brief Lora Initialisation
 *
 * @param callbacks	Pointer to structure containing the callback functions
 * @param lora_param	Pointer to structure containing the parameters
 * @param otaa Choose OTAA (true) or ABP (false) activation
 * @param nodeClass Choose node class CLASS_A, CLASS_B or CLASS_C, default to CLASS_A
 * @param region Choose LoRaWAN region to set correct region parameters, default to EU868
 *
 * @retval error status
 */
lmh_error_status lmh_init(lmh_callback_t *callbacks, lmh_param_t lora_param,
						  bool otaa, eDeviceClass nodeClass = CLASS_A, LoRaMacRegion_t region = LORAMAC_REGION_EU868, bool region_change = false);

/**@brief Send data
 *
 * @param app_data Pointer to data structure to be sent
 * @param is_txconfirmed do we need confirmation?
 *
 * @retval error status
 */
lmh_error_status lmh_send(lmh_app_data_t *app_data, lmh_confirm is_txconfirmed);

/**@brief Send data and wait for RX2 window closed
 *  or timeout occurs
 * @param app_data Pointer to data structure to be sent
 * @param is_txconfirmed do we need confirmation?
 * @param time_out time to wait in milliseconds
 * @retval error status
 */
lmh_error_status lmh_send_blocking(lmh_app_data_t *app_data, lmh_confirm is_tx_confirmed, uint32_t time_out);

/**@brief Join a Lora Network in class A
 */
void lmh_join(void);

/**@brief Check whether the Device is joined to the network
 *
 * @retval returns LORAMACHELPER_SET if joined
 */
lmh_join_status lmh_join_status_get(void);

/**@brief Returns the Device address set by the LoRaWan
 * server after OTAA join success
 *
 * @retval returns Device Address
 */
uint32_t lmh_getDevAddr(void);

/**@brief change Lora Class
 *
 * @note callback LORA_ConfirmClass informs upper layer that the change has occured
 * @note Only switch from class A to class B/C OR from  class B/C to class A is allowed
 * @attention can be called only in LORA_ClassSwitchSlot or LORA_RxData callbacks
 *
 * @param newClass DeviceClass_t NewClass
 *
 * @retval LoraErrorStatus
 */
lmh_error_status lmh_class_request(DeviceClass_t newClass);

/**@brief get the current Lora Class
 *
 * @param currentClass DeviceClass_t NewClass
 */
void lmh_class_get(DeviceClass_t *currentClass);

/**@brief Configure data rate
 *
 * @param data_rate data rate
 * @param enable_adr  enable adaptative data rate
 */
void lmh_datarate_set(uint8_t data_rate, bool enable_adr);

/**@brief Configure tx power
 *
 * @param tx_power tx power
 */
void lmh_tx_power_set(uint8_t tx_power);

/**@brief Set Device IEEE EUI (big endian)
 *
 * @param userDevEui Device EUI as uint8_t[] *
 */
void lmh_setDevEui(uint8_t *userDevEui);

/**@brief Set Application IEEE EUI
 *
 * @param userAppEui Application IEEE EUI as uint8_t[] *
 */
void lmh_setAppEui(uint8_t *userAppEui);

/**@brief Set Application Key
 * AES encryption/decryption cipher application key
 *
 * @param userAppKey Application Key as uint8_t[] *
 */
void lmh_setAppKey(uint8_t *userAppKey);

/**@brief Set Network Session Key
 * AES encryption/decryption cipher network session key
 *
 * @param userNwkSKey Network Session Key as uint8_t[] *
 */
void lmh_setNwkSKey(uint8_t *userNwkSKey);

/**@brief Set Application Session Key
 * AES encryption/decryption cipher application session key
 *
 * @param userAppSKey Application Session Key as uint8_t[] *
 */
void lmh_setAppSKey(uint8_t *userAppSKey);

/**@brief Set Device address on the network (big endian)
 *
 * @param userDevAddr Device address as uint8_t[] *
 */
void lmh_setDevAddr(uint32_t userDevAddr);

/**@brief Set custom channel mask
 * 
 * @param subBand Sub channel number 1 to 8
 */
bool lmh_setSubBandChannels(uint8_t subBand);

/**@brief Disable channel hoping for connnection to
 * single channel gateways
 * Check the file CHANNELS.MD to find out which channel corresponds to which frequency 
 * in a specific region
 * Check the file Region.h to find out which datarate corresponds to which SF 
 * and bandwidth in a specific region
 *
 * @param userSingleChannel Channel to be used 
 * @param userDatarate Datarate to be used 
 */
void lmh_setSingleChannelGateway(uint8_t userSingleChannel, int8_t userDatarate);

/*!
 * \brief Adjust frequency band to AS923-1, AS923-2, AS923-3
 * \param version 1 => use default frequencies (AS923-1)
 *                2 => adjust frequencies by substracting 1.8MHz (AS923-2)
 *                3 => adjust frequencies by substracting 6.6MHz (AS923-3)
 *                4 => adjust frequencies by substracting 5.9MHz (AS923-4), only lower 8 channels supported
 * @retval true if success
 *  */
bool lmh_setAS923Version(uint8_t version);

/**
 * @brief Set max retries for confirmed messages
 * LoRaMac will resend confirmed message if no ACK was received until number of retries are exhausted
 * Limited to max of 8 retries
 * 
 * \param retries Number of retries, must be >0 and <9 
 * \retval true if value is within allowed range, false otherwise
 */
bool lmh_setConfRetries(uint8_t retries);

/**
 * @brief Get max retries for confirmed messages
 * LoRaMac will resend confirmed message if no ACK was received until number of retries are exhausted
 * Limited to max of 8 retries
 *
 * \retval Number of retries
 */
uint8_t lmh_getConfRetries(void);

/**
 * @brief Reset MAC counters
 * 
 */
void lmh_reset_mac(void);

#endif
