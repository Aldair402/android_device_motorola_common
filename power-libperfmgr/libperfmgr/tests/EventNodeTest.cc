/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android-base/file.h>
#include <android-base/stringprintf.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <thread>

#include "perfmgr/EventNode.h"

namespace android {
namespace perfmgr {

using std::literals::chrono_literals::operator""ms;

constexpr double kTIMING_TOLERANCE_MS = std::chrono::milliseconds(25).count();
constexpr auto kSLEEP_TOLERANCE_MS = 2ms;

// Test init with no default value
TEST(EventNodeTest, NoInitDefaultTest) {
    std::string node_val = "uninitialize";
    auto update_callback = [&node_val](const std::string &, const std::string &,
                                       const std::string &val) { node_val = val; };
    EventNode t("EventName", "<Event>:Node", {{"value0"}, {"value1"}, {"value2"}}, 1, false,
                update_callback);
    t.Update(false);
    EXPECT_EQ(node_val, "uninitialize");
}

// Test init with default value
TEST(EventNodeTest, InitDefaultTest) {
    std::string node_val = "uninitialize";
    auto update_callback = [&node_val](const std::string &, const std::string &,
                                       const std::string &val) { node_val = val; };
    EventNode t("EventName", "<Event>:Node", {{"value0"}, {"value1"}, {"value2"}}, 1, true,
                update_callback);
    t.Update(false);
    EXPECT_EQ(node_val, "value1");
    EventNode t2("EventName", "<Event>:Node", {{"value0"}, {"value1"}, {"value2"}}, 0, true,
                 update_callback);
    t2.Update(false);
    EXPECT_EQ(node_val, "value0");
}

// Test DumpToFd
TEST(EventNodeTest, DumpToFdTest) {
    std::string node_val = "uninitialize";
    auto update_callback = [&node_val](const std::string &, const std::string &,
                                       const std::string &val) { node_val = val; };
    EventNode t("EventName", "<Event>:Node", {{"value0"}, {"value1"}, {"value2"}}, 1, true,
                update_callback);
    t.Update(false);
    t.Update(false);
    TemporaryFile dumptf;
    t.DumpToFd(dumptf.fd);
    fsync(dumptf.fd);
    std::string buf(android::base::StringPrintf(
            "Node Name\t"
            "Event Path\t"
            "Current Index\t"
            "Current Value\n"
            "%s\t%s\t%zu\t%s\n",
            "EventName", "<Event>:Node", static_cast<size_t>(1), "value1"));
    std::string s;
    EXPECT_TRUE(android::base::ReadFileToString(dumptf.path, &s)) << strerror(errno);
    EXPECT_EQ(buf, s);
}

// Test GetValueIndex
TEST(EventNodeTest, GetValueIndexTest) {
    std::string node_val = "uninitialize";
    auto update_callback = [&node_val](const std::string &, const std::string &,
                                       const std::string &val) { node_val = val; };
    EventNode t("EventName", "<Event>:Node", {{"value0"}, {"value1"}, {"value2"}}, 1, false,
                update_callback);
    std::size_t index = 0;
    EXPECT_TRUE(t.GetValueIndex("value2", &index));
    EXPECT_EQ(2u, index);
    index = 1234;
    EXPECT_FALSE(t.GetValueIndex("NON_EXIST", &index));
    EXPECT_EQ(1234u, index);
}

// Test GetValues
TEST(EventNodeTest, GetValuesTest) {
    std::string node_val = "uninitialize";
    auto update_callback = [&node_val](const std::string &, const std::string &,
                                       const std::string &val) { node_val = val; };
    EventNode t("EventName", "<Event>:Node", {{"value0"}, {"value1"}, {"value2"}}, 1, false,
                update_callback);
    std::vector values = t.GetValues();
    EXPECT_EQ(3u, values.size());
    EXPECT_EQ("value0", values[0]);
    EXPECT_EQ("value1", values[1]);
    EXPECT_EQ("value2", values[2]);
}

// Test get more properties
TEST(EventNodeTest, GetPropertiesTest) {
    std::string node_val = "uninitialize";
    auto update_callback = [&node_val](const std::string &, const std::string &,
                                       const std::string &val) { node_val = val; };
    std::string test_name = "TESTREQ_1";
    std::string test_path = "TEST_PATH";
    EventNode t(test_name, test_path, {}, 0, false, update_callback);
    EXPECT_EQ(test_name, t.GetName());
    EXPECT_EQ(test_path, t.GetPath());
    EXPECT_EQ(0u, t.GetValues().size());
    EXPECT_EQ(0u, t.GetDefaultIndex());
    EXPECT_FALSE(t.GetResetOnInit());
}

// Test add request
TEST(EventNodeTest, AddRequestTest) {
    std::string node_val = "uninitialize";
    auto update_callback = [&node_val](const std::string &, const std::string &,
                                       const std::string &val) { node_val = val; };
    EventNode t("EventName", "<Event>:Node", {{"value0"}, {"value1"}, {""}}, 2, true,
                update_callback);
    auto start = std::chrono::steady_clock::now();
    EXPECT_TRUE(t.AddRequest(1, "INTERACTION", start + 500ms));
    std::chrono::milliseconds expire_time = t.Update(true);
    // Add request @ value1
    EXPECT_EQ(node_val, "value1");
    EXPECT_NEAR(std::chrono::milliseconds(500).count(), expire_time.count(), kTIMING_TOLERANCE_MS);
    // Add request @ value0 higher prio than value1
    EXPECT_TRUE(t.AddRequest(0, "LAUNCH", start + 200ms));
    expire_time = t.Update(true);
    EXPECT_EQ(node_val, "value0");
    EXPECT_NEAR(std::chrono::milliseconds(200).count(), expire_time.count(), kTIMING_TOLERANCE_MS);
    // Let high prio request timeout, now only request @ value1 active
    std::this_thread::sleep_for(expire_time + kSLEEP_TOLERANCE_MS);
    expire_time = t.Update(true);
    EXPECT_EQ(node_val, "value1");
    EXPECT_NEAR(std::chrono::milliseconds(300).count(), expire_time.count(), kTIMING_TOLERANCE_MS);
    // Let all requests timeout, now default value2
    std::this_thread::sleep_for(expire_time + kSLEEP_TOLERANCE_MS);
    expire_time = t.Update(true);
    EXPECT_EQ(node_val, "");
    EXPECT_EQ(std::chrono::milliseconds::max(), expire_time);
}

// Test remove request
TEST(EventNodeTest, RemoveRequestTest) {
    std::string node_val = "uninitialize";
    auto update_callback = [&node_val](const std::string &, const std::string &,
                                       const std::string &val) { node_val = val; };
    EventNode t("EventName", "<Event>:Node", {{"value0"}, {"value1"}, {"value2"}}, 2, true,
                update_callback);
    auto start = std::chrono::steady_clock::now();
    EXPECT_TRUE(t.AddRequest(1, "INTERACTION", start + 500ms));
    std::chrono::milliseconds expire_time = t.Update(true);
    // Add request @ value1
    EXPECT_EQ(node_val, "value1");
    EXPECT_NEAR(std::chrono::milliseconds(500).count(), expire_time.count(), kTIMING_TOLERANCE_MS);
    // Add request @ value0 higher prio than value1
    EXPECT_TRUE(t.AddRequest(0, "LAUNCH", start + 200ms));
    expire_time = t.Update(true);
    EXPECT_EQ(node_val, "value0");
    EXPECT_NEAR(std::chrono::milliseconds(200).count(), expire_time.count(), kTIMING_TOLERANCE_MS);
    // Remove high prio request, now only request @ value1 active
    t.RemoveRequest("LAUNCH");
    expire_time = t.Update(true);
    EXPECT_EQ(node_val, "value1");
    EXPECT_NEAR(std::chrono::milliseconds(500).count(), expire_time.count(), kTIMING_TOLERANCE_MS);
    // Remove request, now default value2
    t.RemoveRequest("INTERACTION");
    expire_time = t.Update(true);
    EXPECT_EQ(node_val, "value2");
    EXPECT_EQ(std::chrono::milliseconds::max(), expire_time);
}

// Test add request
TEST(EventNodeTest, AddRequestTestOverride) {
    std::string node_val = "uninitialize";
    auto update_callback = [&node_val](const std::string &, const std::string &,
                                       const std::string &val) { node_val = val; };
    EventNode t("EventName", "<Event>:Node", {{"value0"}, {"value1"}, {"value2"}}, 2, true,
                update_callback);
    auto start = std::chrono::steady_clock::now();
    EXPECT_TRUE(t.AddRequest(1, "INTERACTION", start + 500ms));
    std::chrono::milliseconds expire_time = t.Update(true);
    // Add request @ value1
    EXPECT_EQ(node_val, "value1");
    EXPECT_NEAR(std::chrono::milliseconds(500).count(), expire_time.count(), kTIMING_TOLERANCE_MS);
    // Add request @ value0 higher prio than value1
    EXPECT_TRUE(t.AddRequest(0, "LAUNCH", start + 200ms));
    expire_time = t.Update(true);
    EXPECT_EQ(node_val, "value0");
    EXPECT_NEAR(std::chrono::milliseconds(200).count(), expire_time.count(), kTIMING_TOLERANCE_MS);
    // Add request @ value0 shorter
    EXPECT_TRUE(t.AddRequest(0, "LAUNCH", start + 100ms));
    expire_time = t.Update(true);
    EXPECT_EQ(node_val, "value0");
    EXPECT_NEAR(std::chrono::milliseconds(200).count(), expire_time.count(), kTIMING_TOLERANCE_MS);
    // Add request @ value0 longer
    EXPECT_TRUE(t.AddRequest(0, "LAUNCH", start + 300ms));
    expire_time = t.Update(true);
    EXPECT_EQ(node_val, "value0");
    EXPECT_NEAR(std::chrono::milliseconds(300).count(), expire_time.count(), kTIMING_TOLERANCE_MS);
    // Remove high prio request, now only request @ value1 active
    t.RemoveRequest("LAUNCH");
    expire_time = t.Update(true);
    EXPECT_EQ(node_val, "value1");
    EXPECT_NEAR(std::chrono::milliseconds(500).count(), expire_time.count(), kTIMING_TOLERANCE_MS);
    // Remove request, now default value2
    t.RemoveRequest("INTERACTION");
    expire_time = t.Update(true);
    EXPECT_EQ(node_val, "value2");
    EXPECT_EQ(std::chrono::milliseconds::max(), expire_time);
}

}  // namespace perfmgr
}  // namespace android
