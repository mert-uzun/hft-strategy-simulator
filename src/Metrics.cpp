#include "../include/Metrics.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

const int Metrics::TRADING_DAYS_PER_YEAR = 252;
const double Metrics::HOURS_PER_DAY = 6.5;

Metrics::Metrics() : config(0, 0, 0, 0, MarkingMethod::MID), timestamp_series(), total_pnl_ticks_series(), 
                        last_return_bucket_start_us(0), last_return_bucket_total_pnl_ticks(0), realized_pnl_ticks_series(), unrealized_pnl_ticks_series(), spread_ticks_series(), market_price_ticks_series(), returns_series(), order_cache() {
    reset();
}

void Metrics::set_config(double tick_size, long long maker_rebate_per_share_ticks, long long taker_fee_per_share_ticks, MarkingMethod marking_method, long long return_bucket_interval_us) {
    config.tick_size = tick_size;
    config.maker_rebate_per_share_ticks = maker_rebate_per_share_ticks;
    config.taker_fee_per_share_ticks = taker_fee_per_share_ticks;
    config.marking_method = marking_method;
    config.return_bucket_interval_us = return_bucket_interval_us;
}

void Metrics::reset() {
    set_config(0, 0, 0, MarkingMethod::MID, 0);

    fees_ticks = 0;
    position = 0;
    average_entry_price_ticks = 0;
    realized_pnl_ticks = 0;
    unrealized_pnl_ticks = 0;
    total_pnl_ticks = 0;
    timestamp_series.clear();
    total_pnl_ticks_series.clear();
    realized_pnl_ticks_series.clear();
    unrealized_pnl_ticks_series.clear();
    spread_ticks_series.clear();
    market_price_ticks_series.clear();

    gross_traded_qty = 0;
    resting_attempted_qty = 0;
    resting_filled_qty = 0;
    resting_cancelled_qty = 0;
    total_slippage_ticks = 0;

    equity_value_peak_ticks = 0;
    max_dropdown_ticks = 0;

    returns_series.clear();
    last_return_bucket_start_us = 0;
    last_return_bucket_total_pnl_ticks = 0;

    current_best_ask_price_ticks = 0;
    current_best_bid_price_ticks = 0;
    last_trade_price_ticks = 0;
    last_mark_price_ticks = 0;

    order_cache.clear();

    volatility = 0;
    sharpe_ratio = 0;
    gross_profit = 0;
    gross_loss = 0;
    win_rate = 0;
}

void Metrics::finalize(long long timestamp) {
    take_screenshot(timestamp, true);

    // Calculate summary statistics
    double returns_mean = 0;
    double sum_sqrtd_differences = 0;
    double std_returns = 0;
    double raw_sharp_ratio = 0;
    
    int num_winning_return_buckets = 0;
    
    // SHARPE RATIO  
    if (!returns_series.empty()) {
        for (auto iter_returns = returns_series.begin(); iter_returns != returns_series.end(); ++iter_returns) {
            returns_mean += *iter_returns;

            // Also check if they are positive here, so we don't need another loop through for it
            if (*iter_returns > 0) {
                num_winning_return_buckets++;

                // GROSS PROFIT
                gross_profit += *iter_returns;
            }
            else {
                // GROSS LOSS
                gross_loss += std::abs(*iter_returns);
            }
        }

        returns_mean = returns_mean / returns_series.size();

        for (auto iter_returns = returns_series.begin(); iter_returns != returns_series.end(); ++iter_returns) {
            sum_sqrtd_differences += (*iter_returns - returns_mean) * (*iter_returns - returns_mean);
        }

        std_returns = std::sqrt(sum_sqrtd_differences / returns_series.size());
        volatility = std_returns;
    }

    if (std_returns) {
        raw_sharp_ratio = returns_mean / std_returns;

        double num_buckets_in_a_year = (TRADING_DAYS_PER_YEAR * HOURS_PER_DAY * 3600 * 1000000) / config.return_bucket_interval_us;
        double scaling_factor = std::sqrt(num_buckets_in_a_year);

        sharpe_ratio = raw_sharp_ratio * scaling_factor;
    }

    // WIN RATE
    win_rate = double(num_winning_return_buckets) / returns_series.size();
}

void Metrics::on_order_placed(long long order_id, Side side, long long arrival_price_ticks, long long arrival_timestamp_us, int intended_quantity, bool is_instant) {
    order_cache.emplace(order_id, side, arrival_price_ticks, arrival_timestamp_us, intended_quantity, intended_quantity, is_instant);

    if (!is_instant) {
        resting_attempted_qty += intended_quantity;
    }
}

void Metrics::on_order_cancelled(long long order_id, long long delete_timestamp_us) {
    auto iter = order_cache.find(order_id);
    if (iter == order_cache.end()) {
        return;
    }

    auto& order = iter->second;

    if (!order.is_ioc) {
        resting_cancelled_qty += order.remaining_qty;
    }

    order_cache.erase(order_id);
}

void Metrics::on_fill(long long order_id_1, long long fill_price_per_share_ticks, long long fill_timestamp_us, int filled_quantity, bool was_instant) {
    auto iter_order_1 = order_cache.find(order_id_1);
    if (iter_order_1 == order_cache.end()) {
        std::cout << "There is no such order with given id (" << order_id_1 << ") to fill in the cache." << std::endl;
        return;
    }
    
    gross_traded_qty += filled_quantity;
    last_trade_price_ticks = fill_price_per_share_ticks;
    
    if (!was_instant) {
        resting_filled_qty += filled_quantity;
        fees_ticks -= config.maker_rebate_per_share_ticks * filled_quantity;
    }
    else {
        fees_ticks += config.taker_fee_per_share_ticks * filled_quantity;
    }

    Side side = iter_order_1->second.side;

    long long arrival_price = iter_order_1->second.arrival_mark_price_ticks;
    long long slippage_per_share;
    side == Side::BUYS ? slippage_per_share = fill_price_per_share_ticks - arrival_price : slippage_per_share = arrival_price - fill_price_per_share_ticks;
    total_slippage_ticks += slippage_per_share * filled_quantity;
    
    if (position >= 0) {
        if (side == Side::BUYS) { // Means it is either opening or increasing a position
            average_entry_price_ticks = ((position * average_entry_price_ticks) + (fill_price_per_share_ticks * filled_quantity)) / (position + filled_quantity);
            position += filled_quantity;
        }
        else { // Means it is either reducing or closing a position
            realized_pnl_ticks += std::min(position, filled_quantity) * (fill_price_per_share_ticks - average_entry_price_ticks);
            position -= filled_quantity;

            if (position < 0) {
                average_entry_price_ticks = fill_price_per_share_ticks;
            }
        }
    }
    else {
        if (side == Side::SELLS) { // Means it is either opening or increasing a position
            average_entry_price_ticks = ((abs(position) * average_entry_price_ticks) + (fill_price_per_share_ticks * filled_quantity)) / abs(position - filled_quantity);
            position -= filled_quantity;
        }
        else { // Means it is either reducing or closing a position
            realized_pnl_ticks += std::min(abs(position), filled_quantity) * (average_entry_price_ticks - fill_price_per_share_ticks);
            position += filled_quantity;
            
            if (position > 0) {
                average_entry_price_ticks = fill_price_per_share_ticks;
            }
        }
    }

    iter_order_1->second.remaining_qty -= filled_quantity;

    if (iter_order_1->second.remaining_qty == 0) {
        order_cache.erase(order_id_1);
    }
    else if (iter_order_1->second.remaining_qty < 0) {
        throw std::runtime_error("There is an error in the mechanism of matching trades, quantities shouldn't go below 0");
    }

    take_screenshot(fill_timestamp_us, false);
}

void Metrics::on_market_price_update(long long timestamp_us, long long best_bid, long long best_ask) {
    current_best_bid_price_ticks = best_bid;
    current_best_ask_price_ticks = best_ask;

    take_screenshot(timestamp_us, false);
}

void Metrics::take_screenshot(long long timestamp, bool is_final) {
    update_last_mark_price();

    unrealized_pnl_ticks = position * (last_mark_price_ticks - average_entry_price_ticks);
    total_pnl_ticks = realized_pnl_ticks + unrealized_pnl_ticks;

    timestamp_series.push_back(timestamp);
    realized_pnl_ticks_series.push_back(realized_pnl_ticks);
    unrealized_pnl_ticks_series.push_back(unrealized_pnl_ticks);
    total_pnl_ticks_series.push_back(total_pnl_ticks);
    spread_ticks_series.push_back(current_best_ask_price_ticks - current_best_bid_price_ticks);
    market_price_ticks_series.push_back(last_mark_price_ticks);

    equity_value_peak_ticks = std::max(total_pnl_ticks, equity_value_peak_ticks);
    max_dropdown_ticks = std::min(max_dropdown_ticks, total_pnl_ticks - equity_value_peak_ticks);
    
    if (is_final || timestamp - last_return_bucket_start_us >= config.return_bucket_interval_us) {
        returns_series.push_back(total_pnl_ticks - last_return_bucket_total_pnl_ticks);

        last_return_bucket_start_us = timestamp;
        last_return_bucket_total_pnl_ticks = total_pnl_ticks;
    }
}

int Metrics::get_position() {
    return position;
}

long long Metrics::get_avg_entry_price_ticks() {
    return average_entry_price_ticks;
}

long long Metrics::get_realized_pnl_ticks() {
    return realized_pnl_ticks;
}

long long Metrics::get_unrealized_pnl_ticks() {
    return unrealized_pnl_ticks;
} 

long long Metrics::get_total_pnl_ticks() {
    return total_pnl_ticks;
}

long long Metrics::get_gross_traded_qty() {
    return gross_traded_qty;
}

double Metrics::get_fill_ratio() {
    if (resting_attempted_qty == 0) {
        return 0;
    }

    return double(resting_filled_qty) / resting_attempted_qty;
}

long long Metrics::get_max_drawdown_ticks() {
    return max_dropdown_ticks;
}

double Metrics::get_sharpe_ratio() {
    return sharpe_ratio;
}

double Metrics::get_gross_profit() {
    return gross_profit;
}

double Metrics::get_gross_loss() {
    return gross_loss;
}

double Metrics::get_win_rate() {
    return win_rate;
}

double Metrics::get_volatility() {
    return volatility;
}

double Metrics::get_profit_factor() {
    if (gross_loss == 0) {
        return 0;
    }

    return gross_profit / gross_loss;
}

void Metrics::update_last_mark_price() {
    if (config.marking_method == MarkingMethod::MID) {
        last_mark_price_ticks = (current_best_bid_price_ticks + current_best_ask_price_ticks) / 2;
    }
    else {
        last_mark_price_ticks = last_trade_price_ticks;
    }
}