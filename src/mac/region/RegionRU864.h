/*!
 * \file      RegionRU864.h
 *
 * \brief     Region definition for RU864
 *
 * \copyright Revised BSD License, see file LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \defgroup  REGIONRU864 Region RU864
 *            Implementation according to LoRaWAN Specification v1.0.2.
 * \{
 */
#ifndef __REGION_RU864_H__
#define __REGION_RU864_H__

/*!
 * LoRaMac maximum number of channels
 */
#define RU864_MAX_NB_CHANNELS 16

/*!
 * Number of default channels
 */
#define RU864_NUMB_DEFAULT_CHANNELS 8

/*!
 * Number of channels to apply for the CF list
 */
/// \todo should be 6
#define RU864_NUMB_CHANNELS_CF_LIST 5

/*!
 * Minimal datarate that can be used by the node
 */
#define RU864_TX_MIN_DATARATE DR_0

/*!
 * Maximal datarate that can be used by the node
 */
#define RU864_TX_MAX_DATARATE DR_7

/*!
 * Minimal datarate that can be used by the node
 */
#define RU864_RX_MIN_DATARATE DR_0

/*!
 * Maximal datarate that can be used by the node
 */
#define RU864_RX_MAX_DATARATE DR_7

/*!
 * Default datarate used by the node
 */
#define RU864_DEFAULT_DATARATE DR_2

/*!
 * Minimal Rx1 receive datarate offset
 */
#define RU864_MIN_RX1_DR_OFFSET 0

/*!
 * Maximal Rx1 receive datarate offset
 */
#define RU864_MAX_RX1_DR_OFFSET 7

/*!
 * Default Rx1 receive datarate offset
 */
#define RU864_DEFAULT_RX1_DR_OFFSET 0

/*!
 * Minimal Tx output power that can be used by the node
 */
#define RU864_MIN_TX_POWER TX_POWER_7

/*!
 * Maximal Tx output power that can be used by the node
 */
#define RU864_MAX_TX_POWER TX_POWER_0

/*!
 * Default Tx output power used by the node
 */
#define RU864_DEFAULT_TX_POWER TX_POWER_0

/*!
 * Default Max EIRP
 */
#define RU864_DEFAULT_MAX_EIRP 16.0f

/*!
 * Default antenna gain
 */
#define RU864_DEFAULT_ANTENNA_GAIN 2.15f

/*!
 * ADR Ack limit
 */
#define RU864_ADR_ACK_LIMIT 64

/*!
 * ADR Ack delay
 */
#define RU864_ADR_ACK_DELAY 32

/*!
 * Enabled or disabled the duty cycle
 */
#define RU864_DUTY_CYCLE_ENABLED 0

/*!
 * Maximum RX window duration
 */
#define RU864_MAX_RX_WINDOW 3000

/*!
 * Receive delay 1
 */
#define RU864_RECEIVE_DELAY1 1000

/*!
 * Receive delay 2
 */
#define RU864_RECEIVE_DELAY2 2000

/*!
 * Join accept delay 1
 */
#define RU864_JOIN_ACCEPT_DELAY1 5000

/*!
 * Join accept delay 2
 */
#define RU864_JOIN_ACCEPT_DELAY2 6000

/*!
 * Maximum frame counter gap
 */
#define RU864_MAX_FCNT_GAP 16384

/*!
 * Ack timeout
 */
#define RU864_ACKTIMEOUT 2000

/*!
 * Random ack timeout limits
 */
#define RU864_ACK_TIMEOUT_RND 1000

#if (RU864_DEFAULT_DATARATE > DR_5)
#error "A default DR higher than DR_5 may lead to connectivity loss."
#endif

/*!
 * Second reception window channel datarate definition.
 */
#define RU864_RX_WND_2_DR DR_2

/*!
 * Maximum number of bands
 */
#define RU864_MAX_NB_BANDS 1

/*!
 * Band 0 definition
 * { DutyCycle, TxMaxPower, LastTxDoneTime, TimeOff }
 */
#define RU864_BAND0                      \
	{                                    \
		100, RU864_MAX_TX_POWER, 0, 0, 0 \
	} //  1.0 %

/*!
 * LoRaMac default channel 1
 * Channel = { Frequency [Hz], RX1 Frequency [Hz], { ( ( DrMax << 4 ) | DrMin ) }, Band }
 */
#define RU864_LC1                               \
	{                                           \
		868900000, 0, {((DR_5 << 4) | DR_0)}, 0 \
	}

/*!
 * LoRaMac default channel 2
 * Channel = { Frequency [Hz], RX1 Frequency [Hz], { ( ( DrMax << 4 ) | DrMin ) }, Band }
 */
#define RU864_LC2                               \
	{                                           \
		869100000, 0, {((DR_5 << 4) | DR_0)}, 0 \
	}
#define RU864_LC3                               \
	{                                           \
		869300000, 0, {((DR_5 << 4) | DR_0)}, 0 \
	}
#define RU864_LC4                               \
	{                                           \
		869500000, 0, {((DR_5 << 4) | DR_0)}, 0 \
	}
#define RU864_LC5                               \
	{                                           \
		869700000, 0, {((DR_5 << 4) | DR_0)}, 0 \
	}
#define RU864_LC6                               \
	{                                           \
		869900000, 0, {((DR_5 << 4) | DR_0)}, 0 \
	}
#define RU864_LC7                               \
	{                                           \
		870100000, 0, {((DR_5 << 4) | DR_0)}, 0 \
	}
#define RU864_LC8                               \
	{                                           \
		870300000, 0, {((DR_5 << 4) | DR_0)}, 0 \
	}

/*!
 * LoRaMac channels which are allowed for the join procedure
 */
#define RU864_JOIN_CHANNELS (uint16_t)(LC(1) | LC(2))

/*!
 * Data rates table definition
 */
static const uint8_t DataratesRU864[] = {12, 11, 10, 9, 8, 7, 7, 50};

/*!
 * Bandwidths table definition in Hz
 */
static const uint32_t BandwidthsRU864[] = {125000, 125000, 125000, 125000, 125000, 125000, 250000, 0};

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 * The table is valid for uplinks and downlinks.
 */
static const uint8_t MaxPayloadOfDatarateRU864[] = {59, 59, 59, 123, 230, 230, 230, 230};

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 * The table provides
 * repeater support.
 */
static const uint8_t MaxPayloadOfDatarateRepeaterRU864[] = {59, 59, 59, 123, 250, 250, 250, 250};

/*!
 * Effective datarate offsets for receive window 1.
 */
static const int8_t EffectiveRx1DrOffsetRU864[] = {0, 1, 2, 3, 4, 5, -1, -2};

/*!
 * \brief The function gets a value of a specific phy attribute.
 *
 * \param  getPhy Pointer to the function parameters.
 *
 * \retval Returns a structure containing the PHY parameter.
 */
PhyParam_t RegionRU864GetPhyParam(GetPhyParams_t *getPhy);

/*!
 * \brief Updates the last TX done parameters of the current channel.
 *
 * \param  txDone Pointer to the function parameters.
 */
void RegionRU864SetBandTxDone(SetBandTxDoneParams_t *txDone);

/*!
 * \brief Initializes the channels masks and the channels.
 *
 * \param  type Sets the initialization type.
 */
void RegionRU864InitDefaults(InitType_t type);

/*!
 * \brief Verifies a parameter.
 *
 * \param  verify Pointer to the function parameters.
 *
 * \param  phyAttribute Sets the initialization type.
 *
 * \retval Returns true, if the parameter is valid.
 */
bool RegionRU864Verify(VerifyParams_t *verify, PhyAttribute_t phyAttribute);

/*!
 * \brief The function parses the input buffer and sets up the channels of the
 *        CF list.
 *
 * \param  applyCFList Pointer to the function parameters.
 */
void RegionRU864ApplyCFList(ApplyCFListParams_t *applyCFList);

/*!
 * \brief Sets a channels mask.
 *
 * \param  chanMaskSet Pointer to the function parameters.
 *
 * \retval Returns true, if the channels mask could be set.
 */
bool RegionRU864ChanMaskSet(ChanMaskSetParams_t *chanMaskSet);

/*!
 * \brief Calculates the next datarate to set, when ADR is on or off.
 *
 * \param  adrNext Pointer to the function parameters.
 *
 * \param  drOut The calculated datarate for the next TX.
 *
 * \param  txPowOut The TX power for the next TX.
 *
 * \param  adrAckCounter The calculated ADR acknowledgement counter.
 *
 * \retval Returns true, if an ADR request should be performed.
 */
bool RegionRU864AdrNext(AdrNextParams_t *adrNext, int8_t *drOut, int8_t *txPowOut, uint32_t *adrAckCounter);

/*!
 * Computes the Rx window timeout and offset.
 *
 * \param  datarate     Rx window datarate index to be used
 *
 * \param  minRxSymbols Minimum required number of symbols to detect an Rx frame.
 *
 * \param  rxError      System maximum timing error of the receiver. In milliseconds
 *                          The receiver will turn on in a [-rxError : +rxError] ms
 *                          interval around RxOffset
 *
 * \param rxConfigParams Returns updated WindowTimeout and WindowOffset fields.
 */
void RegionRU864ComputeRxWindowParameters(int8_t datarate, uint8_t minRxSymbols, uint32_t rxError, RxConfigParams_t *rxConfigParams);

/*!
 * \brief Configuration of the RX windows.
 *
 * \param  rxConfig Pointer to the function parameters.
 *
 * \param  datarate The datarate index which was set.
 *
 * \retval Returns true, if the configuration was applied successfully.
 */
bool RegionRU864RxConfig(RxConfigParams_t *rxConfig, int8_t *datarate);

/*!
 * \brief TX configuration.
 *
 * \param  txConfig Pointer to the function parameters.
 *
 * \param  txPower The tx power index which was set.
 *
 * \param  txTimeOnAir The time-on-air of the frame.
 *
 * \retval Returns true, if the configuration was applied successfully.
 */
bool RegionRU864TxConfig(TxConfigParams_t *txConfig, int8_t *txPower, TimerTime_t *txTimeOnAir);

/*!
 * \brief The function processes a Link ADR Request.
 *
 * \param  linkAdrReq Pointer to the function parameters.
 *
 * \param  drOut Data rate.
 *
 * \param  txPowOut TX power.
 *
 * \param  nbRepOut Number of repeats.
 *
 * \param  nbBytesParsed Number of parsed bytes.
 *
 * \retval Returns the status of the operation, according to the LoRaMAC specification.
 */
uint8_t RegionRU864LinkAdrReq(LinkAdrReqParams_t *linkAdrReq, int8_t *drOut, int8_t *txPowOut, uint8_t *nbRepOut, uint8_t *nbBytesParsed);

/*!
 * \brief The function processes a RX Parameter Setup Request.
 *
 * \param  rxParamSetupReq Pointer to the function parameters.
 *
 * \retval Returns the status of the operation, according to the LoRaMAC specification.
 */
uint8_t RegionRU864RxParamSetupReq(RxParamSetupReqParams_t *rxParamSetupReq);

/*!
 * \brief The function processes a Channel Request.
 *
 * \param  newChannelReq Pointer to the function parameters.
 *
 * \retval Returns the status of the operation, according to the LoRaMAC specification.
 */
uint8_t RegionRU864NewChannelReq(NewChannelReqParams_t *newChannelReq);

/*!
 * \brief The function processes a TX ParamSetup Request.
 *
 * \param  txParamSetupReq Pointer to the function parameters.
 *
 * \retval Returns the status of the operation, according to the LoRaMAC specification.
 *         Returns -1, if the functionality is not implemented. In this case, the end node
 *         shall not process the command.
 */
int8_t RegionRU864TxParamSetupReq(TxParamSetupReqParams_t *txParamSetupReq);

/*!
 * \brief The function processes a DlChannel Request.
 *
 * \param  dlChannelReq Pointer to the function parameters.
 *
 * \retval Returns the status of the operation, according to the LoRaMAC specification.
 */
uint8_t RegionRU864DlChannelReq(DlChannelReqParams_t *dlChannelReq);

/*!
 * \brief Alternates the datarate of the channel for the join request.
 *
 * \param  alternateDr Pointer to the function parameters.
 *
 * \retval Datarate to apply.
 */
int8_t RegionRU864AlternateDr(AlternateDrParams_t *alternateDr);

/*!
 * \brief Calculates the back-off time.
 *
 * \param  calcBackOff Pointer to the function parameters.
 */
void RegionRU864CalcBackOff(CalcBackOffParams_t *calcBackOff);

/*!
 * \brief Searches and set the next random available channel
 *
 * \param  nextChanParams Parameters of next channel to use for TX.
 *
 * \param  channel Next channel to use for TX.
 *
 * \param  time Time to wait for the next transmission according to the duty
 *              cycle.
 *
 * \param  aggregatedTimeOff Updates the aggregated time off.
 *
 * \retval Function status [1: OK, 0: Unable to find a channel on the current datarate]
 */
bool RegionRU864NextChannel(NextChanParams_t *nextChanParams, uint8_t *channel, TimerTime_t *time, TimerTime_t *aggregatedTimeOff);

/*!
 * \brief Adds a channel.
 *
 * \param  channelAdd Pointer to the function parameters.
 *
 * \retval Status of the operation.
 */
LoRaMacStatus_t RegionRU864ChannelAdd(ChannelAddParams_t *channelAdd);

/*!
 * \brief Removes a channel.
 *
 * \param  channelRemove Pointer to the function parameters.
 *
 * \retval Returns true, if the channel was removed successfully.
 */
bool RegionRU864ChannelsRemove(ChannelRemoveParams_t *channelRemove);

/*!
 * \brief Sets the radio into continuous wave mode.
 *
 * \param  continuousWave Pointer to the function parameters.
 */
void RegionRU864SetContinuousWave(ContinuousWaveParams_t *continuousWave);

/*!
 * \brief Computes new datarate according to the given offset
 *
 * \param  downlinkDwellTime Downlink dwell time configuration. 0: No limit, 1: 400ms
 *
 * \param  dr Current datarate
 *
 * \param  drOffset Offset to be applied
 *
 * \retval newDr Computed datarate.
 */
uint8_t RegionRU864ApplyDrOffset(uint8_t downlinkDwellTime, int8_t dr, int8_t drOffset);

/*!
 * \brief Adjust frequency band to RU864-1, RU864-2, RU864-3
 * \param version 1 => use default frequencies (RU864-1)
 *                2 => adjust frequencies by substracting 1.8MHz (RU864-2)
 *                3 => adjust frequencies by substracting 6.6MHz (RU864-3)
 *                4 => adjust frequencies by substracting 5.9MHz (RU864-4), only lower 8 channels supported
 */
bool RegionRU864SetVersion(uint8_t version);

#endif // __REGION_RU864_H__
