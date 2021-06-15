/*!
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <sys/crc.h>
#include <sys/printk.h>
#include <zephyr.h>
#include <zt_uart.h>
#include "devicetree.h"
#include "kernel.h"
#include "zeta.h"

#define UART_DEVICE_NAME CONFIG_UART_CONSOLE_ON_DEV_NAME

#define MAX_MESSAGE_LENGTH 255

K_MSGQ_DEFINE(channel_queue, 1, 5, 1);

#define ZT_ISC_PKT_ASSERT_EQ(actual, expected, ret) \
    if (actual != expected) {                       \
        return ret;                                 \
    }

#define ZT_ISC_PKT_ASSERT(cond, ret) \
    if (!(cond)) {                   \
        return ret;                  \
    }

#define SELF_TEST_INIT()             \
    uint64_t _self_test_result  = 0; \
    uint8_t _self_test_sequence = 0
#define SELF_TEST_ASSERT(cond) _self_test_result |= !(cond) << _self_test_sequence++
#define SELF_TEST_FINISH() return _self_test_result;

typedef enum {
    OP_READ,
    OP_WRITE,
    OP_READ_RESPONSE,
    OP_WRITE_RESPONSE,
    OP_UPDATE,
    OP_DEBUG
} zt_isc_net_pkt_op_t;

typedef enum { DATA_UNAVALABLE, DATA_AVAILABLE } pkt_data_availability_t;
typedef enum { STATUS_OK, STATUS_FAILED } pkt_status_t;

struct zt_isc_net_pkt_header {
    uint8_t channel : 8; /**!> 256 channels available*/
    uint8_t
        op : 3; /**!> 0: read, 1: write, 2: read response, 3: write response, 4: update */
    uint8_t status : 4;   /**!> 0: ok, 1: failed */
    uint8_t has_data : 1; /**!> 0: no data, 1: contains data */
};


struct zt_isc_net_pkt_header_message_info {
    uint8_t crc : 8;  /**!> CCITT 8, polynom 0x07, initial value 0x00 */
    uint8_t size : 8; /**!> data size */
};

struct zt_isc_net_pkt {
    struct zt_isc_net_pkt_header header;
    struct zt_isc_net_pkt_header_message_info message_info;
    uint8_t *message;
};

void zt_isc_net_pkt_clear(struct zt_isc_net_pkt *self)
{
    memset(&self->header, 0, sizeof(struct zt_isc_net_pkt_header));
    memset(&self->message_info, 0, sizeof(struct zt_isc_net_pkt_header));
    memset(self->message, 0, 255);
}

void zt_isc_net_pkt_send(struct zt_isc_net_pkt *self)
{
    uart_write((uint8_t *) &self->header, sizeof(struct zt_isc_net_pkt_header));
    uart_write((uint8_t *) &self->message_info,
               sizeof(struct zt_isc_net_pkt_header_message_info));
    uart_write(self->message, self->message_info.size);
}

int zt_isc_net_pkt_set_message(struct zt_isc_net_pkt *self, char *message,
                               size_t message_size)
{
    printk("%d, %d", self->message_info.size, message_size);
    if (self->message_info.size < message_size) {
        return -EINVAL;
    }
    self->message_info.size = message_size;
    self->header.has_data   = DATA_AVAILABLE;
    self->message_info.crc  = crc8(message, message_size, 0x07, 0x00, 0);
    memcpy(self->message, message, message_size);
    return 0;
}

void zt_isc_net_pkt_calc_crc(struct zt_isc_net_pkt *self)
{
    self->message_info.crc = crc8(self->message, self->message_info.size, 0x07, 0x00, 0);
}


/* This is just a stub and must be replaced by the NUMBER_OF_CHANNELS generated by zeta */
#define NUMBER_OF_CHANNELS 3

uint8_t zt_isc_net_pkt_header_is_valid(struct zt_isc_net_pkt_header *header)
{
    if (header->channel > NUMBER_OF_CHANNELS) {
        return 0;
    }
    if (header->op > OP_UPDATE) {
        return 0;
    }
    if (header->status > 1) {
        return 0;
    }
    return 1;
}

uint8_t zt_isc_net_pkt_message_is_valid(struct zt_isc_net_pkt *pkt)
{
    if (pkt->header.has_data) {
        uint8_t expected_crc = crc8(pkt->message, pkt->message_info.size, 0x07, 0x00, 0);
        if (expected_crc != pkt->message_info.crc) {
            return 0;
        }
    } else {
        return 0;
    }
    return 1;
}


int digest_byte(uint8_t data, struct zt_isc_net_pkt *pkt)
{
    typedef enum { OP, MESSAGE_INFO, MESSAGE, ERROR } digestion_state_t;
    static uint8_t buffer[MAX_MESSAGE_LENGTH] = {0};
    static uint8_t *iter                      = buffer;
    static const uint8_t *end                 = buffer + MAX_MESSAGE_LENGTH;
    static digestion_state_t state            = OP;
    int pkt_ready                             = 0;
    int cleanup_needed                        = 0;
    if (iter < end) {
        *iter = data;
        ++iter;
        switch (state) {
        case OP: {
            if ((iter - buffer) == 2) {
                memcpy(&pkt->header, buffer, sizeof(struct zt_isc_net_pkt_header));
                /*! The pkt can be discarded here. If the zt_isc_net_pkt_header_is_valid
                 * returns 0 the packet will be discarded. */
                if (zt_isc_net_pkt_header_is_valid(&pkt->header)) {
                    if (pkt->header.has_data == DATA_AVAILABLE) {
                        state = MESSAGE_INFO;
                    } else {
                        pkt_ready = 1;
                    }
                }
                cleanup_needed = 1;
            }
        } break;
        case MESSAGE_INFO: {
            if ((iter - buffer) == 2) {
                memcpy(&pkt->message_info, buffer,
                       sizeof(struct zt_isc_net_pkt_header_message_info));
                state          = MESSAGE;
                cleanup_needed = 1;
            }
        } break;
        case MESSAGE: {
            /* It would be good to enable a timer here.
                        If the data takes too much time, it will drop that */
            if ((iter - buffer) == pkt->message_info.size) {
                memcpy(pkt->message, buffer, pkt->message_info.size);
                if (zt_isc_net_pkt_message_is_valid(pkt)) {
                    pkt_ready = 1;
                }
                state          = OP;
                cleanup_needed = 1;
            }
        } break;
        default: {
            state = ERROR;
        }
        }
    }
    if (cleanup_needed) {
        memset(buffer, 0, iter - buffer);
        iter = buffer;
    }
    return pkt_ready;
}


#define ZT_ISC_NET_PKT_WITH_MSG_SIZE(_size)                                    \
    {                                                                          \
        .header = {0}, .message_info = {.size = _size}, .message = (uint8_t[]) \
        {                                                                      \
            [0 ...(_size - 1)] = 0                                             \
        }                                                                      \
    }

uint64_t digest_byte_test()
{
    struct zt_isc_net_pkt pkt = ZT_ISC_NET_PKT_WITH_MSG_SIZE(2);

    uint8_t stub[] = {0x01, 0x84, 0x22, 0x02, 0xa4, 0xa1};

    uint8_t data_ready = 0;
    for (uint8_t *iter = stub; iter < (stub + sizeof(stub)); ++iter) {
        data_ready = digest_byte(*iter, &pkt);
    }

    SELF_TEST_INIT();
    SELF_TEST_ASSERT(pkt.header.channel == 1);
    SELF_TEST_ASSERT(pkt.header.op == 4);
    SELF_TEST_ASSERT(pkt.header.status == 0);
    SELF_TEST_ASSERT(pkt.header.has_data == 1);
    SELF_TEST_ASSERT(pkt.message_info.crc == 34);
    SELF_TEST_ASSERT(pkt.message_info.size == 2);
    SELF_TEST_ASSERT(memcmp(pkt.message, stub + 4, 2) == 0);
    SELF_TEST_FINISH();
}

static uint8_t SELF_TEST()
{
    uint8_t test_failed       = 0;
    uint64_t self_test_result = digest_byte_test();
    size_t written_size       = 0;

    struct zt_isc_net_pkt pkt = ZT_ISC_NET_PKT_WITH_MSG_SIZE(255);
    pkt.header.op             = OP_DEBUG;
    pkt.header.has_data       = DATA_AVAILABLE;
    if (self_test_result) {
        written_size =
            snprintk(pkt.message, pkt.message_info.size,
                     "Self test failed with code: 0x%08llX\n", self_test_result);
        test_failed = 1;
    } else {
        written_size = snprintk(pkt.message, pkt.message_info.size,
                                "Self test ok. Target ISC running...\n");
    }
    pkt.message_info.size = written_size;
    zt_isc_net_pkt_calc_crc(&pkt);
    zt_isc_net_pkt_send(&pkt);
    return test_failed;
}

void ISC_TX_service_callback(zt_channel_e id)
{
    k_msgq_put(&channel_queue, &id, K_NO_WAIT);
}

void ISC_service_callback(zt_channel_e id)
{
}

void ISC_TX_task(void)
{
    zt_channel_e id = ZT_CHANNEL_COUNT;
    while (1) {
        if (!k_msgq_get(&channel_queue, &id, K_FOREVER)) {
            switch (id) {
            case ZT_LIGHT_STATUS_CHANNEL: {
                struct zt_isc_net_pkt pkt =
                    ZT_ISC_NET_PKT_WITH_MSG_SIZE(sizeof(zt_data_light_status_msg_t));
                pkt.header.op           = OP_UPDATE;
                pkt.header.channel      = id;
                zt_data_t *light_status = ZT_DATA_LIGHT_STATUS_MSG(0);
                zt_chan_read(ZT_LIGHT_STATUS_CHANNEL, light_status);
                int err = zt_isc_net_pkt_set_message(&pkt, (char *) light_status,
                                                     sizeof(zt_data_light_status_msg_t));
                if (err) {
                    printk("Error set %d\n", err);
                }
                zt_isc_net_pkt_send(&pkt);
                break;
            }
            default: {
                printk("D: tx task invalid channel");
            }
            }
        }
    }
}

void ISC_task(void)
{
    char dev_name[] = UART_DEVICE_NAME;
    if (uart_open(dev_name)) {
        return;
    }

    if (SELF_TEST()) {
        return;
    }

    char data                 = 0;
    struct zt_isc_net_pkt pkt = ZT_ISC_NET_PKT_WITH_MSG_SIZE(255);
    pkt.header.op             = OP_DEBUG;
    zt_isc_net_pkt_set_message(&pkt, "hello world", sizeof("hello world!"));

    while (1) {
        if (!k_msgq_get(uart_get_input_msgq(), &data, K_FOREVER)) {
            if (digest_byte(data, &pkt)) {
                /* Process the pkt */
                switch (pkt.header.op) {
                case OP_WRITE:
                case OP_UPDATE: {
                    if (pkt.header.channel == ZT_SUNLIGHT_LEVEL_CHANNEL) {
                        zt_data_t *s = (zt_data_t *) pkt.message;
                        int err      = zt_chan_pub(pkt.header.channel, s);
                        if (err) {
                            printk("publishing status %d\n", err);
                        }
                        // printk("Size = %u, level = %d\n", s->sunlight_level_msg.size,
                        //        s->sunlight_level_msg.value.level);
                        // printk("write or update\n");
                        // for (int i = 0; i < pkt.message_info.size; ++i) {
                        //     printk("%02X ", pkt.message[i]);
                        // }
                        // printk("\n");
                    }
                }
                }
                // zt_isc_net_pkt_send(&pkt);
                zt_isc_net_pkt_clear(&pkt);
            }
        }
    }
}

ZT_SERVICE_DECLARE(ISC, ISC_task, ISC_service_callback);
ZT_SERVICE_DECLARE(ISC_TX, ISC_TX_task, ISC_TX_service_callback);