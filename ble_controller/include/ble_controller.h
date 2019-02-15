/*
 * Copyright (c) 2018 - 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */


/**
 * @file ble_controller.h
 *
 * @brief This file defines APIs needed to configure, enable, and use the BLE Controller.
 */


#ifndef BLE_CONTROLLER_H__
#define BLE_CONTROLLER_H__


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include "nrf.h"
#include "nrf_errno.h"


/**< @brief Default resource configuration tag. */
#define BLE_CONTROLLER_DEFAULT_RESOURCE_CFG_TAG  0

/**< @brief Default maximum number of concurrent slave links. */
#define BLE_CONTROLLER_DEFAULT_SLAVE_COUNT   1

/**< @brief Default maximum number of concurrent master links. */
#define BLE_CONTROLLER_DEFAULT_MASTER_COUNT  1

/**< @brief Default maximum LL TX packet size. */
#define BLE_CONTROLLER_DEFAULT_TX_PACKET_SIZE 27

/**< @brief Default maximum LL RX packet size. */
#define BLE_CONTROLLER_DEFAULT_RX_PACKET_SIZE 27

/**< @brief Default maximum LL TX packet count per connection. */
#define BLE_CONTROLLER_DEFAULT_TX_PACKET_COUNT 3

/**< @brief Default maximum LL RX packet count per connection. */
#define BLE_CONTROLLER_DEFAULT_RX_PACKET_COUNT 3

/**< @brief Default connection event length */
#define BLE_CONTROLLER_DEFAULT_EVENT_LENGTH_US 7500UL

/**< @brief Recommended RC clock calibration timer interval. */
#define BLE_CONTROLLER_RECOMMENDED_RC_CTIV        16

/**< @brief Recommended RC clock calibration timer interval for temperature changes. */
#define BLE_CONTROLLER_RECOMMENDED_RC_TEMP_CTIV   2


/** @brief    Function prototype for the fault handler.
 *
 * @note      The BLE Controller will disable all interrupts prior to calling the fault handler.
 *            The BLE Controller will reset the chip if the application returns from this function.
 *
 * @param[in] file   The filename where the assertion occurred.
 * @param[in] line   The line number where the assertion occurred.
 */
typedef void (*ble_controller_fault_handler_t)(const char * file, const uint32_t line);


/** @brief Function prototype for the BLE Controller callback.
 *
 *  @sa @ref ble_controller_enable */
typedef void (*ble_controller_callback_t)(void);


/** @brief Low frequency clock accuracy. */
enum NRF_LF_CLOCK_ACCURACY
{
    NRF_LF_CLOCK_ACCURACY_250_PPM = 0,
    NRF_LF_CLOCK_ACCURACY_500_PPM = 1,
    NRF_LF_CLOCK_ACCURACY_150_PPM = 2,
    NRF_LF_CLOCK_ACCURACY_100_PPM = 3,
    NRF_LF_CLOCK_ACCURACY_75_PPM  = 4,
    NRF_LF_CLOCK_ACCURACY_50_PPM  = 5,
    NRF_LF_CLOCK_ACCURACY_30_PPM  = 6,
    NRF_LF_CLOCK_ACCURACY_20_PPM  = 7,
    NRF_LF_CLOCK_ACCURACY_10_PPM  = 8,
    NRF_LF_CLOCK_ACCURACY_5_PPM   = 9,
    NRF_LF_CLOCK_ACCURACY_2_PPM   = 10,
    NRF_LF_CLOCK_ACCURACY_1_PPM   = 11,
};


/** @brief Low frequency clock source. */
enum NRF_LF_CLOCK_SRC
{
    NRF_LF_CLOCK_SRC_RC      = 0,    /**< LFCLK RC oscillator. */
    NRF_LF_CLOCK_SRC_XTAL    = 1,    /**< LFCLK crystal oscillator. */
    NRF_LF_CLOCK_SRC_SYNTH   = 2,    /**< LFCLK Synthesized from HFCLK. */
};


/**@brief Type representing LFCLK oscillator source. */
typedef struct
{
    uint8_t lf_clk_source; /**< LF oscillator clock source, see @ref NRF_LF_CLOCK_SRC. */
    uint8_t rc_ctiv;       /**< Only for ::NRF_LF_CLOCK_SRC_RC.
                                Calibration timer interval in 1/4 second units.
                                @note To avoid excessive clock drift, 0.5 degrees Celsius is the
                                      maximum temperature change allowed in one calibration timer
                                      interval. The interval should be selected to ensure this.
                                @note Must be 0 if source is not ::NRF_LF_CLOCK_SRC_RC. */
    uint8_t rc_temp_ctiv;  /**< Only for ::NRF_LF_CLOCK_SRC_RC:
                                How often (in number of calibration intervals) the RC oscillator
                                will be calibrated.
                                  0: Always calibrate even if the temperature hasn't changed.
                                  1: Only calibrate if the temperature has changed.
                                  2-33: Check the temperature and only calibrate if it has changed,
                                        however calibration will take place every rc_temp_ctiv
                                        intervals in any case.

                                @note Must be 0 if source is not ::NRF_LF_CLOCK_SRC_RC.

                                @note The application must ensure calibration at least once every
                                      8 seconds to ensure +/-500 ppm clock stability.
                                      The recommended configuration for ::NRF_LF_CLOCK_SRC_RC
                                      is given by @ref BLE_CONTROLLER_RECOMMENDED_RC_CTIV
                                      and @ref BLE_CONTROLLER_RECOMMENDED_RC_TEMP_CTIV.
                                      This sets the calibration interval to 4 seconds and guarantees
                                      clock calibration every second calibration interval. That is,
                                      the clock will be calibrated every 8 seconds.
                                      If the temperature changes more that 0.5 every 4 seconds, the
                                      clock will be calibrated every 4 seconds. See the
                                      Product Specification for more information. */
    uint8_t accuracy;    /**< External clock accuracy used in the LL to compute timing windows,
                              see @ref NRF_LF_CLOCK_SRC. */
} nrf_lf_clock_cfg_t;


enum BLE_CONTROLLER_CFG_TYPE
{
    BLE_CONTROLLER_CFG_TYPE_NONE         = 0,  /**< No configuration update. */
    BLE_CONTROLLER_CFG_TYPE_MASTER_COUNT = 1,  /**< Number of concurrent master roles.
                                                    @sa ble_controller_cfg_t::master_count. */
    BLE_CONTROLLER_CFG_TYPE_SLAVE_COUNT  = 2,  /**< Number of concurrent slave roles.
                                                    @sa ble_controller_cfg_t::slave_count. */
    BLE_CONTROLLER_CFG_TYPE_BUFFER_CFG   = 3,  /**< Buffer configuration per connection.
                                                    @sa ble_controller_cfg_t::buffer_cfg. */
    BLE_CONTROLLER_CFG_TYPE_EVENT_LENGTH = 4,  /**< Maximum event length.
                                                    @sa ble_controller_cfg_t::event_length. */
};


/** @brief Role count. */
typedef struct
{
    uint8_t count;   /**< Max number of concurrent roles. */
} ble_controller_cfg_role_count_t;


/** @brief Buffer configuration. */
typedef struct
{
    uint8_t tx_packet_size;   /**< Link Layer TX packet size. Valid range: 27-251. */
    uint8_t rx_packet_size;   /**< Link Layer RX packet size. Valid range: 27-251. */
    uint8_t tx_packet_count;  /**< Link Layer TX packet count per link. */
    uint8_t rx_packet_count;  /**< Link Layer RX packet count per link. */
} ble_controller_cfg_buffer_cfg_t;


/** @brief Connection event length configuration. */
typedef struct
{
    uint32_t event_length_us; /**< Maximum connection event length */
} ble_controller_cfg_event_length_t;


/** @brief BLE controller configuration.  */
typedef union
{
    ble_controller_cfg_role_count_t   master_count;  /**< Max number of concurrent master connections.
                                                          Default: @ref BLE_CONTROLLER_DEFAULT_MASTER_COUNT. */
    ble_controller_cfg_role_count_t   slave_count;   /**< Max number of concurrent slave connections.
                                                          Default: @ref BLE_CONTROLLER_DEFAULT_SLAVE_COUNT. */
    ble_controller_cfg_buffer_cfg_t   buffer_cfg;    /**< Max number of concurrent slave connections.
                                                          Default: @ref BLE_CONTROLLER_DEFAULT_SLAVE_COUNT. */
    ble_controller_cfg_event_length_t event_length;  /**< Max connection event length.
                                                          Default: @ref BLE_CONTROLLER_DEFAULT_EVENT_LENGTH_US. */
} ble_controller_cfg_t;


/** @brief     Initialize the BLE Controller
 *
 * After this function is called, the application may use SoC and timeslot APIs.
 *
 * @param[in]  fault_handler           The fault handler will be executed when there is an internal
 *                                     error in the BLE Controller.
 * @param[in]  p_clk_cfg               Clock configuration. If NULL is provided, the default clock
 *                                     configuration will be used. That is @ref NRF_CLOCK_LF_SRC_RC,
 *                                     @ref NRF_CLOCK_LF_ACCURACY_250_PPM,
 *                                     @ref BLE_CONTROLLER_RECOMMENDED_RC_CTIV,
 *                                     and @ref BLE_CONTROLLER_RECOMMENDED_RC_TEMP_CTIV.
 * @param[in] low_prio_tasks_irq       The BLE controller library will pend this IRQ when there
 *                                     are low priority tasks to be processed. The application
 *                                     shall call @ref ble_controller_low_prio_tasks_process
 *                                     after this IRQ has occurred.
 *
 * @retval 0              Success
 * @retval - ::NRF_EINVAL Invalid argument provided
 */
int32_t ble_controller_init(ble_controller_fault_handler_t fault_handler,
                            nrf_lf_clock_cfg_t * p_clk_cfg,
                            IRQn_Type low_prio_tasks_irq);


/** @brief Change or add a BLE Controller configuration
 *
 * To change the default configuration, update @ref BLE_CONTROLLER_DEFAULT_RESOURCE_CFG_TAG.
 * To create or update a new configuration, provide another resource_cfg_tag.
 *
 * @note The application can set config_type to @ref BLE_CONTROLLER_CFG_TYPE_NONE to obtain
 *       the required memory size for the current configuration in bytes.
 *
 * @note Resource configuration can only be performed prior to calling @ref ble_controller_enable.
 *       However, the current configuration may be changed after enabling the BLE Controller.
 *
 * @param[in]  config_tag       Configuration tag.
 * @param[in]  config_type      Configuration type. @sa BLE_CONTROLLER_CFG_TYPE.
 * @param[in]  p_resource_cfg   Configuration to be changed.
 *
 * @returns Required memory size for the current configuration in bytes.
 * @retval - ::NRF_EOPNOTSUPP  Unsupported configuration
 * @retval - ::NRF_EINVAL      Invalid argument provided
 */
int32_t ble_controller_cfg_set(uint8_t config_tag,
                               uint8_t config_type,
                               ble_controller_cfg_t const * p_resource_cfg);


/** @brief     Enable the BLE Controller
 *
 * After this function is called, the application may utilize HCI APIs.
 *
 * @param[in]  callback       The callback will be executed when HCI data or and HCI event is available.
 *                            The callback will be executed in the same context as
 *                            @ref ble_controller_low_prio_tasks_process.
 *                            @sa hci_evt_get and @ref hci_data_get.
 * @param[in]  p_mem          Provide memory for the current resource configuration.
 *                            If custom resource configurations are used, use the value returned
 *                            from @ref ble_controller_cfg_set.
 *
 * @retval 0              Success
 * @retval - ::NRF_EINVAL Invalid argument provided
 */
int32_t ble_controller_enable(ble_controller_callback_t callback,
                              uint8_t * p_mem);


/** @brief Disable the BLE Controller
 *
 * This call is synchronous. After the BLE Controller is disabled, BLE functionality is no
 * longer available.
 *
 * @retval 0 Success
 */
int32_t ble_controller_disable(void);


/** @brief Instruct the BLE controller to process low priority tasks
 *
 * This function should be called after the controller has indicated that it has low priority tasks
 * to be processed. See @ref ble_controller_init.
 *
 * @note This function is expected to be called from the same execution priority as the HCI APIs.
 *       Not doing so will lead to undefined behavior.
 */
void ble_controller_low_prio_tasks_process(void);


/** @brief BLE Controller RADIO interrupt handler
 *
 * @note   This function should be called when a radio interrupt occurs.
 *         The interrupt priority level should be set to 0.
 */
void ble_controller_RADIO_IRQHandler(void);


/** @brief BLE Controller RTC0 interrupt handler
 *
 * @note   This function should be called when a RTC0 interrupt occurs.
 *         The interrupt priority level should be set to 0.
 */
void ble_controller_RTC0_IRQHandler(void);


/** @brief BLE Controller TIMER0 interrupt handler.
 *
 * @note   This function should be called when a TIMER0 interrupt occurs.
 *         The interrupt priority level should be set to 0.
 */
void ble_controller_TIMER0_IRQHandler(void);


/** @brief BLE Controller RNG interrupt handler
 *
 * @note   This function should be called when a RNG interrupt occurs.
 *         The interrupt priority level should be lower than priority level 0, that is,
 *         a higher numerical priority value.
 */
void ble_controller_RNG_IRQHandler(void);


/** @brief BLE Controller POWER_CLOCK interrupt handler
 *
 * @note   This function should be called when a POWER_CLOCK interrupt occurs.
 *         The interrupt priority level should be lower than priority level 0, that is,
 *         a higher numerical priority value.
 */
void ble_controller_POWER_CLOCK_IRQHandler(void);


#ifdef __cplusplus
}
#endif


#endif /* BLE_CONTROLLER_H__ */
