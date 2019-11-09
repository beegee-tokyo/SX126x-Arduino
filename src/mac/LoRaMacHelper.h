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

extern "C"
{

#define LORAWAN_CONFIRMED_MSG_ON 0			/**< LoRaWAN confirmed messages */
#define LORAWAN_CERTIF_PORT 224				/**< LoRaWAN certification port */
#define LORAWAN_APP_PORT 2					/**< LoRaWAN application port, do not use 224. It is reserved for certification */
#define LORAWAN_APP_DATA_MAX_SIZE 242		/**< LoRaWAN User application data buffer size*/
#define LORAWAN_DEFAULT_DATARATE DR_0		/**< LoRaWAN Default datarate*/
#define LORAWAN_DEFAULT_TX_POWER TX_POWER_0 /**< LoRaWAN Default tx power*/

	typedef struct lmh_param_s
	{
		bool adr_enable;			/**< Activation state of adaptative Datarate */
		int8_t tx_data_rate;		/**< Uplink datarate, if AdrEnable is off */
		bool enable_public_network; /**< Enable or disable a public network */
		uint8_t nb_trials;			/**< Number of trials for the join request. */
		int8_t tx_power;			/**< Uplink power */

	} lmh_param_t;

	typedef enum
	{
		LMH_RESET = 0,
		LMH_SET = !LMH_RESET
	} lmh_join_status;

	typedef enum
	{
		LMH_ERROR = -1,
		LMH_SUCCESS = 0
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
		uint8_t port;	 /**< Port on which the LoRa App is data is sent/ received */
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
 * @param[in] id	Pointer to an array that will contain the Unique ID
 */
		void (*BoardGetUniqueId)(uint8_t *id);

		/**@brief Returns a pseudo random seed generated using the MCU Unique ID
 * @retval seed Generated pseudo random seed
 */
		uint32_t (*BoardGetRandomSeed)(void);

		/**@brief Process Rx Data received from Lora network 
 * @param[in] AppData 	Rx structure
 */
		void (*lmh_RxData)(lmh_app_data_t *appdata);

		/**@brief callback indicating EndNode has just joined
 */
		void (*lmh_has_joined)();

		/**@brief Confirms the class change 
 * @param[in] Class A, B, or C
 */
		void (*lmh_ConfirmClass)(DeviceClass_t Class);

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
 * @param[in] callbacks	Pointer to structure containing the callback functions
 * @param[in] LoRaParam	Pointer to structure containing the parameters
 */
	lmh_error_status lmh_init(lmh_callback_t *callbacks, lmh_param_t lora_param);

	/**@brief Send data
 *
 * @param[in] app_data Pointer to data structure to be sent
 * @param[in] is_txconfirmed do we need confirmation?
 *
 * @retval error status
 */
	lmh_error_status lmh_send(lmh_app_data_t *app_data, lmh_confirm is_txconfirmed);

	/**@brief Join a Lora Network in class A
 */
	void lmh_join(void);

	/**@brief Check whether the Device is joined to the network
 *
 * @retval returns LORAMACHELPER_SET if joined
 */
	lmh_join_status lmh_join_status_get(void);

	/**@brief change Lora Class
 *
 * @Note callback LORA_ConfirmClass informs upper layer that the change has occured
 * @Note Only switch from class A to class B/C OR from  class B/C to class A is allowed
 * @Attention can be called only in LORA_ClassSwitchSlot or LORA_RxData callbacks
 *
 * @param[in] DeviceClass_t NewClass
 *
 * @retval LoraErrorStatus
 */
	lmh_error_status lmh_class_request(DeviceClass_t newClass);

	/**@brief get the current Lora Class
 *
 * @param[in] DeviceClass_t NewClass
 */
	void lmh_class_get(DeviceClass_t *currentClass);

	/**@brief Configure data rate
 *
 * @param[in] data_rate data rate
 * @param[in] enable_adr  enable adaptative data rate
 */
	void lmh_datarate_set(uint8_t data_rate, uint8_t enable_adr);

	/**@brief Configure tx power
 *
 * @param[in] tx_power tx power
 */
	void lmh_tx_power_set(uint8_t tx_power);

	/**@brief Set Device IEEE EUI (big endian)
 *
 * @param[in] Device EUI as uint8_t[] *
 */
	void lmh_setDevEui(uint8_t *userDevEui);

	/**@brief Set Application IEEE EUI
 *
 * @param[in] Application IEEE EUI as uint8_t[] *
 */
	void lmh_setAppEui(uint8_t *userAppEui);

	/**@brief Set Application Key
 * AES encryption/decryption cipher application key
 *
 * @param[in] Application Key as uint8_t[] *
 */
	void lmh_setAppKey(uint8_t *userAppKey);

	/**@brief Set Network Session Key
 * AES encryption/decryption cipher network session key
 *
 * @param[in] Network Session Key as uint8_t[] *
 */
	void lmh_setNwkSKey(uint8_t *userNwkSKey);

	/**@brief Set Application Session Key
 * AES encryption/decryption cipher application session key
 *
 * @param[in] Application Session Key as uint8_t[] *
 */
	void lmh_setAppSKey(uint8_t *userAppSKey);

	/**@brief Set Device address on the network (big endian)
 *
 * @param[in] Device address as uint8_t[] *
 */
	void lmh_setDevAddr(uint32_t userDevAddr);

	/**@brief Set custom channel mask
 * 
 * @param[in] sub channel number 1 to 8
 */
	bool lmh_setSubBandChannels(uint8_t subBand);

	/**@brief Disable channel hoping for connnection to
 * single channel gateways
 * Check the file CHANNELS.MD to find out which channel corresponds to which frequency 
 * in a specific region
 * Check the file Region.h to find out which datarate corresponds to which SF 
 * and bandwidth in a specific region
 *
 * @param[in] Channel to be used 
 * @param[in] Datarate to be used 
 */
	void lmh_setSingleChannelGateway(uint8_t userSingleChannel, int8_t userDatarate);
};
#endif
