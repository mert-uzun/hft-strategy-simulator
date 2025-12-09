#pragma once

#include <unordered_map>
#include <map>
#include <list>
#include "Order.h"
#include "TradeLog.h"
#include "Metrics.h"

class OrderBook {
    private:
        std::map<long long, std::list<Order>> buys;
        std::map<long long, std::list<Order>> sells;
        std::unordered_map<long long, std::tuple<long long, std::list<Order>::iterator>> order_lookup;
        TradeLog trade_log;
        Metrics& metrics; // related metrics object from strategy
    public:
        OrderBook(Metrics& metrics);
        long long add_limit_order(bool isBuy, long long priceTick, int quantity, long long timestamp);
        std::map<long long, std::list<Order>>::reverse_iterator get_best_bid();
        std::map<long long, std::list<Order>>::iterator get_best_ask();
        long long add_IOC_order(bool isBuy, int quantity, long long timestamp);
        int cancel_order(long long orderId);
        void modify_order(long long order_id, int new_quantity, long long timestamp);
        void snapshot();

        std::map<long long, std::list<Order>>& get_buys() { return buys; }
        std::map<long long, std::list<Order>>& get_sells() { return sells; }
        std::unordered_map<long long, std::tuple<long long, std::list<Order>::iterator>>& get_order_lookup() {
            return order_lookup;
        }
        TradeLog& get_trade_log() { return trade_log; }
};