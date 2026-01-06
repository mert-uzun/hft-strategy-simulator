#include <gtest/gtest.h>
#include <iostream>
#include <list>
#include "../include/LatencyQueue.h"

/**
    ============================================================
    TEST 1: DefaultInitialization
    ============================================================
    PURPOSE: Verify queue starts empty with correct default latency boundaries
    ============================================================
*/
TEST(LatencyQueueTest, DefaultInitialization) {
    LatencyQueue latency_queue;

    EXPECT_TRUE(latency_queue.is_empty())
        << "Queue should be empty initially.";

    EXPECT_EQ(latency_queue.get_latency_boundaries().order_send_min, 50)
        << "Order send min should be 50 initially. Result: " << latency_queue.get_latency_boundaries().order_send_min << std::endl;
    
    EXPECT_EQ(latency_queue.get_latency_boundaries().order_send_max, 200)
        << "Order send max should be 200 initially. Result: " << latency_queue.get_latency_boundaries().order_send_max << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().cancel_min, 30)
        << "Cancel min should be 30 initially. Result: " << latency_queue.get_latency_boundaries().cancel_min << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().cancel_max, 150)
        << "Cancel max should be 150 initially. Result: " << latency_queue.get_latency_boundaries().cancel_max << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().modify_min, 40)
        << "Modify min should be 40 initially. Result: " << latency_queue.get_latency_boundaries().modify_min << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().modify_max, 180)
        << "Modify max should be 180 initially. Result: " << latency_queue.get_latency_boundaries().modify_max << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().acknowledge_fill_min, 100)
        << "Acknowledge fill min should be 100 initially. Result: " << latency_queue.get_latency_boundaries().acknowledge_fill_min << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().acknowledge_fill_max, 400)
        << "Acknowledge fill max should be 400 initially. Result: " << latency_queue.get_latency_boundaries().acknowledge_fill_max << std::endl;
        
    EXPECT_EQ(latency_queue.get_latency_boundaries().market_update_min, 50)
        << "Market update min should be 50 initially. Result: " << latency_queue.get_latency_boundaries().market_update_min << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().market_update_max, 150)
        << "Market update max should be 150 initially. Result: " << latency_queue.get_latency_boundaries().market_update_max << std::endl;
}

/**
    ============================================================
    TEST 2: CustomLatencyProfileSetting
    ============================================================
    PURPOSE: Test setting custom latency boundaries works with limitations
    ============================================================
*/
TEST(LatencyQueueTest, CustomLatencyProfileSetting) {
    LatencyQueue latency_queue;

    // Set the custom values with invalid order_send_min and order_send_max.
    EXPECT_NO_THROW(latency_queue.reset_latency_profile(100, 20, 40, 150, 50, 180, 120, 450, 60, 160))
        << "No exceptions should be thrown even some of the given values are not valid.";

    // Since given min and max for order send are not logical (100 !<= 20), their values should not be updated.
    EXPECT_EQ(latency_queue.get_latency_boundaries().order_send_min, 50)
        << "Order send min should be 50 initially. Result: " << latency_queue.get_latency_boundaries().order_send_min << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().order_send_max, 200)
        << "Order send max should be 200 initially. Result: " << latency_queue.get_latency_boundaries().order_send_max << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().cancel_min, 40)
        << "Cancel min should be 30 initially. Result: " << latency_queue.get_latency_boundaries().cancel_min << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().cancel_max, 150)
        << "Cancel max should be 150 initially. Result: " << latency_queue.get_latency_boundaries().cancel_max << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().modify_min, 50)
        << "Modify min should be 40 initially. Result: " << latency_queue.get_latency_boundaries().modify_min << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().modify_max, 180)
        << "Modify max should be 180 initially. Result: " << latency_queue.get_latency_boundaries().modify_max << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().acknowledge_fill_min, 120)
        << "Acknowledge fill min should be 100 initially. Result: " << latency_queue.get_latency_boundaries().acknowledge_fill_min << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().acknowledge_fill_max, 450)
        << "Acknowledge fill max should be 400 initially. Result: " << latency_queue.get_latency_boundaries().acknowledge_fill_max << std::endl;
        
    EXPECT_EQ(latency_queue.get_latency_boundaries().market_update_min, 60)
        << "Market update min should be 50 initially. Result: " << latency_queue.get_latency_boundaries().market_update_min << std::endl;

    EXPECT_EQ(latency_queue.get_latency_boundaries().market_update_max, 160)
        << "Market update max should be 150 initially. Result: " << latency_queue.get_latency_boundaries().market_update_max << std::endl;
}

/**
    ============================================================
    TEST 3: GeneratedLatencyWithinBounds
    ============================================================
    PURPOSE: Verify computed latency is within min/max range using fixed boundaries
    ============================================================
*/
TEST(LatencyQueueTest, GeneratedLatencyWithinBounds) {
    LatencyQueue latency_queue;

    // We know from TEST 1 that the default latency boundaries are:
    // Order send min: 50, Order send max: 200
    // Cancel min: 30, Cancel max: 150
    // Modify min: 40, Modify max: 180
    // Acknowledge fill min: 100, Acknowledge fill max: 400
    // Market update min: 50, Market update max: 150

    // Record latency value generated for each action type.

    long long order_send_latency = latency_queue.compute_execution_latency(LatencyQueue::ActionType::ORDER_SEND);
    long long cancel_latency = latency_queue.compute_execution_latency(LatencyQueue::ActionType::CANCEL);
    long long modify_latency = latency_queue.compute_execution_latency(LatencyQueue::ActionType::MODIFY);
    long long acknowledge_fill_latency = latency_queue.compute_execution_latency(LatencyQueue::ActionType::ACKNOWLEDGE_FILL);
    long long market_update_latency = latency_queue.compute_execution_latency(LatencyQueue::ActionType::MARKET_UPDATE);
    
    // Verify these latencies are within the bounds.

    // ========================================
    // ORDER SEND
    // ========================================
    EXPECT_GE(order_send_latency, 50)
        << "Generated order send latency should be greater than or equal to minimum. Minimum: 50, Result: " << order_send_latency << std::endl;
    EXPECT_LE(order_send_latency, 200)
        << "Generated order send latency should be less than or equal to maximum. Maximum: 200, Result: " << order_send_latency << std::endl;

    // ========================================
    // CANCEL
    // ========================================
    EXPECT_GE(cancel_latency, 30)
        << "Generated cancel latency should be greater than or equal to minimum. Minimum: 30, Result: " << cancel_latency << std::endl;
    EXPECT_LE(cancel_latency, 150)
        << "Generated cancel latency should be less than or equal to maximum. Maximum: 150, Result: " << cancel_latency << std::endl;

    // ========================================
    // MODIFY
    // ========================================
    EXPECT_GE(modify_latency, 40)
        << "Generated modify latency should be greater than or equal to minimum. Minimum: 40, Result: " << modify_latency << std::endl;
    EXPECT_LE(modify_latency, 180)
        << "Generated modify latency should be less than or equal to maximum. Maximum: 180, Result: " << modify_latency << std::endl;
    
    // ========================================
    // ACKNOWLEDGE FILL
    // ========================================
    EXPECT_GE(acknowledge_fill_latency, 100)
        << "Generated acknowledge fill latency should be greater than or equal to minimum. Minimum: 100, Result: " << acknowledge_fill_latency << std::endl;
    EXPECT_LE(acknowledge_fill_latency, 400)
        << "Generated acknowledge fill latency should be less than or equal to maximum. Maximum: 400, Result: " << acknowledge_fill_latency << std::endl;
    
    // ========================================
    // MARKET UPDATE
    // ========================================
    EXPECT_GE(market_update_latency, 50)
        << "Generated market update latency should be greater than or equal to minimum. Minimum: 50, Result: " << market_update_latency << std::endl;
    EXPECT_LE(market_update_latency, 150)
        << "Generated market update latency should be less than or equal to maximum. Maximum: 150, Result: " << market_update_latency << std::endl;
}

/**
    ============================================================
    TEST 4: SingleEventProcessing
    ============================================================
    PURPOSE: Schedule one event, process it, verify callback executes at correct time
    ============================================================
*/
TEST(LatencyQueueTest, SingleEventProcessing) {
    LatencyQueue latency_queue;

    long long start_time_us = 1000000;
    
    latency_queue.schedule_event(start_time_us, LatencyQueue::ActionType::ORDER_SEND, [start_time_us](long long exec_time) {
        EXPECT_GE(exec_time, start_time_us + 50) // 50 is the default min latenct for order send action
            << "Order send execution time should be greater than or equal to start time plus minimum latency. Start time: " << start_time_us << ", Minimum latency: 50, Result: " << exec_time << std::endl;
        EXPECT_LE(exec_time, start_time_us + 200) // 200 is the default max latency for order send action
            << "Order send execution time should be less than or equal to start time plus maximum latency. Start time: " << start_time_us << ", Maximum latency: 200, Result: " << exec_time << std::endl;
    });

    latency_queue.process_until(start_time_us + 10000000); // include the event scheduled above
}

/**
    ============================================================
    TEST 5: MultipleEventsInOrder
    ============================================================
    PURPOSE: Schedule multiple events at different times, verify they execute in chronological order
    ============================================================
*/
TEST(LatencyQueueTest, MultipleEventsInOrder) {
    LatencyQueue latency_queue;

    /*
        Schedule 5 different tasks, each pushing a number to a list.
        Then we will check if these numbers are in correct order, then we will make sure scheduled events execute in a chronological order.    
    */

    // We know from TEST 1 that the default latency boundaries are:
    // Order send min: 50, Order send max: 200
    // Cancel min: 30, Cancel max: 150
    // Modify min: 40, Modify max: 180
    // Acknowledge fill min: 100, Acknowledge fill max: 400
    // Market update min: 50, Market update max: 150

    std::list<int> number_list;

    // ========================================
    // EVENT 1
    // ========================================
    latency_queue.schedule_event(1000000, LatencyQueue::ActionType::ORDER_SEND, [&number_list](long long exec_time) {
        number_list.push_back(1);
    });

    // ========================================
    // EVENT 2 - 1000000 + 200 is because maximum latency for the previous order send action is 200.
    // ========================================
    latency_queue.schedule_event(1000000 + 200, LatencyQueue::ActionType::CANCEL, [&number_list](long long exec_time) {
        number_list.push_back(2);
    });

    // ========================================
    // EVENT 3 - 1000000 + 200 + 150 is because maximum latency for the previous cancel action is 150.
    // ========================================
    latency_queue.schedule_event(1000000 + 200 + 150, LatencyQueue::ActionType::MODIFY, [&number_list](long long exec_time) {
        number_list.push_back(3);
    });

    // ========================================
    // EVENT 4 - 1000000 + 200 + 150 + 180 is because maximum latency for the previous modify action is 180.
    // ========================================
    latency_queue.schedule_event(1000000 + 200 + 150 + 180, LatencyQueue::ActionType::ACKNOWLEDGE_FILL, [&number_list](long long exec_time) {
        number_list.push_back(4);
    });
    
    // ========================================
    // EVENT 5 - 1000000 + 200 + 150 + 180 + 400 is because maximum latency for the previous acknowledge fill action is 400.
    // ========================================
    latency_queue.schedule_event(1000000 + 200 + 150 + 180 + 400, LatencyQueue::ActionType::MARKET_UPDATE, [&number_list](long long exec_time) {
        number_list.push_back(5);
    });
    
    // ========================================
    // PROCESS UNTIL 1000000 + 200 + 150 + 180 + 400 + 150 because maximum latency for the previous market update action is 150.
    // ========================================
    latency_queue.process_until(1000000 + 200 + 150 + 180 + 400 + 150);

    // Now we expect the number list to be [1, 2, 3, 4, 5] if the process_until fuction executes tasks in true chronological order.
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(number_list.front(), i + 1)
            << i + 1 << "th number in the list should be " << i + 1 << ". Result: " << number_list.front() << ". This means scheduled events are not executed in corect chronological order.";
        number_list.pop_front();
    }
}

/**
    ============================================================
    TEST 6: ProcessBeforeEventTime
    ============================================================
    PURPOSE: Verify event doesn't execute if you process before its scheduled time
    ============================================================
*/
TEST(LatencyQueueTest, ProcessBeforeEventTime) {
    LatencyQueue latency_queue;

    // Schedule a task to throw an exception, we will check if that exception is thrown before that task's execution time.
    latency_queue.schedule_event(1000000, LatencyQueue::ActionType::ORDER_SEND, [](long long exec_time) {
        throw std::runtime_error("Test exception");
    });

    EXPECT_NO_THROW(latency_queue.process_until(1000000 + 49)) // 50 is the minimum default latency for an order send action
        << "Scheduled event executes before its execution time via process.until.";
}

/**
    ============================================================
    TEST 7: IsEmptyWorks
    ============================================================
    PURPOSE: Verify is_empty() returns correct state throughout lifecycle
    ============================================================
*/
TEST(LatencyQueueTest, IsEmptyWorks) {
    LatencyQueue latency_queue;

    // We know from TEST 1 that the default latency boundaries are:
    // Order send min: 50, Order send max: 200
    // Cancel min: 30, Cancel max: 150
    // Modify min: 40, Modify max: 180
    // Acknowledge fill min: 100, Acknowledge fill max: 400
    // Market update min: 50, Market update max: 150

    // ========================================
    // CHECK 1: Is empty at the start
    // ========================================
    EXPECT_TRUE(latency_queue.is_empty())
        << "Latency queue should be empty at the start.";

    // ========================================
    // CHECK 2: Is not empty after scheduling of multiple events and not executing them. This is checked in between each scheduling action.
    // ========================================
    latency_queue.schedule_event(1000000, LatencyQueue::ActionType::ORDER_SEND, [](long long exec_time) {
        // Something
    });
    EXPECT_FALSE(latency_queue.is_empty())
        << "Latency queue should not be empty after scheduling events and not executing them (1st).";

    latency_queue.schedule_event(1000000 + 200, LatencyQueue::ActionType::CANCEL, [](long long exec_time) {
        // Something
    });
    EXPECT_FALSE(latency_queue.is_empty())
        << "Latency queue should not be empty after scheduling events and not executing them (2nd).";

    latency_queue.schedule_event(1000000 + 200 + 150, LatencyQueue::ActionType::MODIFY, [](long long exec_time) {
        // Something
    });
    EXPECT_FALSE(latency_queue.is_empty())
        << "Latency queue should not be empty after scheduling events and not executing them (3rd).";

    latency_queue.schedule_event(1000000 + 200 + 150 + 180, LatencyQueue::ActionType::ACKNOWLEDGE_FILL, [](long long exec_time) {
        // Something
    });
    EXPECT_FALSE(latency_queue.is_empty())
        << "Latency queue should not be empty after scheduling events and not executing them (4th).";

    latency_queue.schedule_event(1000000 + 200 + 150 + 180 + 400, LatencyQueue::ActionType::MARKET_UPDATE, [](long long exec_time) {
        // Something
    });
    EXPECT_FALSE(latency_queue.is_empty())
        << "Latency queue should not be empty after scheduling events and not executing them (5th).";
    
    // Check if the event queue has exactly 5 events after scheduling
    EXPECT_EQ(latency_queue.get_event_queue().size(), 5)
        << "Event queue should have 5 events after scheduling all 5 events.";

    // ========================================
    // CHECK 3: Process all 5 events check if queue is empty
    // ========================================
    latency_queue.process_until(1000000 + 200 + 150 + 180 + 400 + 150);
    EXPECT_TRUE(latency_queue.is_empty())
        << "Latency queue should be empty after processing all 5 events.";
    EXPECT_EQ(latency_queue.get_event_queue().size(), 0)
        << "Event queue should have 0 events after processing all 5 events.";
}