#include "../include/MarketEngine.h"
#include <algorithm>
#include <cmath>
#include <random>

long long MarketEngine::env_order_id = 1000000;
const double MarketEngine::tick_size = 0.001;
    
MarketEngine::MarketEngine(int strategy_quote_size, long long strategy_tick_offset, long long strategy_max_inv, long long strategy_cancel_threshold, long long strategy_cooldown_between_requotes, long long starting_mid_price, long long start_spread, double start_vol, double min_volatility, double start_fill_prob) : metrics(), orderbook(metrics), 
                            strategy(metrics, orderbook, strategy_quote_size, strategy_tick_offset, strategy_max_inv, strategy_cancel_threshold, strategy_cooldown_between_requotes), rng(), rand_engine(rng()), market_price_ticks(starting_mid_price), spread(start_spread), volatility(start_vol), min_volatility(min_volatility), fill_probability(start_fill_prob) {
    // Set default metrics config - return_bucket_interval of 1ms (1000 us) is a reasonable default for HFT
    metrics.set_config(tick_size, 2, 3, Metrics::MarkingMethod::MID, 1000);
}


void MarketEngine::update(long long timestamp_us) {
    simulate_background_dynamics();

    notify_metrics_of_market_state(timestamp_us);

    check_and_trigger_fills(timestamp_us);

    strategy.on_market_update(timestamp_us, market_price_ticks);

    execute_events_until(timestamp_us);
}

void MarketEngine::simulate_background_dynamics() {
    std::uniform_int_distribution<int> move(-1, 1);
    std::uniform_real_distribution<double> jump(0.0, 1.0);

    long long prev_price_ticks = market_price_ticks;

    market_price_ticks += std::llround(move(rand_engine) * volatility);

    // A rare jump in the market simulating news
    if (jump(rand_engine) < 0.001) {
        market_price_ticks += std::llround(move(rand_engine) * (5 * volatility));
    }

    long long price_change = market_price_ticks - prev_price_ticks;

    spread = std::max<long long>(1, (long long)(2 + 0.5 * volatility)); 

    volatility = std::sqrt((0.1 * price_change * price_change) + 0.9 * (volatility * volatility));
    
    // Minimum volatility floor to prevent market from freezing
    volatility = std::max(min_volatility, volatility);

    double base_fill = 0.002;
    double k = 0.2;
    double alpha = 0.1;

    fill_probability = (1 - alpha) * fill_probability + alpha * (base_fill * std::exp(-k * volatility));
}

void MarketEngine::check_and_trigger_fills(long long timestamp_us) {
    std::uniform_real_distribution<double> probability(0, 1);
    std::uniform_int_distribution<int> filled_quantity(1, strategy.get_quote_size());

    double k = 0.5; // How fast fill probability decays based on distance from market price

    // -------------- BUY SIDE -------------- //
    if (strategy.get_active_buy_order_id() != -1) {
        Metrics::OrderCacheData order_data = strategy.get_active_buy_order_data();
        long long order_price = order_data.arrival_mark_price_ticks;
        long long dist = market_price_ticks - order_price;

        double fill_prob = fill_probability * std::exp(-k * dist);
        fill_prob = std::min<double>(1, fill_prob);

        long long low = std::min(order_price, market_price_ticks + spread / 2);
        long long high = std::max(order_price, market_price_ticks + spread / 2);
        std::uniform_int_distribution<long long> fill_price(low, high);

        if (probability(rand_engine) < fill_prob) {
            Trade trade(strategy.get_active_buy_order_id(), env_order_id++, fill_price(rand_engine), std::min<int>(order_data.remaining_qty, filled_quantity(rand_engine)), timestamp_us, false);
            strategy.on_fill(trade);
        }
    }

    // -------------- SELL SIDE -------------- //
    if (strategy.get_active_sell_order_id() != -1) {
        Metrics::OrderCacheData order_data = strategy.get_active_sell_order_data();
        long long order_price = order_data.arrival_mark_price_ticks;
        long long dist = order_price - market_price_ticks;

        double fill_prob = fill_probability * std::exp(-k * dist);
        fill_prob = std::min<double>(1, fill_prob);

        long long low = std::min(market_price_ticks - spread / 2, order_price);
        long long high = std::max(market_price_ticks - spread / 2, order_price);
        std::uniform_int_distribution<long long> fill_price(low, high);

        if (probability(rand_engine) < fill_prob) {
            Trade trade(env_order_id++, strategy.get_active_sell_order_id(), fill_price(rand_engine), std::min<int>(order_data.remaining_qty, filled_quantity(rand_engine)), timestamp_us, false);
            strategy.on_fill(trade);
        }
    }    
}

void MarketEngine::execute_events_until(long long timestamp) {
    strategy.execute_latency_queue(timestamp);
}

void MarketEngine::notify_metrics_of_market_state(long long timestamp_us) {
    long long simulated_best_bid = market_price_ticks - spread / 2;
    long long simulated_best_ask = market_price_ticks + spread / 2;
    strategy.get_metrics().on_market_price_update(timestamp_us, simulated_best_bid, simulated_best_ask);
}