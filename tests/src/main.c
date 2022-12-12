/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <posix/time.h>
#include "headers/coap.h"
#include "headers/sntp.h"
#include "headers/status.h"
#include "headers/hort_thread.h"
#include "headers/text_message.h"
#include "headers/definitions.h"
/**
 * @brief Test Asserts
 *
 * This test verifies various assert macros provided by ztest.
 * 
 */

static void test_status(void) {
        status_set(RUNNING_OK);
        zassert_equal(RUNNING_OK, 2, "RUNNING_OK not set");
        zassert_equal(RUNNING_OK, status_get(), "status_get() not set");
}

static void test_clock(void) {
    k_msleep(30000);
    set_clock();
        struct timespec currenttime;
        query_sntp();
        clock_gettime(CLOCK_REALTIME, &currenttime);
        zassert_true((currenttime.tv_sec > 1657892557), "Year less than 2022");
}

static void test_settings_get(void) {
    k_msleep(30000);
    static struct CoapSettCtx sett_ctx = {
        .settings_type = 0,
        .channel = 0,
        .value = 0,
        .updated_at = 0};
    coap_get_sett(&sett_ctx);
    zassert_true((sett_ctx.updated_at > 1), "No new settings downloaded");
}

static void test_sensor_post(void) {
        zassert_true(1, "1 was false");
}

static void test_neighbordata_post(void) {
                int ret = horti_thread_send_neighbordata();
                zassert_false(ret, "Neighbordata sending failed");
}

static void test_text_message(void) {
    char msgtext[] = "test av litt text";
    char msgtext2[] = "test av litt text2";
        struct timespec currenttime;
        clock_gettime(CLOCK_REALTIME, &currenttime);
    const struct text_message msg = {
        .receiver_id = 1,
        .sender_id = 3,
        .display_at = currenttime.tv_sec -1,
        .text_len = sizeof(msgtext),
        .text = &msgtext[0],
    };
    const struct text_message msg2 = {
        .receiver_id = 1,
        .sender_id = 3,
        .display_at = currenttime.tv_sec +1,
        .text_len = sizeof(msgtext),
        .text = &msgtext2[0],
    };

    for (int i = 0; i < 10; i++) {
        if (i != 5) {
            message_store(&msg);
        } else {
            message_store(&msg2);
        }
    }
    int unread = message_get_count(WAITING);
    zassert_equal(unread, 9, "unread messages not correct");
    for (int i = 0; i < unread; i++) {
        int msgnext = message_get_next();
         struct text_message *msgret = NULL;
        msgret = message_get(msgnext);
        printk("msgnext: '%i' \n", msgnext);
        zassert_not_equal(msgret, NULL, "returned message is nullpointer");
        zassert_false(strcmp(msg.text, msgret->text), "Message not eqal");
        message_free(msgnext);
    }
}

static void test_get_message(void) {
    printk("start test");
    char txt[] = "hoi hoi ahoy";
    const struct text_message msg = {
        .msg_id = 61,
        .status = INFO,
        .receiver_id = 1,
        .sender_id = 5,
        .display_at = 0,
        .updated_at = 0,
        .text_len = sizeof txt,
        .text = &txt[0],
    };
        int ret = coap_post_message(1, &msg);
    zassert_false(ret, "error on coap message post");
    ret = coap_get_msg();
    zassert_false(ret, "error on coap message get");
}
void test_main(void) {
        ztest_test_suite(framework_tests,
                ztest_unit_test(test_status),
                ztest_unit_test(test_clock),
                ztest_unit_test(test_sensor_post),
                ztest_unit_test(test_neighbordata_post),
                ztest_unit_test(test_text_message),
                ztest_unit_test(test_get_message),
                ztest_unit_test(test_settings_get),
                ztest_unit_test(test_sensor_post) );

        ztest_run_test_suite(framework_tests);
}

