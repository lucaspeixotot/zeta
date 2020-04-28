/* ***************************************************************** */
/*                      FILE GENERATED BY ZetaCLI                    */
/*                         DON'T EDIT THIS FILE                      */
/* ***************************************************************** */

/**
 * @file   zeta.template.h
 * @author Rodrigo Peixoto
 * @author Lucas Peixoto <lucaspeixotoac@gmail.com>
 *
 * @brief Zeta header file
 *
 *
 */


#ifndef ZETA_H_
#define ZETA_H_


#include <stddef.h>
#include <zephyr.h>
#include <zephyr/types.h>

/**
 * @brief Stack size that is used in Zeta thread that manages the
 * persistent data.
 *
 */
#define ZETA_THREAD_NVS_STACK_SIZE 512

/**
 * @brief Stack size that is used in Zeta thread that manages
 * callback calls.
 *
 */
#define ZETA_THREAD_STACK_SIZE 512

/**
 * @brief Priority that is used for all Zeta threads.
 *
 */
#define ZETA_THREAD_PRIORITY 0

/**
 * @brief Get variable reference and size easily to use in
 * Zeta API.
 *
 * @param x variable name
 *
 */
#define ZETA_VARIABLE_REF_SIZE(x) (u8_t *) (&x), sizeof(x)

/**
 * @brief Check if _v value is equal to _c, otherwise _err will be
 * returned and a message will be sent to LOG.
 *
 * @param _v Value
 * @param _c Condition
 * @param _err Error code
 *
 */
#define ZETA_CHECK_VAL(_p, _e, _err, ...) \
    if (_p == _e) {                       \
        LOG_INF(__VA_ARGS__);             \
        return _err;                      \
    }

/**
 * @brief Check if _v is true, otherwise _err will be returned and a
 * message will be sent to LOG.
 *
 * @param _v Value
 * @param _err Error code
 *
 * @return
 */
#define ZETA_CHECK(_p, _err, ...) \
    if (_p) {                     \
        LOG_INF(__VA_ARGS__);     \
        return _err;              \
    }

//$channels_enum

/**
 * @brief zeta_callback_f define the callback function type of Zeta.
 *
 * @param id Channel Id.
 *
 */
typedef void (*zeta_callback_f)(zeta_channel_e id);

/**
 * @brief Define pendent options that a channel can have.
 */
union opt_data {
    struct {
        u8_t pend_persistent : 1; /**< Active represent that channel must be saved in
                                     flash by zeta_thread_nvs */
        u8_t pend_callback : 1;   /**< Active represent that services callbacks from
                                     subscribers must be called by zeta_thread */
    } field;
    u8_t data; /**< Raw data */
};

/**
 * @brief Define Zeta channel type
 */
struct zeta_channel {
    const char *name;                         /**< Channel name */
    u8_t *data;                               /**< Channel raw data */
    int (*validate)(u8_t *data, size_t size); /**< Valid data sent to be set to channel */
    int (*pre_get)(zeta_channel_e id, u8_t *channel_value,
                   size_t size); /**< Called before some get call */
    int (*get)(zeta_channel_e id, u8_t *channel_value, size_t size); /**< Get call */
    int (*pos_get)(zeta_channel_e id, u8_t *channel_value,
                   size_t size); /**< Called after some get call */
    int (*pre_set)(zeta_channel_e id, u8_t *channel_value,
                   size_t size); /**< Called before some set call */
    int (*set)(zeta_channel_e id, u8_t *channel_value, size_t size); /**< Set call */
    int (*pos_set)(zeta_channel_e id, u8_t *channel_value,
                   size_t size);      /**< Called after some set call */
    u8_t size;                        /**< Channel size */
    u8_t persistent;                  /**< Persistent type */
    union opt_data opt;               /**< Pendent options */
    struct k_sem *sem;                /**< Preserve shared-memory */
    const k_tid_t *publishers_id;     /**< Publishers Ids */
    zeta_callback_f *subscribers_cbs; /**< Subscribers callbacks */
    zeta_channel_e id;                /**< Channel Id */
};
typedef struct zeta_channel zeta_channel_t;

/**
 * @brief Return the channel size.
 *
 * @param id Channel Id
 * @param error Handle possible errors
 *
 * @return Channel size
 */
size_t zeta_channel_size(zeta_channel_e id, int *error);

/**
 * @brief Return the channel name.
 *
 * @param id Channel Id
 * @param error Handle possible errors
 *
 * @return Channel name
 */
const char *zeta_channel_name(zeta_channel_e id, int *error);

/**
 * @brief Get channel value.
 *
 * @param id Channel Id
 * @param channel_value Handle channel value
 * @param size Channel size
 *
 * @return Error code
 * @retval -ENODATA The channel was not found
 * @retval -EFAULT Channel value is NULL
 * @retval -EPERM  Channel hasn't get function implemented
 * @retval -EINVAL Size passed is different to channel size
 */
int zeta_channel_get(zeta_channel_e id, u8_t *channel_value, size_t size);

/**
 * @brief Set channel value.
 *
 * @param id Channel Id
 * @param channel_value New channel value
 * @param size Channel size
 *
 * @return Error code
 * @retval -ENODATA The channel was not found
 * @retval -EACCESS Current thread hasn't permission to set this channel
 * @retval -EFAULT Channel value is NULL
 * @retval -EPERM Channel is read only
 * @retval -EINVAL Size passed is different to channel size
 * @retval -EAGAIN Valid function returns false
 */
int zeta_channel_set(zeta_channel_e id, u8_t *channel_value, size_t size);

#endif
