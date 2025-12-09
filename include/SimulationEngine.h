#pragma once

#include "MarketEngine.h"

class SimulationEngine {
    private:
        MarketEngine market_engine;

        long long starting_timestamp_us;
        long long current_timestamp_us;
        long long ending_timestamp_us;
        long long step_us;

    public:
        SimulationEngine(long long starting_timestamp_us, long long ending_timestamp_us, long long step_us, int strategy_quote_size = 1, long long strategy_tick_offset = 1, long long strategy_max_inv = 10, long long strategy_cancel_threshold = 1, long long strategy_cooldown_between_requotes = 1, long long starting_mid_price = 10000, long long start_spread = 2, double start_vol= 1.0, double start_fill_prob = 0.3);
        void run();

        void finalize(long long final_timestamp_us);

        long long get_starting_timestamp_us() { return starting_timestamp_us; }
        long long get_current_timestamp_us() { return current_timestamp_us; }
        long long get_ending_timestamp_us() { return ending_timestamp_us; }
        long long get_step_us() {return step_us; }
        MarketEngine& get_market_engine() { return market_engine; }
};