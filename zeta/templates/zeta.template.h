/* ***************************************************************** */
/*                      FILE GENERATED BY ZetaCLI                    */
/*                         DON'T EDIT THIS FILE                      */
/* ***************************************************************** */

#ifndef ZETA_H_
#define ZETA_H_


#include <stddef.h>
#include <zephyr.h>
#include <zephyr/types.h>

/**
 * @brief Stack size that is used in Zeta thread that manages
 * channels callback calls.
 *
 */
#define ZT_MONITOR_THREAD_STACK_SIZE 512


/**
 * @brief Storage sleep time.
 *
 */
#define ZT_STORAGE_SLEEP_TIME $storage_period

/**
 * @brief Channels thread priority
 *
 */
#define ZT_MONITOR_THREAD_PRIORITY 0

#ifdef CONFIG_ZETA_STORAGE
/**
 * @brief Stack size that is used in Zeta thread that manages the
 * persistent data.
 *
 */
#define ZT_STORAGE_THREAD_STACK_SIZE 1024

/**
 * @brief Storage thread priority.
 *
 */
#define ZT_STORAGE_THREAD_PRIORITY 2
#endif

#ifdef CONFIG_ZETA_FORWARDER
/**
 * @brief Forwarder thread priority.
 *
 */
#define ZT_FORWARDER_THREAD_PRIORITY 1

/**
 * @brief Stack size that is used in Zeta thread that manages
 * the forwarder of messages between services and channels.
 *
 */
#define ZT_FORWARDER_THREAD_STACK_SIZE 512

#define ZT_FWD_OP_READ 0
#define ZT_FWD_OP_PUBLISH 1
#define ZT_FWD_OP_CALLBACK 2
#define ZT_FWD_OP_SAVED 3
#endif

/**
 * @brief Initialize a zeta service.
 *
 * @param _name Service name
 * @param _task Task pointer function
 * @param _cb Callback to be called when some subscribed channel change
 *
 */
#define ZT_SERVICE_INIT(_name, _task, _cb)                                     \
    zt_service_t _name##_service = {                                           \
        .id = ZT_##_name##_SERVICE, .name = #_name, .cb = _cb};                \
    K_THREAD_STACK_DEFINE(_k_thread_stack_##_name, _name##_STACK_SIZE);        \
    Z_STRUCT_SECTION_ITERABLE(_static_thread_data, _k_thread_data_##_name) =   \
        Z_THREAD_INITIALIZER(&_name##_service.thread, _k_thread_stack_##_name, \
                             _name##_STACK_SIZE, _task, NULL, NULL, NULL,      \
                             _name##_TASK_PRIORITY, 0, 0, NULL, _name);

/**
 * @brief Read variable reference and size easily to use in
 * Zeta API.
 *
 * @param x variable name
 *
 */
#define ZT_VARIABLE_REF_SIZE(x) (u8_t *) (&x), sizeof(x)

/**
 * @brief Check if _v value is equal to _c, otherwise _err will be
 * returned and a message will be sent to LOG.
 *
 * @param _v Value
 * @param _c Condition
 * @param _err Error code
 *
 */
#define ZT_CHECK_VAL(_p, _e, _err, ...) \
    if (_p == _e) {                     \
        LOG_INF(__VA_ARGS__);           \
        return _err;                    \
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
#define ZT_CHECK(_p, _err, ...) \
    if (_p) {                   \
        LOG_INF(__VA_ARGS__);   \
        return _err;            \
    }


#define ZT_DATA_S8(data)               \
    (zt_data_t *) (zt_data_s8_t[])     \
    {                                  \
        {                              \
            sizeof(s8_t), (s8_t)(data) \
        }                              \
    }

#define ZT_DATA_U8(data)               \
    (zt_data_t *) (zt_data_u8_t[])     \
    {                                  \
        {                              \
            sizeof(u8_t), (u8_t)(data) \
        }                              \
    }

#define ZT_DATA_S16(data)                \
    (zt_data_t *) (zt_data_s16_t[])      \
    {                                    \
        {                                \
            sizeof(s16_t), (s16_t)(data) \
        }                                \
    }

#define ZT_DATA_U16(data)                \
    (zt_data_t *) (zt_data_u16_t[])      \
    {                                    \
        {                                \
            sizeof(u16_t), (u16_t)(data) \
        }                                \
    }

#define ZT_DATA_S32(data)                \
    (zt_data_t *) (zt_data_s32_t[])      \
    {                                    \
        {                                \
            sizeof(s32_t), (s32_t)(data) \
        }                                \
    }

#define ZT_DATA_U32(data)                \
    (zt_data_t *) (zt_data_u32_t[])      \
    {                                    \
        {                                \
            sizeof(u32_t), (u32_t)(data) \
        }                                \
    }

#define ZT_DATA_S64(data)                \
    (zt_data_t *) (zt_data_s64_t[])      \
    {                                    \
        {                                \
            sizeof(s64_t), (s64_t)(data) \
        }                                \
    }

#define ZT_DATA_U64(data)                \
    (zt_data_t *) (zt_data_u64_t[])      \
    {                                    \
        {                                \
            sizeof(u64_t), (u64_t)(data) \
        }                                \
    }

#define ZT_DATA_BYTES(_size, data, ...) \
    (zt_data_t *) (struct {             \
        size_t size;                    \
        u8_t value[_size];              \
    }[])                                \
    {                                   \
        {                               \
            _size,                      \
            {                           \
                data, ##__VA_ARGS__     \
            }                           \
        }                               \
    }

typedef struct {
    size_t size;
    s8_t value;
} zt_data_s8_t;

typedef struct {
    size_t size;
    u8_t value;
} zt_data_u8_t;

typedef struct {
    size_t size;
    s16_t value;
} zt_data_s16_t;

typedef struct {
    size_t size;
    u16_t value;
} zt_data_u16_t;

typedef struct {
    size_t size;
    s32_t value;
} zt_data_s32_t;

typedef struct {
    size_t size;
    u32_t value;
} zt_data_u32_t;

typedef struct {
    size_t size;
    s64_t value;
} zt_data_s64_t;

typedef struct {
    size_t size;
    u64_t value;
} zt_data_u64_t;

typedef struct {
    size_t size;
    u8_t value[];
} zt_data_bytes_t;

union data {
    zt_data_s8_t s8;
    zt_data_u8_t u8;
    zt_data_s16_t s16;
    zt_data_u16_t u16;
    zt_data_s32_t s32;
    zt_data_u32_t u32;
    zt_data_s64_t s64;
    zt_data_u64_t u64;
    zt_data_bytes_t bytes;
};

typedef union data zt_data_t;

// <ZT_CODE_INJECTION>$channels_enum// </ZT_CODE_INJECTION>

// <ZT_CODE_INJECTION>$services_enum// </ZT_CODE_INJECTION>

/**
 * @brief zeta_callback_f define the callback function type of Zeta.
 *
 * @param id Channel Id.
 *
 */
typedef void (*zt_callback_f)(zt_channel_e id);

/**
 * @brief Define Zeta service type
 */
struct zt_service {
    zt_service_e id;        /**< Service ID */
    const char *name;       /**< Service name */
    struct k_thread thread; /**< Service RTOS thread */
    zt_callback_f cb;       /**< Service callback */
};
typedef struct zt_service zt_service_t;

struct zt_isc_packet {
    u32_t id;
    u8_t service_id;
    u8_t channel_id;
    u8_t op;
    u8_t size;
    u8_t message[$max_channel_size];
} __attribute__((packed));
typedef struct zt_isc_packet zt_isc_packet_t;

/**
 * @brief Define pendent options that a channel can have.
 */
union flag_data {
    struct {
        u8_t pend_persistent : 1; /**< Active represent that channel must be saved in
                                     flash by zeta_thread_nvs */
        u8_t pend_callback : 1;   /**< Active represent that services callbacks from
                                     subscribers must be called by zeta_thread */
        u8_t on_changed : 1;      /**< Active represent that the service callback will
                                            be called on change and not on update */
    } field;
    u8_t data; /**< Raw data */
};

/**
 * @brief Define Zeta channel type
 */
struct zt_channel {
    const char *name; /**< Channel name */
    u8_t *data;       /**< Channel raw data */
    u8_t read_only;
    u8_t size;                  /**< Channel size */
    u8_t persistent;            /**< Persistent type */
    zt_channel_e id;            /**< Channel Id */
    union flag_data flag;       /**< Options */
    struct k_sem *sem;          /**< Preserve shared-memory */
    zt_service_t **publishers;  /**< Publishers */
    zt_service_t **subscribers; /**< Subscribers */
};
typedef struct zt_channel zt_channel_t;

/**
 * @brief Return the channel size.
 *
 * @param id Channel Id
 * @param error Handle possible errors
 *
 * @return Channel size
 */
size_t zt_channel_size(zt_channel_e id, int *error);

/**
 * @brief Return the channel name.
 *
 * @param id Channel Id
 * @param error Handle possible errors
 *
 * @return Channel name
 */
const char *zt_channel_name(zt_channel_e id, int *error);

/**
 * @brief Read channel value.
 *
 * @param id Channel Id
 * @param channel_data pointer to a zt_data_t where the data will be retrieved.
 *
 * @return Error code
 * @retval -ENODATA The channel was not found
 * @retval -EFAULT Channel value is NULL
 * @retval -EPERM  Channel hasn't read function implemented
 * @retval -EINVAL Size passed is different to channel size
 */
int zt_chan_read(zt_channel_e id, zt_data_t *channel_data);

/**
 * @brief Publish channel value.
 *
 * @param id Channel Id
 * @param channel_data pointer to a zt_data_t where the data is.
 *
 * @return Error code
 * @retval -ENODATA The channel was not found
 * @retval -EACCESS Current thread hasn't permission to publish this channel
 * @retval -EFAULT Channel value is NULL
 * @retval -EPERM Channel is read only
 * @retval -EINVAL Size passed is different to channel size
 * @retval -EAGAIN Valid function returns false
 */
int zt_chan_pub(zt_channel_e id, zt_data_t *channel_data);

// <ZT_CODE_INJECTION>$services_reference// </ZT_CODE_INJECTION>

#endif
