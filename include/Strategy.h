#pragma once

#include "LatencyQueue.h"
#include "Metrics.h"
#include "OrderBook.h"
#include <functional>
#include <queue>
#include <map>
#include <utility>
#include <vector>

/**
    Ping pong strategy is implemented
*/
class Strategy {
    public:
        // State enum must be public for Python bindings and declared before use
        enum class State {
            WAITING_TO_BUY,
            WAITING_TO_SELL,
            BALANCED
        };

    private:
        Metrics& metrics;
        OrderBook& order_book;
        LatencyQueue latency_queue;

        using PongOrderData = std::pair<long long, std::pair<long long, int>>;
        std::priority_queue<PongOrderData> buy_pongs;
        std::priority_queue<PongOrderData, std::vector<PongOrderData>, std::greater<PongOrderData>> sell_pongs;

        long long best_bid_ticks;
        long long best_ask_ticks;
        long long mid_price_ticks;
        long long current_market_price_ticks;
        long long spread_ticks;
        
        int quote_size;
        long long tick_offset_from_mid;
        long long max_inventory;
        long long cancel_threshold_ticks;
        long long cooldown_between_requotes;

        long long active_buy_order_id;
        long long active_sell_order_id;
        long long last_pinged_market_price_ticks;
        long long last_quote_time_us;

        State state;

    public:
        Strategy(Metrics& metrics, OrderBook& orderbook, int quote_size, long long tick_offset, long long max_inv, long long cancel_threshold, long long cooldown_between_requotes);
        ~Strategy();
        
        void observe_the_market(long long timestamp, long long market_price);
        void cancel_mechanism(long long);
        void update_last_used_mark_price();
        void place_ping_buy(long long);
        void place_ping_ask(long long);
        bool is_bid_ping_filled(long long);
        long long pong_on_bid_filled(long long);
        bool is_ask_ping_filled(long long);
        long long pong_on_ask_filled(long long);
        void check_and_fill_pongs(long long market_price, long long timestamp_us);

        void on_market_update(long long timestamp, long long market_price);
        void on_fill(const Trade& trade);

        void execute_latency_queue(long long current_timestamp_us) {
            latency_queue.process_until(current_timestamp_us);
        }

        // Getters
        long long get_best_bid_ticks() const { return order_book.get_buys().empty() ? 0 : order_book.get_buys().rbegin()->first; }
        long long get_best_ask_ticks() const { return order_book.get_sells().empty() ? 0 : order_book.get_sells().begin()->first; }
        long long get_mid_price_ticks() const { return (best_bid_ticks + best_ask_ticks) / 2; }
        long long get_current_market_price_ticks() const { return current_market_price_ticks; }
        long long get_current_inventory() const { return metrics.position; }
        long long get_spread_ticks() const { return get_best_ask_ticks() - get_best_bid_ticks(); }
        
        int get_quote_size() const { return quote_size; }
        long long get_tick_offset_from_mid() const { return tick_offset_from_mid; }
        long long get_max_inventory() const { return max_inventory; }
        long long get_cancel_threshold_ticks() const { return cancel_threshold_ticks; }
        long long get_cooldown_between_requotes() const { return cooldown_between_requotes; }
        
        long long get_active_buy_order_id() const { return active_buy_order_id; }
        long long get_active_sell_order_id() const { return active_sell_order_id; }
        long long get_last_pinged_mid_price_ticks() const { return last_pinged_market_price_ticks; }
        long long get_last_quote_time_us() const { return last_quote_time_us; }
        
        State get_state() const { return state; }

        Metrics::OrderCacheData& get_active_buy_order_data();
        Metrics::OrderCacheData& get_active_sell_order_data();

        Metrics& get_metrics() { return metrics; }

        std::priority_queue<PongOrderData>& get_buy_pongs() { return buy_pongs; }
        std::priority_queue<PongOrderData, std::vector<PongOrderData>, std::greater<PongOrderData>>& get_sell_pongs() { return sell_pongs; }
        
        LatencyQueue& get_latency_queue() { return latency_queue; }

        // Setters
        void set_quote_size(int value) { 
            if (value > 0) {
                quote_size = value; 
            }
        }
        void set_tick_offset_from_mid(long long value) { tick_offset_from_mid = value; }
        void set_max_inventory(long long value) { 
            if (value > 0) {
                max_inventory = value; 
            }
        }
        void set_cancel_threshold_ticks(long long value) { cancel_threshold_ticks = value; }
        void set_cooldown_between_requotes(long long value) { 
            if (value > 0) {
                cooldown_between_requotes = value; 
            }
        }
        
        void set_active_buy_order_id(long long value) { active_buy_order_id = value; }
        void set_active_sell_order_id(long long value) { active_sell_order_id = value; }
        void set_last_pinged_mid_price_ticks(long long value) { 
            if (value > 0) {
                last_pinged_market_price_ticks = value; 
            }
        }
        void set_last_quote_time_us(long long value) { 
            if (value > last_quote_time_us) {
                last_quote_time_us = value;
            }
        }
        
        void set_state(State value) { state = value; }

        void set_latency_config(long long order_send_min, long long order_send_max, 
                                long long cancel_min, long long cancel_max, 
                                long long modify_min, long long modify_max, 
                                long long acknowledge_fill_min, long long acknowledge_fill_max,
                                long long market_update_min, long long market_update_max) {
            
            latency_queue.reset_latency_profile(order_send_min, order_send_max, 
                                                cancel_min, cancel_max, 
                                                modify_min, modify_max, 
                                                acknowledge_fill_min, acknowledge_fill_max, 
                                                market_update_min, market_update_max);
        }
};