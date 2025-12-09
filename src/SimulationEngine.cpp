#include "../include/SimulationEngine.h"
#include "Strategy.h"
#include <algorithm>
#include <iostream>

SimulationEngine::SimulationEngine(long long starting_timestamp_us, long long ending_timestamp_us, long long step_us, int strategy_quote_size, long long strategy_tick_offset, long long strategy_max_inv, long long strategy_cancel_threshold, long long strategy_cooldown_between_requotes, long long starting_mid_price, long long start_spread, double start_vol, double min_volatility, double start_fill_prob)
                                    : starting_timestamp_us(starting_timestamp_us), current_timestamp_us(starting_timestamp_us), ending_timestamp_us(ending_timestamp_us), step_us(step_us), market_engine(strategy_quote_size, strategy_tick_offset, strategy_max_inv, strategy_cancel_threshold, strategy_cooldown_between_requotes, starting_mid_price, start_spread, start_vol, min_volatility, start_fill_prob) {}

void SimulationEngine::run() {
    if (current_timestamp_us <= 0 || ending_timestamp_us <= 0) {
        finalize(std::max<long long>(0, ending_timestamp_us));
        return;
    }

    long long total_time = ending_timestamp_us - starting_timestamp_us;
    const int log_percentage_step = 2;
    double last_logged_percentage = 0;

    std::cout << std::endl << std::endl;
    
    while (current_timestamp_us < ending_timestamp_us) {
        market_engine.update(current_timestamp_us);

        long long completed = current_timestamp_us - starting_timestamp_us;
        double percentage_completion = ((double)completed / total_time) * 100;        

        if (percentage_completion > last_logged_percentage && (int)percentage_completion % log_percentage_step == 0) {
            std::cout << "\033[A\033[A"; // Move up two lines

            std::cout << "\r" << "Start: " << starting_timestamp_us << ", Current: " << current_timestamp_us << ", End: " << ending_timestamp_us << " (in microseconds), " << percentage_completion << "% completed." << "\033[K" << std::endl; 

            std::string bar = "[";
            for (int i = log_percentage_step; i <= 100; i += log_percentage_step) {
                if (percentage_completion >= i) {
                    bar += "=";

                    last_logged_percentage = (i > last_logged_percentage) ? i : last_logged_percentage;
                }
                else {
                    bar += " ";
                }
            }
            bar += "]";

            std::cout << "\r" << bar << "\033[K" << std::endl;
        }

        current_timestamp_us += step_us;
    }

    finalize(ending_timestamp_us);
}

void SimulationEngine::finalize(long long final_timestamp_us) {
    market_engine.get_strategy().get_metrics().finalize(final_timestamp_us);
}