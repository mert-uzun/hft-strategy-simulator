#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "LatencyQueue.h"
#include "MarketEngine.h"
#include "Metrics.h"
#include "Order.h"
#include "OrderBook.h"
#include "SimulationEngine.h"
#include "Strategy.h"
#include "Trade.h"
#include "TradeLog.h"


namespace py = pybind11;

PYBIND11_MODULE(orderbook_wrapper, m) {
    m.doc() = "HFT Strategy Simulator";

    // ----------------------------------------------------------------
    // 1. Order
    // ----------------------------------------------------------------
    py::class_<Order>(m, "Order")
        .def(py::init<bool, long long, int, long long>())
        .def(py::init<bool, int, long long>())
        .def_readonly_static("tick_size", &Order::tick_size)
        .def_readwrite("id", &Order::id)
        .def_readwrite("is_buy", &Order::isBuy)
        .def_readwrite("is_active", &Order::isActive)
        .def_readwrite("price_tick", &Order::priceTick)
        .def_readwrite("quantity", &Order::quantity)
        .def_readwrite("ts_created_us", &Order::tsCreatedUs)
        .def_readwrite("ts_last_update_us", &Order::tsLastUpdateUs);

    // ----------------------------------------------------------------
    // 2. Trade
    // ----------------------------------------------------------------
    py::class_<Trade>(m, "Trade")
        .def(py::init<>())
        .def(py::init<long long, long long, long long, int, long long, bool>())
        .def_readwrite("trade_id", &Trade::tradeId)
        .def_readwrite("price_tick", &Trade::priceTick)
        .def_readwrite("quantity", &Trade::quantity)
        .def_readwrite("buy_order_id", &Trade::buyOrderId)
        .def_readwrite("sell_order_id", &Trade::sellOrderId)
        .def_readwrite("timestamp_us", &Trade::timestampUs)
        .def_readwrite("was_instant", &Trade::was_instant);

    // ----------------------------------------------------------------
    // 3. TradeLog
    // ----------------------------------------------------------------
    py::class_<TradeLog>(m, "TradeLog")
        .def(py::init<>())
        // return_value_policy::reference is important so we don't copy the whole
        // list
        .def("get_trades", &TradeLog::get_trades,py::return_value_policy::reference)
        .def("add_trade", &TradeLog::add_trade)
        .def("show_trades", &TradeLog::show_trades);

    // ----------------------------------------------------------------
    // 4. Metrics (and inner types)
    // ----------------------------------------------------------------
    // We must declare 'Metrics' first so we can bind inner types to it
    py::class_<Metrics> metrics(m, "Metrics");

    // Bind Inner Enum: MarkingMethod
    py::enum_<Metrics::MarkingMethod>(metrics, "MarkingMethod")
        .value("MID", Metrics::MarkingMethod::MID)
        .value("LAST", Metrics::MarkingMethod::LAST)
        .export_values();

    // Bind Inner Enum: Side
    py::enum_<Metrics::Side>(metrics, "Side")
        .value("BUYS", Metrics::Side::BUYS)
        .value("SELLS", Metrics::Side::SELLS)
        .export_values();

    // Bind Inner Struct: Config
    py::class_<Metrics::Config>(metrics, "Config")
        .def(py::init<double, long long, long long, long long, Metrics::MarkingMethod>())
        .def_readwrite("tick_size", &Metrics::Config::tick_size)
        .def_readwrite("maker_rebate_per_share_ticks", &Metrics::Config::maker_rebate_per_share_ticks)
        .def_readwrite("taker_fee_per_share_ticks", &Metrics::Config::taker_fee_per_share_ticks)
        .def_readwrite("return_bucket_interval_us", &Metrics::Config::return_bucket_interval_us)
        .def_readwrite("marking_method", &Metrics::Config::marking_method);

    // Bind Inner Struct: OrderCacheData
    py::class_<Metrics::OrderCacheData>(metrics, "OrderCacheData")
        .def(py::init<Metrics::Side, long long, long long, int, int, int>())
        .def_readwrite("side", &Metrics::OrderCacheData::side)
        .def_readwrite("arrival_mark_price_ticks", &Metrics::OrderCacheData::arrival_mark_price_ticks)
        .def_readwrite("arrival_timestamp_us", &Metrics::OrderCacheData::arrival_timestamp_us)
        .def_readwrite("intended_quantity", &Metrics::OrderCacheData::intended_quantity)
        .def_readwrite("remaining_qty", &Metrics::OrderCacheData::remaining_qty)
        .def_readwrite("is_ioc", &Metrics::OrderCacheData::is_ioc);

    // Now bind the Metrics class itself
    metrics.def(py::init<>())
        .def_readonly_static("TRADING_DAYS_PER_YEAR",     &Metrics::TRADING_DAYS_PER_YEAR) // FIXED: Static
        .def_readonly_static("HOURS_PER_DAY",     &Metrics::HOURS_PER_DAY) // FIXED: Static
        .def_readwrite("config", &Metrics::config)
        .def_readwrite("fees_ticks", &Metrics::fees_ticks)
        .def_readwrite("position", &Metrics::position)
        .def_readwrite("average_entry_price_ticks", &Metrics::average_entry_price_ticks)
        .def_readwrite("realized_pnl_ticks", &Metrics::realized_pnl_ticks)
        .def_readwrite("unrealized_pnl_ticks", &Metrics::unrealized_pnl_ticks)
        .def_readwrite("total_pnl_ticks", &Metrics::total_pnl_ticks)
        .def_readwrite("timestamp_series", &Metrics::timestamp_series)
        .def_readwrite("total_pnl_ticks_series", &Metrics::total_pnl_ticks_series)
        .def_readwrite("realized_pnl_ticks_series", &Metrics::realized_pnl_ticks_series)
        .def_readwrite("unrealized_pnl_ticks_series", &Metrics::unrealized_pnl_ticks_series)
        .def_readwrite("spread_ticks_series", &Metrics::spread_ticks_series)
        .def_readwrite("market_price_ticks_series", &Metrics::market_price_ticks_series)
        .def_readwrite("gross_traded_qty", &Metrics::gross_traded_qty)
        .def_readwrite("resting_attempted_qty", &Metrics::resting_attempted_qty)
        .def_readwrite("resting_filled_qty", &Metrics::resting_filled_qty)
        .def_readwrite("resting_cancelled_qty", &Metrics::resting_cancelled_qty)
        .def_readwrite("total_slippage_ticks", &Metrics::total_slippage_ticks)
        .def_readwrite("equity_value_peak_ticks", &Metrics::equity_value_peak_ticks)
        .def_readwrite("max_dropdown_ticks", &Metrics::max_dropdown_ticks)
        .def_readwrite("returns_series", &Metrics::returns_series)
        .def_readwrite("last_return_bucket_start_us", &Metrics::last_return_bucket_start_us)
        .def_readwrite("last_return_bucket_total_pnl_ticks", &Metrics::last_return_bucket_total_pnl_ticks)
        .def_readwrite("current_best_bid_price_ticks", &Metrics::current_best_bid_price_ticks)
        .def_readwrite("current_best_ask_price_ticks", &Metrics::current_best_ask_price_ticks)
        .def_readwrite("last_trade_price_ticks", &Metrics::last_trade_price_ticks)
        .def_readwrite("last_mark_price_ticks", &Metrics::last_mark_price_ticks)
        .def_readwrite("order_cache", &Metrics::order_cache)
        .def_readwrite("volatility", &Metrics::volatility)
        .def_readwrite("sharpe_ratio", &Metrics::sharpe_ratio)
        .def_readwrite("gross_profit", &Metrics::gross_profit)
        .def_readwrite("gross_loss", &Metrics::gross_loss)
        .def_readwrite("win_rate", &Metrics::win_rate)

        .def("set_config", &Metrics::set_config)
        .def("reset", &Metrics::reset)
        .def("finalize", &Metrics::finalize)
        .def("on_order_placed", &Metrics::on_order_placed)
        .def("on_order_cancelled", &Metrics::on_order_cancelled)
        .def("on_fill", &Metrics::on_fill)
        .def("on_market_price_update", &Metrics::on_market_price_update)
        .def("update_last_mark_price", &Metrics::update_last_mark_price)
        .def("take_screenshot", &Metrics::take_screenshot)
        .def("get_position", &Metrics::get_position)
        .def("get_avg_entry_price_ticks", &Metrics::get_avg_entry_price_ticks)
        .def("get_realized_pnl_ticks", &Metrics::get_realized_pnl_ticks)
        .def("get_unrealized_pnl_ticks", &Metrics::get_unrealized_pnl_ticks)
        .def("get_total_pnl_ticks", &Metrics::get_total_pnl_ticks)
        .def("get_gross_traded_qty", &Metrics::get_gross_traded_qty)
        .def("get_fill_ratio", &Metrics::get_fill_ratio)
        .def("get_max_drawdown_ticks", &Metrics::get_max_drawdown_ticks)
        .def("get_volatility", &Metrics::get_volatility)
        .def("get_sharpe_ratio", &Metrics::get_sharpe_ratio)
        .def("get_gross_profit", &Metrics::get_gross_profit)
        .def("get_gross_loss", &Metrics::get_gross_loss)
        .def("get_profit_factor", &Metrics::get_profit_factor)
        .def("get_win_rate", &Metrics::get_win_rate);

    // ----------------------------------------------------------------
    // 5. OrderBook
    // ----------------------------------------------------------------
    py::class_<OrderBook>(m, "OrderBook")
        // ═══════════════════════════════════════════════════════════════
        // CONSTRUCTOR
        // ═══════════════════════════════════════════════════════════════
        .def(py::init<Metrics&>(), py::arg("metrics"))

        // ═══════════════════════════════════════════════════════════════
        // GETTER METHODS (return references - Python CAN modify!)
        // ═══════════════════════════════════════════════════════════════
        // Using .def() instead of .def_property_readonly() to make it explicit
        // that these are methods returning mutable references.
        // 
        // In Python: order_book.get_buys()[price_level].append(order)
        //
        // return_value_policy::reference_internal means:
        //   - Return a reference (not a copy)
        //   - Keep the parent (OrderBook) alive while reference exists
        .def("get_buys", &OrderBook::get_buys, 
            py::return_value_policy::reference_internal)
        .def("get_sells", &OrderBook::get_sells, 
            py::return_value_policy::reference_internal)
        .def("get_order_lookup", &OrderBook::get_order_lookup, 
            py::return_value_policy::reference_internal)
        .def("get_trade_log", &OrderBook::get_trade_log, 
            py::return_value_policy::reference_internal)

        // ═══════════════════════════════════════════════════════════════
        // REGULAR METHODS
        // ═══════════════════════════════════════════════════════════════
        .def("add_limit_order", &OrderBook::add_limit_order,
            py::arg("is_buy"), py::arg("price_tick"), 
            py::arg("quantity"), py::arg("timestamp"))
        .def("add_ioc_order", &OrderBook::add_IOC_order,
            py::arg("is_buy"), py::arg("quantity"), py::arg("timestamp"))
        .def("cancel_order", &OrderBook::cancel_order, py::arg("order_id"))
        .def("modify_order", &OrderBook::modify_order,
            py::arg("order_id"), py::arg("new_quantity"), py::arg("timestamp"))
        .def("snapshot", &OrderBook::snapshot)

        // ═══════════════════════════════════════════════════════════════
        // ITERATOR METHODS - Cannot expose C++ iterators to Python!
        // ═══════════════════════════════════════════════════════════════
        // C++ iterators (std::map::iterator, reverse_iterator) have no 
        // Python equivalent. We MUST wrap them to return Python-friendly data.
        //
        // Options for get_best_bid() which returns reverse_iterator:
        //   1. Return just the price (long long) - simplest
        //   2. Return a tuple (price, orders_list) - more complete
        //   3. Return the full price level as a dict - most Pythonic
        //
        // Here we return a py::tuple of (price, list<Order>&) or None if empty
        .def("get_best_bid", [](OrderBook& self) -> py::object {
            auto& buys = self.get_buys();
            if (buys.empty()) return py::none();
            auto it = buys.rbegin();  // Best bid = highest price = last in sorted map
            return py::make_tuple(it->first, py::cast(it->second, 
                py::return_value_policy::reference_internal));
        })
        .def("get_best_ask", [](OrderBook& self) -> py::object {
            auto& sells = self.get_sells();
            if (sells.empty()) return py::none();
            auto it = sells.begin();  // Best ask = lowest price = first in sorted map
            return py::make_tuple(it->first, py::cast(it->second, 
                py::return_value_policy::reference_internal));
        });

    // ================================================================
    // 6. LatencyQueue
    // ================================================================
    // LatencyQueue simulates network/processing latency for HFT simulation.
    // It queues actions (order sends, cancels, fills) with random delays.
    //
    // IMPORTANT: schedule_event() cannot be exposed to Python because it
    // takes a C++ lambda/std::function. The latency simulation is meant 
    // to be used internally by Strategy. Python can configure latency
    // parameters and observe queue state.
    
    // First bind the ActionType enum
    py::enum_<LatencyQueue::ActionType>(m, "ActionType")
        .value("ORDER_SEND", LatencyQueue::ActionType::ORDER_SEND)
        .value("CANCEL", LatencyQueue::ActionType::CANCEL)
        .value("MODIFY", LatencyQueue::ActionType::MODIFY)
        .value("ACKNOWLEDGE_FILL", LatencyQueue::ActionType::ACKNOWLEDGE_FILL)
        .value("MARKET_UPDATE", LatencyQueue::ActionType::MARKET_UPDATE)
        .export_values();

    py::class_<LatencyQueue>(m, "LatencyQueue")
        .def(py::init<>())
        
        // Configuration - set min/max latency bounds for each action type
        .def("reset_latency_profile", &LatencyQueue::reset_latency_profile,
            py::arg("order_send_min"), py::arg("order_send_max"),
            py::arg("cancel_min"), py::arg("cancel_max"),
            py::arg("modify_min"), py::arg("modify_max"),
            py::arg("acknowledge_fill_min"), py::arg("acknowledge_fill_max"),
            py::arg("market_update_min"), py::arg("market_update_max"),
            "Configure latency bounds (in microseconds) for each action type")
        
        // Process events that are due
        .def("process_until", &LatencyQueue::process_until, 
            py::arg("timestamp_us"),
            "Execute all queued events with execution time <= timestamp_us")
        
        // Compute what latency would be generated for an action type
        .def("compute_execution_latency", &LatencyQueue::compute_execution_latency,
            py::arg("action_type"),
            "Get a random latency sample for the given action type")
        
        // State inspection
        .def("is_empty", &LatencyQueue::is_empty, 
            "Returns True if no events are pending")
        
        // Latency boundary getters (useful for debugging/display)
        .def("get_order_send_min", &LatencyQueue::get_order_send_min)
        .def("get_order_send_max", &LatencyQueue::get_order_send_max)
        .def("get_cancel_min", &LatencyQueue::get_cancel_min)
        .def("get_cancel_max", &LatencyQueue::get_cancel_max)
        .def("get_modify_min", &LatencyQueue::get_modify_min)
        .def("get_modify_max", &LatencyQueue::get_modify_max)
        .def("get_acknowledge_fill_min", &LatencyQueue::get_acknowledge_fill_min)
        .def("get_acknowledge_fill_max", &LatencyQueue::get_acknowledge_fill_max)
        .def("get_market_update_min", &LatencyQueue::get_market_update_min)
        .def("get_market_update_max", &LatencyQueue::get_market_update_max)
        
        // Helper: get number of pending events
        .def("get_pending_event_count", [](const LatencyQueue& self) {
            return self.get_event_queue().size();
        }, "Returns the number of events waiting to be executed");

    // ================================================================
    // 7. Strategy 
    // ================================================================
    // Ping-pong market making strategy. Places "ping" orders at mid +/- offset,
    // and when filled, places "pong" orders to capture the spread.
    //
    // Strategy owns a LatencyQueue and operates on Metrics/OrderBook via 
    // references passed at construction.

    // First, declare the Strategy class so we can bind its inner enum
    py::class_<Strategy> strategy_class(m, "Strategy");
    
    // Bind the Strategy::State enum
    py::enum_<Strategy::State>(strategy_class, "State")
        .value("WAITING_TO_BUY", Strategy::State::WAITING_TO_BUY)
        .value("WAITING_TO_SELL", Strategy::State::WAITING_TO_SELL)
        .value("BALANCED", Strategy::State::BALANCED)
        .export_values();
    
    // Now bind Strategy methods
    strategy_class
        .def(py::init<Metrics&, OrderBook&, int, long long, long long, long long, long long>(),
            py::arg("metrics"), py::arg("orderbook"), 
            py::arg("quote_size"), py::arg("tick_offset"), 
            py::arg("max_inv"), py::arg("cancel_threshold"), 
            py::arg("cooldown_between_requotes"),
            "Create a ping-pong strategy with given parameters")
        
        // ═══════════════════════════════════════════════════════════════
        // CORE STRATEGY METHODS (called by MarketEngine)
        // ═══════════════════════════════════════════════════════════════
        .def("on_market_update", &Strategy::on_market_update,
            py::arg("timestamp"), py::arg("market_price"),
            "Called each tick with current market price - main strategy logic")
        .def("on_fill", &Strategy::on_fill, 
            py::arg("trade"),
            "Called when a trade fills one of our orders")
        .def("execute_latency_queue", &Strategy::execute_latency_queue,
            py::arg("current_timestamp_us"),
            "Process pending latency-delayed events")
        
        // ═══════════════════════════════════════════════════════════════
        // GETTERS - Market State
        // ═══════════════════════════════════════════════════════════════
        .def("get_best_bid_ticks", &Strategy::get_best_bid_ticks)
        .def("get_best_ask_ticks", &Strategy::get_best_ask_ticks)
        .def("get_mid_price_ticks", &Strategy::get_mid_price_ticks)
        .def("get_current_market_price_ticks", &Strategy::get_current_market_price_ticks)
        .def("get_spread_ticks", &Strategy::get_spread_ticks)
        .def("get_current_inventory", &Strategy::get_current_inventory)
        
        // ═══════════════════════════════════════════════════════════════
        // GETTERS - Strategy Parameters
        // ═══════════════════════════════════════════════════════════════
        .def("get_quote_size", &Strategy::get_quote_size)
        .def("get_tick_offset_from_mid", &Strategy::get_tick_offset_from_mid)
        .def("get_max_inventory", &Strategy::get_max_inventory)
        .def("get_cancel_threshold_ticks", &Strategy::get_cancel_threshold_ticks)
        .def("get_cooldown_between_requotes", &Strategy::get_cooldown_between_requotes)
        
        // ═══════════════════════════════════════════════════════════════
        // GETTERS - Order State
        // ═══════════════════════════════════════════════════════════════
        .def("get_active_buy_order_id", &Strategy::get_active_buy_order_id)
        .def("get_active_sell_order_id", &Strategy::get_active_sell_order_id)
        .def("get_last_pinged_mid_price_ticks", &Strategy::get_last_pinged_mid_price_ticks)
        .def("get_last_quote_time_us", &Strategy::get_last_quote_time_us)
        .def("get_state", &Strategy::get_state)
        
        // ═══════════════════════════════════════════════════════════════
        // GETTERS - References to owned/referenced objects
        // ═══════════════════════════════════════════════════════════════
        .def("get_metrics", &Strategy::get_metrics, 
            py::return_value_policy::reference_internal)
        .def("get_latency_queue", &Strategy::get_latency_queue,
            py::return_value_policy::reference_internal)
        
        // Active order data (may throw if no active order)
        .def("get_active_buy_order_data", &Strategy::get_active_buy_order_data,
            py::return_value_policy::reference_internal,
            "Get data for active buy order (throws if none)")
        .def("get_active_sell_order_data", &Strategy::get_active_sell_order_data,
            py::return_value_policy::reference_internal,
            "Get data for active sell order (throws if none)")
        
        // ═══════════════════════════════════════════════════════════════
        // GETTERS - Pong Order Queues (priority_queue -> list)
        // ═══════════════════════════════════════════════════════════════
        // std::priority_queue cannot be exposed directly. We provide helpers
        // that return the pongs as a Python list of tuples.
        .def("get_buy_pongs_list", [](Strategy& self) {
            py::list result;
            // We need to copy since priority_queue doesn't allow iteration
            auto pongs_copy = self.get_buy_pongs();
            while (!pongs_copy.empty()) {
                auto& top = pongs_copy.top();
                // PongOrderData = pair<price, pair<order_id, quantity>>
                result.append(py::make_tuple(top.first, top.second.first, top.second.second));
                pongs_copy.pop();
            }
            return result;
        }, "Get pending buy pong orders as list of (price, order_id, quantity) tuples")
        
        .def("get_sell_pongs_list", [](Strategy& self) {
            py::list result;
            auto pongs_copy = self.get_sell_pongs();
            while (!pongs_copy.empty()) {
                auto& top = pongs_copy.top();
                result.append(py::make_tuple(top.first, top.second.first, top.second.second));
                pongs_copy.pop();
            }
            return result;
        }, "Get pending sell pong orders as list of (price, order_id, quantity) tuples")
        
        // ═══════════════════════════════════════════════════════════════
        // SETTERS - Strategy Parameters
        // ═══════════════════════════════════════════════════════════════
        .def("set_quote_size", &Strategy::set_quote_size, py::arg("value"))
        .def("set_tick_offset_from_mid", &Strategy::set_tick_offset_from_mid, py::arg("value"))
        .def("set_max_inventory", &Strategy::set_max_inventory, py::arg("value"))
        .def("set_cancel_threshold_ticks", &Strategy::set_cancel_threshold_ticks, py::arg("value"))
        .def("set_cooldown_between_requotes", &Strategy::set_cooldown_between_requotes, py::arg("value"))
        .def("set_active_buy_order_id", &Strategy::set_active_buy_order_id, py::arg("value"))
        .def("set_active_sell_order_id", &Strategy::set_active_sell_order_id, py::arg("value"))
        .def("set_last_pinged_mid_price_ticks", &Strategy::set_last_pinged_mid_price_ticks, py::arg("value"))
        .def("set_last_quote_time_us", &Strategy::set_last_quote_time_us, py::arg("value"))
        .def("set_state", &Strategy::set_state, py::arg("value"))
        
        // ═══════════════════════════════════════════════════════════════
        // LATENCY CONFIGURATION
        // ═══════════════════════════════════════════════════════════════
        .def("set_latency_config", &Strategy::set_latency_config,
            py::arg("order_send_min"), py::arg("order_send_max"),
            py::arg("cancel_min"), py::arg("cancel_max"),
            py::arg("modify_min"), py::arg("modify_max"),
            py::arg("acknowledge_fill_min"), py::arg("acknowledge_fill_max"),
            py::arg("market_update_min"), py::arg("market_update_max"),
            "Configure strategy's latency queue parameters (in microseconds)");

    // ================================================================
    // 8. MarketEngine
    // ================================================================
    // MarketEngine is the core simulation component. It:
    //   - Owns Metrics, OrderBook, Strategy
    //   - Simulates market price movements (random walk + jumps)
    //   - Triggers fills probabilistically based on distance from market
    //   - Calls strategy.on_market_update() each tick
    
    py::class_<MarketEngine>(m, "MarketEngine")
        .def(py::init<int, long long, long long, long long, long long, long long, long long, double, double, double>(),
            py::arg("strategy_quote_size") = 1,
            py::arg("strategy_tick_offset") = 1,
            py::arg("strategy_max_inv") = 10,
            py::arg("strategy_cancel_threshold") = 1,
            py::arg("strategy_cooldown_between_requotes") = 1,
            py::arg("starting_mid_price") = 10000,
            py::arg("start_spread") = 2,
            py::arg("start_vol") = 1.0,
            py::arg("min_volatility") = 0.5,
            py::arg("start_fill_prob") = 0.3,
            "Create market engine with strategy and market parameters")
        
        // ═══════════════════════════════════════════════════════════════
        // CORE SIMULATION METHOD
        // ═══════════════════════════════════════════════════════════════
        .def("update", &MarketEngine::update, 
            py::arg("timestamp_us"),
            "Run one simulation tick: update market, check fills, notify strategy")
        
        // ═══════════════════════════════════════════════════════════════
        // GETTERS - Component Access
        // ═══════════════════════════════════════════════════════════════
        .def("get_orderbook", &MarketEngine::get_orderbook,
            py::return_value_policy::reference_internal,
            "Get reference to the OrderBook")
        .def("get_strategy", &MarketEngine::get_strategy,
            py::return_value_policy::reference_internal,
            "Get reference to the Strategy")
        .def("get_metrics", &MarketEngine::get_metrics,
            py::return_value_policy::reference_internal,
            "Get reference to the Metrics")
        
        // ═══════════════════════════════════════════════════════════════
        // GETTERS - Market State
        // ═══════════════════════════════════════════════════════════════
        .def("get_market_price_ticks", &MarketEngine::get_market_price_ticks,
            "Current simulated market price in ticks")
        .def("get_spread", &MarketEngine::get_spread,
            "Current bid-ask spread in ticks")
        .def("get_volatility", &MarketEngine::get_volatility,
            "Current market volatility estimate")
        .def("get_fill_probability", &MarketEngine::get_fill_probability,
            "Current base fill probability");

    // ================================================================
    // 9. SimulationEngine
    // ================================================================
    // SimulationEngine is the TOP-LEVEL entry point for running simulations.
    // It owns a MarketEngine and drives the main simulation loop.
    //
    // TYPICAL PYTHON USAGE:
    //   sim = SimulationEngine(
    //       starting_timestamp_us=0,
    //       ending_timestamp_us=1_000_000,  # 1 second
    //       step_us=100,  # 100 microsecond steps
    //       strategy_quote_size=10,
    //       ...
    //   )
    //   sim.run()
    //   metrics = sim.get_market_engine().get_metrics()
    //   print(f"Sharpe: {metrics.get_sharpe_ratio()}")
    
    py::class_<SimulationEngine>(m, "SimulationEngine")
        .def(py::init<long long, long long, long long, int, long long, long long, long long, long long, long long, long long, double, double, double>(),
            py::arg("starting_timestamp_us"),
            py::arg("ending_timestamp_us"),
            py::arg("step_us"),
            py::arg("strategy_quote_size") = 1,
            py::arg("strategy_tick_offset") = 1,
            py::arg("strategy_max_inv") = 10,
            py::arg("strategy_cancel_threshold") = 1,
            py::arg("strategy_cooldown_between_requotes") = 1,
            py::arg("starting_mid_price") = 10000,
            py::arg("start_spread") = 2,
            py::arg("start_vol") = 1.0,
            py::arg("min_volatility") = 0.5,
            py::arg("start_fill_prob") = 0.3,
            "Create simulation engine with full configuration")
        
        // ═══════════════════════════════════════════════════════════════
        // MAIN SIMULATION METHODS
        // ═══════════════════════════════════════════════════════════════
        .def("run", &SimulationEngine::run,
            "Run the full simulation from start to end timestamp")
        .def("finalize", &SimulationEngine::finalize,
            py::arg("final_timestamp_us"),
            "Finalize metrics calculation (called automatically by run())")
        
        // ═══════════════════════════════════════════════════════════════
        // GETTERS - Simulation State
        // ═══════════════════════════════════════════════════════════════
        .def("get_starting_timestamp_us", &SimulationEngine::get_starting_timestamp_us)
        .def("get_current_timestamp_us", &SimulationEngine::get_current_timestamp_us)
        .def("get_ending_timestamp_us", &SimulationEngine::get_ending_timestamp_us)
        .def("get_step_us", &SimulationEngine::get_step_us)
        
        // ═══════════════════════════════════════════════════════════════
        // ACCESS TO INNER COMPONENTS
        // ═══════════════════════════════════════════════════════════════
        .def("get_market_engine", &SimulationEngine::get_market_engine,
            py::return_value_policy::reference_internal,
            "Get reference to the MarketEngine");
}