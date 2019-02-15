/*
 * Copyright (c) 2018 - 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */


/**
 * @file timeslot.h
 *
 * @brief BLE Controller Timeslot Interface
 *
 * The Timeslot interface allows the application to run another radio protocol concurrently with
 * BLE activity. When a timeslot is granted, the application has exclusive access
 * to the normally blocked RADIO, TIMER0, CCM, and AAR peripherals.
 * The application can use the peripherals freely for the duration of the timeslot.
 */


#ifndef TIMESLOT_H__
#define TIMESLOT_H__


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include "nrf_errno.h"


/**@brief  The shortest allowed timeslot event in microseconds. */
#define TIMESLOT_LENGTH_MIN_US                    (100UL)

/**@brief  The longest allowed timeslot event in microseconds. */
#define TIMESLOT_LENGTH_MAX_US                    (100000UL)

/**@brief  The longest timeslot distance in microseconds allowed for the distance parameter
           see @ref timeslot_request_normal_t. */
#define TIMESLOT_DISTANCE_MAX_US                  (128000000UL - 1UL)

/**@brief  The longest timeout in microseconds allowed when requesting the earliest possible timeslot. */
#define TIMESLOT_EARLIEST_TIMEOUT_MAX_US          (128000000UL - 1UL)

/**@brief  The maximum jitter in @ref TIMESLOT_SIGNAL_START relative to the requested start time. */
#define TIMESLOT_START_JITTER_US                  (2UL)

/**@brief The minimum allowed timeslot extension time. */
#define TIMESLOT_EXTENSION_TIME_MIN_US            (200UL)

/**@brief The maximum processing time to handle a timeslot extension. */
#define TIMESLOT_EXTENSION_PROCESSING_TIME_MAX_US (17UL)

/**@brief The latest time before the end of a timeslot when timeslot can be extended. */
#define TIMESLOT_EXTENSION_MARGIN_MIN_US          (79UL)

/**@brief The timeslot signal types. */
enum TIMESLOT_SIGNAL
{
    TIMESLOT_SIGNAL_START            = 0, /**< This signal indicates the start of the timeslot.
                                               The signal will be executed in the same context as
                                               @ref ble_controller_TIMER0_IRQHandler. */
    TIMESLOT_SIGNAL_TIMER0           = 1, /**< This signal indicates the TIMER0 interrupt.
                                               The signal will be executed in the same context as
                                               @ref ble_controller_TIMER0_IRQHandler. */
    TIMESLOT_SIGNAL_RADIO            = 2, /**< This signal indicates the RADIO interrupt.
                                               The signal will be executed in the same context as
                                               @ref ble_controller_RADIO_IRQHandler. */
    TIMESLOT_SIGNAL_EXTEND_FAILED    = 3, /**< This signal indicates extend action failed.
                                               The signal will be executed in the same context as
                                               the previous signal. */
    TIMESLOT_SIGNAL_EXTEND_SUCCEEDED = 4, /**< This signal indicates extend action succeeded.
                                               The signal will be executed in the same context as
                                               the previous signal. */

    TIMESLOT_SIGNAL_BLOCKED          = 5, /**< The previous request was blocked. The signal will
                                               be executed in the same context as
                                               @ref ble_controller_low_prio_tasks_process. */
    TIMESLOT_SIGNAL_CANCELLED        = 6, /**< The previous request was cancelled. The signal will
                                               be executed in the same context as
                                               @ref ble_controller_low_prio_tasks_process.  */
    TIMESLOT_SIGNAL_SESSION_IDLE     = 7, /**< The timeslot session has no more pending requests.
                                               The signal will be executed in the same context as
                                               @ref ble_controller_low_prio_tasks_process. */
    TIMESLOT_SIGNAL_INVALID_RETURN   = 8, /**< The previous timeslot callback return value was invalid.
                                               The signal will be executed in the same context as
                                               the previous signal which had an invalid return value.
                                               The application should avoid to continuously provide
                                               invalid return values. Doing so, will lead to an
                                               infinite loop. */
};


/**@brief The actions requested by the signal callback.
 *
 *  This code gives instructions about what action to take when the signal callback has
 *  returned.
 */
enum TIMESLOT_SIGNAL_ACTION
{
    TIMESLOT_SIGNAL_ACTION_NONE            = 0,  /**< Return without action. */
    TIMESLOT_SIGNAL_ACTION_EXTEND          = 1,  /**< Request an extension of the current
                                                      timeslot event.
                                                      Maximum execution time for this action:
                                                      @ref TIMESLOT_EXTENSION_PROCESSING_TIME_MAX_US.
                                                      This action must be started at least
                                                      @ref TIMESLOT_EXTENSION_MARGIN_MIN_US before
                                                      the end of a timeslot event.
                                                      @note This signal action may only be used from
                                                      within a timeslot event. */
    TIMESLOT_SIGNAL_ACTION_END             = 2,  /**< End the current timeslot event.
                                                      @note This signal action may only be called
                                                      from within a timeslot event. */
    TIMESLOT_SIGNAL_ACTION_REQUEST         = 3,  /**< Request a new timeslot event.
                                                      @note If this signal action is used from within
                                                      a timeslot, the current timeslot event is closed. */
};


/**@brief Timeslot high frequency clock source configuration. */
enum TIMESLOT_HFCLK_CFG
{
    TIMESLOT_HFCLK_CFG_XTAL_GUARANTEED = 0, /**< The high frequency clock source is the external crystal
                                                 for the whole duration of the timeslot. This should be the
                                                 preferred option for events that use the radio or
                                                 require high timing accuracy.
                                                 @note The external crystal will automatically be
                                                 turned on and off at the beginning and end of the
                                                 timeslot. */
    TIMESLOT_HFCLK_CFG_NO_GUARANTEE    = 1, /**< This configuration allows for earlier and tighter
                                                 scheduling of timeslots. The RC oscillator may be
                                                 the clock source in part or for the whole duration
                                                 of the timeslot. The RC oscillator's accuracy must
                                                 therefore be taken into consideration.
                                                 @note If the application will use the radio peripheral
                                                 in timeslots with this configuration, it must ensure
                                                 that the crystal is running and stable before
                                                 starting the radio. */
};


/**@brief Timeslot event priorities. */
enum TIMESLOT_PRIORITY
{
    TIMESLOT_PRIORITY_HIGH   = 0,           /**< High priority. */
    TIMESLOT_PRIORITY_NORMAL = 1,           /**< Low priority. */
};

/**@brief Timeslot request type. */
enum TIMESLOT_REQUEST_TYPE
{
    TIMESLOT_REQ_TYPE_EARLIEST    = 0,      /**< Request timeslot as early as possible.
                                                 This should always be used for the first request
                                                 in a session.
                                                 @note It is not permitted to request an earliest
                                                 timeslot from within a timeslot. */
    TIMESLOT_REQ_TYPE_NORMAL      = 1,      /**< Normal timeslot request. */
};


/**@brief Parameters for a request for a timeslot as early as possible. */
typedef struct
{
    uint8_t       hfclk;        /**< High frequency clock source, see @ref TIMESLOT_HFCLK_CFG. */
    uint8_t       priority;     /**< The timeslot priority, see @ref TIMESLOT_PRIORITY. */
    uint32_t      length_us;    /**< The timeslot length, @sa @ref TIMESLOT_LENGTH_MIN_US,
                                     @sa @ref TIMESLOT_LENGTH_MAX_US. */
    uint32_t      timeout_us;   /**< Longest acceptable delay until the start of the requested
                                     timeslot, up to @ref TIMESLOT_EARLIEST_TIMEOUT_MAX_US
                                     microseconds. */
} timeslot_request_earliest_t;


/**@brief Parameters for a normal timeslot request. */
typedef struct
{
    uint8_t       hfclk;        /**< High frequency clock source, see @ref TIMESLOT_HFCLK_CFG. */
    uint8_t       priority;     /**< The timeslot priority, see @ref TIMESLOT_PRIORITY. */
    uint32_t      distance_us;  /**< Distance from the start of the previous timeslot
                                     up to @ref TIMESLOT_DISTANCE_MAX_US microseconds. */
    uint32_t      length_us;    /**< The timeslot length, @sa @ref TIMESLOT_LENGTH_MIN_US,
                                     @sa @ref TIMESLOT_LENGTH_MAX_US. */
} timeslot_request_normal_t;


/**@brief Timeslot request parameters. */
typedef struct
{
    uint8_t                       request_type;  /**< Type of request, see @ref TIMESLOT_REQUEST_TYPE. */
    union
    {
        timeslot_request_earliest_t earliest;      /**< Parameters for requesting a timeslot as
                                                        early as possible. */
        timeslot_request_normal_t   normal;        /**< Parameters for requesting a normal timeslot. */
    } params;
} timeslot_request_t;


/**@brief Return parameters of the timeslot signal callback. */
typedef struct
{
    uint8_t         callback_action;      /**< The action requested by the application when
                                               returning from the signal callback, see
                                               @ref TIMESLOT_SIGNAL_ACTION. */
    union
    {
        struct
        {
          timeslot_request_t *p_next;       /**< The request parameters for the next timeslot. */
        } request;                          /**< Additional parameters for return_code
                                                 @ref TIMESLOT_SIGNAL_ACTION_REQUEST. */
        struct
        {
          uint32_t             length_us;   /**< Requested extension of the timeslot duration.
                                                 The minimum time is
                                                 @ref TIMESLOT_EXTENSION_TIME_MIN_US). */
        } extend;                           /**< Additional parameters for return_code
                                                 @ref TIMESLOT_SIGNAL_ACTION_EXTEND. */
    } params;                             /**< Parameter union. */
} timeslot_signal_return_param_t;


/**@brief The timeslot signal callback type.
 *
 * @note In case of invalid return parameters, the timeslot will automatically end
 *       immediately after returning from the signal callback and the
 *       @ref TIMESLOT_SIGNAL_INVALID_RETURN event will be sent.
 * @note The returned struct pointer must remain valid after the signal callback
 *       function returns. For instance, this means that it must not point to a stack variable.
 *
 * @param[in] signal Type of signal, see @ref TIMESLOT_SIGNAL.
 *
 * @return Pointer to structure containing action requested by the application.
 */
typedef timeslot_signal_return_param_t * (*timeslot_callback_t) (uint32_t signal);


/**@brief Opens a session for timeslot requests.
 *
 * @note Only one session can be open at a time.
 * @note timeslot_signal_callback(@ref TIMESLOT_SIGNAL_START) will be called when the timeslot
 *       starts. From this point the RADIO, TIMER0, AAR, and CCM peripherals can be freely accessed
 *       by the application.
 * @note timeslot_signal_callback(@ref TIMESLOT_SIGNAL_TIMER0) is called whenever
 *       the TIMER0 interrupt occurs.
 * @note timeslot_signal_callback(@ref TIMESLOT_SIGNAL_RADIO) is called whenever the RADIO
 *       interrupt occurs.
 *
 * @param[in] timeslot_signal_callback The signal callback.
 *
 * @retval 0              Success
 * @retval - ::NRF_EAGAIN Session already open
 */
int32_t timeslot_session_open(timeslot_callback_t timeslot_signal_callback);


/**@brief Closes a session for timeslot requests.
 *
 * @note Any current timeslot will be finished before the session is closed.
 * @note If a timeslot is scheduled when the session is closed, it will be canceled.
 *
 * @retval 0              Success
 * @retval - ::NRF_EAGAIN Session already closed
 */
int32_t timeslot_session_close(void);


/**@brief Requests a timeslot.
 *
 * @note The first request in a session must always be of type @ref TIMESLOT_REQ_TYPE_EARLIEST.

 * @note Successful requests will result in timeslot_signal_callback_t(@ref TIMESLOT_SIGNAL_START).
 *       Unsuccessful requests will result in a @ref TIMESLOT_SIGNAL_BLOCKED event.
 * @note The jitter in the start time of the timeslots is +/- @ref TIMESLOT_START_JITTER_US us.
 * @note The timeslot_signal_callback_t(@ref TIMESLOT_SIGNAL_START) call has a latency relative to the
 *       specified timeslot start, but this does not affect the actual start time of the timeslot.
 * @note TIMER0 is reset at the start of the timeslot, and is clocked at 1MHz from the high frequency
 *       (16 MHz) clock source
 * @note The BLE Controller will neither access the RADIO peripheral nor the TIMER0 peripheral
 *       during the timeslot.
 *
 * @param[in] p_request Pointer to the request parameters.
 *
 * @retval 0              Success
 * @retval - ::NRF_EINVAL The parameters of p_request are not valid
 * @retval - ::NRF_EAGAIN Either
 *                        - The session is not open.
 *                        - The session is not IDLE. */
int32_t timeslot_request(timeslot_request_t const * p_request);


#ifdef __cplusplus
}
#endif


#endif /* TIMESLOT_H__ */
