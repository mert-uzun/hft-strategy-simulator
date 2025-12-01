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
      // FIXED: 'trades' is private, use the getter.
      // return_value_policy::reference is important so we don't copy the whole
      // list
      .def("get_trades", &TradeLog::get_trades,
           py::return_value_policy::reference)
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
      .def(py::init<double, long long, long long, long long,
                    Metrics::MarkingMethod>())
      .def_readwrite("tick_size", &Metrics::Config::tick_size)
      .def_readwrite("maker_rebate_per_share_ticks",
                     &Metrics::Config::maker_rebate_per_share_ticks)
      .def_readwrite("taker_fee_per_share_ticks",
                     &Metrics::Config::taker_fee_per_share_ticks)
      .def_readwrite("return_bucket_interval_us",
                     &Metrics::Config::return_bucket_interval_us)
      .def_readwrite("marking_method", &Metrics::Config::marking_method);

  // Bind Inner Struct: OrderCacheData
  py::class_<Metrics::OrderCacheData>(metrics, "OrderCacheData")
      .def(py::init<Metrics::Side, long long, long long, int, int, int>())
      .def_readwrite("side", &Metrics::OrderCacheData::side)
      .def_readwrite("arrival_mark_price_ticks",
                     &Metrics::OrderCacheData::arrival_mark_price_ticks)
      .def_readwrite("arrival_timestamp_us",
                     &Metrics::OrderCacheData::arrival_timestamp_us)
      .def_readwrite("intended_quantity",
                     &Metrics::OrderCacheData::intended_quantity)
      .def_readwrite("remaining_qty", &Metrics::OrderCacheData::remaining_qty)
      .def_readwrite("is_ioc", &Metrics::OrderCacheData::is_ioc);

  // Now bind the Metrics class itself
  metrics.def(py::init<>())
      .def_readonly_static("TRADING_DAYS_PER_YEAR",
                           &Metrics::TRADING_DAYS_PER_YEAR) // FIXED: Static
      .def_readonly_static("HOURS_PER_DAY",
                           &Metrics::HOURS_PER_DAY) // FIXED: Static
      .def_readwrite("config", &Metrics::config)
      .def_readwrite("fees_ticks", &Metrics::fees_ticks)
      .def_readwrite("position", &Metrics::position)
      .def_readwrite("average_entry_price_ticks",
                     &Metrics::average_entry_price_ticks)
      .def_readwrite("realized_pnl_ticks", &Metrics::realized_pnl_ticks)
      .def_readwrite("unrealized_pnl_ticks", &Metrics::unrealized_pnl_ticks)
      .def_readwrite("total_pnl_ticks", &Metrics::total_pnl_ticks)
      .def_readwrite("timestamp_series", &Metrics::timestamp_series)
      .def_readwrite("total_pnl_ticks_series", &Metrics::total_pnl_ticks_series)
      .def_readwrite("realized_pnl_ticks_series",
                     &Metrics::realized_pnl_ticks_series)
      .def_readwrite("unrealized_pnl_ticks_series",
                     &Metrics::unrealized_pnl_ticks_series)
      .def_readwrite("spread_ticks_series", &Metrics::spread_ticks_series)
      .def_readwrite("market_price_ticks_series",
                     &Metrics::market_price_ticks_series)
      .def_readwrite("gross_traded_qty", &Metrics::gross_traded_qty)
      .def_readwrite("resting_attempted_qty", &Metrics::resting_attempted_qty)
      .def_readwrite("resting_filled_qty", &Metrics::resting_filled_qty)
      .def_readwrite("resting_cancelled_qty", &Metrics::resting_cancelled_qty)
      .def_readwrite("total_slippage_ticks", &Metrics::total_slippage_ticks)
      .def_readwrite("equity_value_peak_ticks",
                     &Metrics::equity_value_peak_ticks)
      .def_readwrite("max_dropdown_ticks", &Metrics::max_dropdown_ticks)
      .def_readwrite("returns_series", &Metrics::returns_series)
      .def_readwrite("last_return_bucket_start_us",
                     &Metrics::last_return_bucket_start_us)
      .def_readwrite("last_return_bucket_total_pnl_ticks",
                     &Metrics::last_return_bucket_total_pnl_ticks)
      .def_readwrite("current_best_bid_price_ticks",
                     &Metrics::current_best_bid_price_ticks)
      .def_readwrite("current_best_ask_price_ticks",
                     &Metrics::current_best_ask_price_ticks)
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
}