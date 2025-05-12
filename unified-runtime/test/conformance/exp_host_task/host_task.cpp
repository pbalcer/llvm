// Copyright (C) 2024 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "ur_api.h"
#include <gtest/gtest.h>
#include <uur/fixtures.h>
#include <uur/known_failure.h>
#include <vector>

using T = uint32_t;

struct urEnqueueHostTaskTest : uur::urKernelExecutionTest {
  void SetUp() override {
    program_name = "fill";
    UUR_RETURN_ON_FATAL_FAILURE(urKernelExecutionTest::SetUp());
  }

  uint32_t val = 42;
  size_t global_size = 32;
  size_t global_offset = 0;
  size_t n_dimensions = 1;
};
UUR_INSTANTIATE_DEVICE_TEST_SUITE(urEnqueueHostTaskTest);

void HostTask(void *data) {
    int *num = (int*)data;
    *num += 1;
}

TEST_P(urEnqueueHostTaskTest, Success) {
  ur_mem_handle_t buffer = nullptr;
  AddBuffer1DArg(sizeof(val) * global_size, &buffer);
  AddPodArg(val);
  ur_event_handle_t KernelEvent;
  ASSERT_SUCCESS(urEnqueueKernelLaunch(queue, kernel, n_dimensions,
                                       &global_offset, &global_size, nullptr, 0,
                                       nullptr, &KernelEvent));
  int userdata = 1;
  ur_event_handle_t HostTaskEvent;
  urEnqueueHostTaskExp(queue, HostTask, &userdata, nullptr, 1, &KernelEvent, &HostTaskEvent);

  urEventWait(1, &HostTaskEvent);
  ASSERT_EQ(userdata, 2);

  ASSERT_SUCCESS(urQueueFinish(queue));
  ValidateBuffer(buffer, sizeof(val) * global_size, val);
}
