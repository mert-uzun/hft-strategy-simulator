#pragma once

#include <unordered_map>
#include <vector>

class Metrics {
    public:
        static const int TRADING_DAYS_PER_YEAR;
        static const double HOURS_PER_DAY;

        enum class MarkingMethod { 
            MID, // Default is MID
            LAST
        };

        enum class Side {
            BUYS,
            SELLS
        };

        struct Config {
            double tick_size;
            long long maker_rebate_per_share_ticks;
            long long taker_fee_per_share_ticks;
            long long return_bucket_interval_us;
            MarkingMethod marking_method;

            Config(double tick_size, long long maker_rebate_per_share_ticks, long long taker_fee_per_share_ticks, long long return_bucket_interval_us, MarkingMethod marking_method)
                    : tick_size(tick_size), maker_rebate_per_share_ticks(maker_rebate_per_share_ticks), taker_fee_per_share_ticks(taker_fee_per_share_ticks), return_bucket_interval_us(return_bucket_interval_us), marking_method(marking_method) {}
        };

        Config config;

        long long fees_ticks;

        int position;
        long long average_entry_price_ticks;
        long long realized_pnl_ticks;
        long long unrealized_pnl_ticks;
        long long total_pnl_ticks;
        std::vector<long long> timestamp_series;
        std::vector<long long> total_pnl_ticks_series;
        std::vector<long long> realized_pnl_ticks_series;
        std::vector<long long> unrealized_pnl_ticks_series;
        std::vector<long long> spread_ticks_series;
        std::vector<long long> market_price_ticks_series;

        long gross_traded_qty;
        long resting_attempted_qty;
        long resting_filled_qty;
        long resting_cancelled_qty;
        long long total_slippage_ticks;

        long long equity_value_peak_ticks;
        long long max_dropdown_ticks;

        std::vector<double> returns_series;
        long long last_return_bucket_start_us;
        long long last_return_bucket_total_pnl_ticks;

        long long current_best_bid_price_ticks;
        long long current_best_ask_price_ticks;
        long long last_trade_price_ticks;
        long long last_mark_price_ticks; // This is the price where you last calculated PnL

        struct OrderCacheData {
            Side side;
            long long arrival_mark_price_ticks;
            long long arrival_timestamp_us;
            int intended_quantity;
            int remaining_qty;
            int is_ioc;

            OrderCacheData(Side side, long long arrival_mark_price_ticks, long long arrival_timestamp_us, int intended_quantity, int remaining_qty, int is_ioc)
                            : side(side), arrival_mark_price_ticks(arrival_mark_price_ticks), arrival_timestamp_us(arrival_timestamp_us), intended_quantity(intended_quantity), remaining_qty(remaining_qty), is_ioc(is_ioc) {}
        };

        std::unordered_map<long long, OrderCacheData> order_cache;

        // RESULTS
        double volatility;
        double sharpe_ratio;
        double gross_profit;
        double gross_loss; 
        double win_rate;

        Metrics();

        void set_config(double tick_size, long long maker_rebate_per_share_ticks, long long taker_fee_per_share_ticks, MarkingMethod marking_method, long long return_bucket_interval_us);
        void reset();
        void finalize(long long timestamp);
        void on_order_placed(long long order_id, Side side, long long arrival_price_ticks, long long arrival_timestamp_us, int intended_quantity, bool is_instant);
        void on_order_cancelled(long long order_id, long long delete_timestamp_us);
        void on_fill(long long order_id_1, long long fill_price_ticks, long long fill_timestamp_us, int filled_quantity, bool was_instant);
        void on_market_price_update(long long timestamp_us, long long best_bid, long long best_ask);
        void update_last_mark_price();
        void take_screenshot(long long timestamp, bool is_final);

        int get_position();
        long long get_avg_entry_price_ticks();
        long long get_realized_pnl_ticks();
        long long get_unrealized_pnl_ticks(); 
        long long get_total_pnl_ticks();
        long long get_gross_traded_qty();
        double get_fill_ratio();
        long long get_max_drawdown_ticks();

        double get_volatility();
        double get_sharpe_ratio();
        double get_gross_profit();
        double get_gross_loss();
        double get_profit_factor();
        double get_win_rate();
};