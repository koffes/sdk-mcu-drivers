/**
 * @file bsp_driver_if.h
 *
 * @brief Functions and prototypes that define the BSP-to-Device Driver Interface
 *
 * @copyright
 * Copyright (c) Cirrus Logic 2019 All Rights Reserved, http://www.cirrus.com/
 *
 * This code and information are provided 'as-is' without warranty of any
 * kind, either expressed or implied, including but not limited to the
 * implied warranties of merchantability and/or fitness for a particular
 * purpose.
 *
 */

#ifndef BSP_DRIVER_IF_H
#define BSP_DRIVER_IF_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 * INCLUDES
 **********************************************************************************************************************/
#include <stdint.h>

/***********************************************************************************************************************
 * LITERALS & CONSTANTS
 **********************************************************************************************************************/

/**
 * @defgroup BSP_STATUS_
 * @brief Return values for all public and most private API calls
 *
 * @{
 */
#define BSP_STATUS_OK               (0)
#define BSP_STATUS_FAIL             (1)
/** @} */

/**
 * @defgroup BSP_TIMER_DURATION_
 * @brief Values used for calls to BSP timer APIs
 *
 * @see bsp_driver_if_t.set_timer
 *
 * @{
 */
#define BSP_TIMER_DURATION_1MS      (1)
#define BSP_TIMER_DURATION_2MS      (2)
#define BSP_TIMER_DURATION_10MS     (10)
#define BSP_TIMER_DURATION_2S       (2000)
/** @} */

/**
 * Value to indicate driving a GPIO low
 *
 * @see bsp_driver_if_t.set_gpio
 *
 */
#define BSP_GPIO_LOW                (0)

/**
 * Value to indicate driving a GPIO high
 *
 * @see bsp_driver_if_t.set_gpio
 *
 */
#define BSP_GPIO_HIGH               (1)

/***********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/

/**
 * Macro to extract byte from multi-byte word
 *
 * @param [in] A                multi-byte word
 * @param [in] B                zero-indexed byte position
 *
 * @return                      byte at position B in word A
 */
#define GET_BYTE_FROM_WORD(A, B)   ((A >> (B * 8)) & 0xFF)

/**
 * Macro to insert byte into multi-byte word
 *
 * @param [in, out] A           multi-byte word
 * @param [in] B                byte value
 * @param [in] C                zero-indexed byte position
 *
 * @return none
 */
#define ADD_BYTE_TO_WORD(A, B, C) \
{ \
    A &= (0xFFFFFF00 << (C * 8)); \
    A |= ((B & 0xFF) << (C * 8)); \
}

/***********************************************************************************************************************
 * ENUMS, STRUCTS, UNIONS, TYPEDEFS
 **********************************************************************************************************************/

/**
 * Callback type for BSP-to-Driver callbacks
 *
 * @param [in] status           Result of BSP call
 * @param [in] arg              Argument registered when BSP call was issued
 *
 * @return none
 *
 * @see BSP_STATUS_
 *
 */
typedef void (*bsp_callback_t)(uint32_t status, void* arg);

/**
 * BSP-to-Driver public API
 *
 * All API calls return a status @see CS35L41_STATUS_
 *
 */
typedef struct
{
    /**
     * Set GPIO to LOW/HIGH
     *
     * @param [in] gpio_id      ID for GPIO to change - can be defined in implementation header
     * @param [in] gpio_state   LOW or HIGH
     *
     * @return
     * - BSP_STATUS_FAIL        if gpio_id is invalid, or call to MCU HAL fails
     * - BSP_STATUS_OK          otherwise
     *
     * @see BSP_GPIO_LOW BSP_GPIO_HIGH
     *
     */
    uint32_t (*set_gpio)(uint32_t gpio_id, uint8_t gpio_state);

    /**
     * Toggle GPIO state
     *
     * @param [in] gpio_id      ID for GPIO to change - can be defined in implementation header
     *
     * @return
     * - BSP_STATUS_FAIL        if gpio_id is invalid, or call to MCU HAL fails
     * - BSP_STATUS_OK          otherwise
     *
     */
    uint32_t (*toggle_gpio)(uint32_t gpio_id);

    /**
     * Register GPIO Callback
     *
     * Register a callback for when a GPIO changes state.
     *
     * @param [in] gpio_id      ID for GPIO to change - can be defined in implementation header
     * @param [in] cb           pointer to callback function
     * @param [in] cb_arg       pointer to argument to use when calling callback
     *
     * @return
     * - BSP_STATUS_FAIL        if gpio_id is invalid, if any pointers are NULL
     * - BSP_STATUS_OK          otherwise
     *
     */
    uint32_t (*register_gpio_cb)(uint32_t gpio_id, bsp_callback_t cb, void *cb_arg);

    /**
     * Set a timer to expire
     *
     * @param [in] duration_ms  Duration of timer in milliseconds
     * @param [in] cb           pointer to callback function
     * @param [in] cb_arg       pointer to argument to use when calling callback
     *
     * @return
     * - BSP_STATUS_FAIL        if duration_ms is invalid, if any pointers are NULL
     * - BSP_STATUS_OK          otherwise
     *
     */
    uint32_t (*set_timer)(uint32_t duration_ms, bsp_callback_t cb, void *cb_arg);

    /**
     * Reset I2C Port used for a specific device
     *
     * Abort the current I2C transaction and reset the I2C peripheral.  This is required for quickly handling of
     * CS35L41 IRQ events.
     *
     * @param [in] bsp_dev_id   ID of the I2C device corresponding to the I2C peripheral to reset
     *
     * @return
     * - BSP_STATUS_FAIL        if bsp_dev_id is invalid
     * - BSP_STATUS_OK          otherwise
     *
     */
    uint32_t (*i2c_reset)(uint32_t bsp_dev_id);

    /**
     * Perform an I2C Write-Repeated Start-Read transaction
     *
     * This is the common way to read data from an I2C device with a register file, since the address of the
     * register to read must first be written to the device before reading any contents.
     *
     * Perform transaction in the order:
     * 1. I2C Start
     * 2. I2C write of \b write_length bytes from \b write_buffer
     * 3. I2C Repeated Start
     * 4. I2C read of \b read_length bytes into \b read_buffer
     * 5. I2C Stop
     *
     * BSP will decode \b bsp_dev_id to the correct I2C bus and I2C address.
     *
     * @param [in] bsp_dev_id       ID of the I2C device corresponding to the I2C peripheral to reset
     * @param [in] write_buffer     pointer to array of bytes to write
     * @param [in] write_length     total number of bytes in \b write_buffer
     * @param [in] read_buffer      pointer to array of bytes to load with I2C bytes read
     * @param [in] read_length      total number of bytes to read into \b read_buffer
     * @param [in] cb               pointer to callback function
     * @param [in] cb_arg           pointer to argument to use when calling callback
     *
     * @return
     * - BSP_STATUS_FAIL            if bsp_dev_id is invalid, if any portion of I2C transaction failed
     * - BSP_STATUS_OK              otherwise
     *
     */
    uint32_t (*i2c_read_repeated_start)(uint32_t bsp_dev_id,
                                        uint8_t *write_buffer,
                                        uint32_t write_length,
                                        uint8_t *read_buffer,
                                        uint32_t read_length,
                                        bsp_callback_t cb,
                                        void *cb_arg);

    /**
     * Perform I2C Write
     *
     * BSP will decode \b bsp_dev_id to the correct I2C bus and I2C address.
     *
     * @param [in] bsp_dev_id       ID of the I2C device corresponding to the I2C peripheral to reset
     * @param [in] write_buffer     pointer to array of bytes to write
     * @param [in] write_length     total number of bytes in \b write_buffer
     * @param [in] cb               pointer to callback function
     * @param [in] cb_arg           pointer to argument to use when calling callback
     *
     * @return
     * - BSP_STATUS_FAIL            if bsp_dev_id is invalid, if any portion of I2C transaction failed
     * - BSP_STATUS_OK              otherwise
     *
     */
    uint32_t (*i2c_write)(uint32_t bsp_dev_id,
                          uint8_t *write_buffer,
                          uint32_t write_length,
                          bsp_callback_t cb,
                          void *cb_arg);

    /**
     * Perform a Double-Buffered ("db") I2C Write
     *
     * This will first write the contents of \b write_buffer_0 to the I2C device, and then write the contents of
     * \b write_buffer_1.
     *
     * @param [in] bsp_dev_id       ID of the I2C device corresponding to the I2C peripheral to reset
     * @param [in] write_buffer_0   pointer to array of first batch of bytes to write
     * @param [in] write_length_0   total number of bytes in \b write_buffer_0
     * @param [in] write_buffer_1   pointer to array of second batch of bytes to write
     * @param [in] write_length_1   total number of bytes in \b write_buffer_1
     * @param [in] cb               pointer to callback function
     * @param [in] cb_arg           pointer to argument to use when calling callback
     *
     * @return
     * - BSP_STATUS_FAIL            if bsp_dev_id is invalid, if any portion of I2C transaction failed
     * - BSP_STATUS_OK              otherwise
     *
     */
    uint32_t (*i2c_db_write)(uint32_t bsp_dev_id,
                          uint8_t *write_buffer_0,
                          uint32_t write_length_0,
                          uint8_t *write_buffer_1,
                          uint32_t write_length_1,
                          bsp_callback_t cb,
                          void *cb_arg);
} bsp_driver_if_t;

/***********************************************************************************************************************
 * GLOBAL VARIABLES
 **********************************************************************************************************************/

/**
 * Pointer to BSP-to-Driver API implementation
 */
extern bsp_driver_if_t *bsp_driver_if_g;

/***********************************************************************************************************************
 * API FUNCTIONS
 **********************************************************************************************************************/

#endif // BSP_DRIVER_IF_H
