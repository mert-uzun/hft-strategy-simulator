#include <algorithm>
#include <iostream>

#include "../include/Order.h"
#include "../include/OrderBook.h"

OrderBook::OrderBook(Metrics& metrics) : buys(), sells(), order_lookup(), trade_log(), metrics(metrics) {}

/**
 *   Check if given price has a match in the current market (if its a buy >= best_ask | if its a sell <= best_bid) 
 *   if yes: reduce the min price from the new order and the matched one, if new order goes zero->end, if matched goes zero->continue exhausting the iterator.
 *           if list empties without new order gettign satisfied, delete that price level, then check for new best prices, then exhaust them again,
 *           UNTIL THERE ARE NO ORDERS IN THE OPPOSITE SIDE LEFT OR THE NEW BEST PRICE LEVEL DOESN'T MATCH WITH THE NEW ORDER OR NEW ORDER GETTING EXHAUSTED


 * if no: put the object directly in the data holders    
 */
long long OrderBook::add_limit_order(bool isBuy, long long priceTick, int quantity, long long timestamp) {
    Order new_order(isBuy, priceTick, quantity, timestamp);
    long long new_order_id = new_order.id;
    auto& side = isBuy ? buys : sells;
    auto& opposite_side = isBuy ? sells : buys;

    long long opposite_best_price;
    // Has matches in the market     
    while (!opposite_side.empty()) {
        opposite_best_price= isBuy ? get_best_ask()->first : get_best_bid()->first;

        if (!(isBuy ? priceTick >= opposite_best_price : priceTick <= opposite_best_price)) {
            break;
        }

        std::list<Order>& orders_at_price = isBuy ? get_best_ask()->second : get_best_bid()->second;

        while (!orders_at_price.empty()) {
            Order& matched_order = orders_at_price.front();

            int matched_quantity = std::min(new_order.quantity, matched_order.quantity);
            new_order.quantity -= matched_quantity;
            new_order.tsLastUpdateUs = timestamp;
            matched_order.quantity -= matched_quantity;
            matched_order.tsLastUpdateUs = timestamp;

            if (isBuy) {
                trade_log.add_trade(new_order.id, matched_order.id, opposite_best_price, matched_quantity, timestamp, false);
            }
            else {
                trade_log.add_trade(matched_order.id, new_order.id, opposite_best_price, matched_quantity, timestamp, false);
            }
            
            if (matched_order.quantity == 0) {
                order_lookup.erase(matched_order.id);
                metrics.on_fill(matched_order.id, opposite_best_price, timestamp, matched_quantity, false);
                orders_at_price.pop_front();
            }
            if (new_order.quantity == 0) {
                if (orders_at_price.empty()) {
                    opposite_side.erase(opposite_best_price);
                }
                return new_order_id;
            }
        }

        opposite_side.erase(opposite_best_price);
    }

    // Has no match to trade OR exhausted the whole market, just hold in the market.
    auto& level = side[priceTick];
    auto iter = level.insert(level.end(), new_order); // insert() returns an iterator directly to the newOrder inserted

    order_lookup[new_order_id] = std::make_tuple(priceTick, iter);

    return new_order_id;
}

/**
    First check if orders exists at the best price level, then exhaust all the orders until IOC is satisfied, when its satisfied: return
        if its not satisfied & orders.empty(): remove this price level from the matching side and reset the loop

    @return  order id of the IOC order
*/
long long OrderBook::add_IOC_order(bool isBuy, int quantity, long long timestamp) {
    Order new_market_order(isBuy, quantity, timestamp);
    auto& opposite_side = isBuy ? sells : buys;
    
    while (!opposite_side.empty()) {
        long long current_best_price = isBuy ? get_best_ask()->first : get_best_bid()->first;
        std::list<Order>& orders_at_best_price = isBuy ? get_best_ask()->second : get_best_bid()->second;
        
        while (!orders_at_best_price.empty()) {
            Order& matched_order = orders_at_best_price.front();
            int matched_quantity = std::min(new_market_order.quantity, matched_order.quantity);
            new_market_order.quantity -= matched_quantity;
            matched_order.quantity -= matched_quantity;
            matched_order.tsLastUpdateUs = timestamp;

            if (isBuy) {
                trade_log.add_trade(new_market_order.id, matched_order.id, matched_order.priceTick, matched_quantity, timestamp, true);
            }
            else {
                trade_log.add_trade(matched_order.id, new_market_order.id, matched_order.priceTick, matched_quantity, timestamp, true);
            }

            if (matched_order.quantity == 0) {
                order_lookup.erase(matched_order.id);
                metrics.on_fill(matched_order.id, matched_order.priceTick, matched_quantity, matched_quantity, true);
                orders_at_best_price.pop_front();
            }
            if (new_market_order.quantity == 0) {
                if (orders_at_best_price.empty()) {
                    opposite_side.erase(current_best_price);
                }
                return new_market_order.id;
            }
        }

        opposite_side.erase(current_best_price);
    }

    return new_market_order.id;
}

int OrderBook::cancel_order(long long orderId) {
    auto iter_lookup = order_lookup.find(orderId); // .find(key) returns a temp iterator to the corresponding <key, value> std::pair
    if (iter_lookup == order_lookup.end()){
        std::cout << "This order doesn't exist.";
        return 1;
    }

    auto& iter_to_order = std::get<1>(iter_lookup->second);

    Order& order_to_delete = *iter_to_order;
    long long order_price = order_to_delete.priceTick;

    auto& side = order_to_delete.isBuy ? buys : sells;

    auto price_level = side.find(order_price);
    if (price_level == side.end()){
        std::cout << "Error occured finding the price level or order to modify.";
        return 1;
    }
    auto& price_level_list = price_level->second;

    price_level_list.erase(iter_to_order);
    if (price_level_list.empty()) {
        side.erase(order_price);
    }
    order_lookup.erase(iter_lookup);

    return 0;
}

void OrderBook::modify_order(long long order_id, int new_quantity, long long timestamp) {
    if (new_quantity <= 0) {
        cancel_order(order_id);
    }

    // get the iterator pointing to the [price, iter_to_order] tuple
    auto iter_lookup = order_lookup.find(order_id);
    if (iter_lookup == order_lookup.end()) {
        return; // Meaning it didn't exist
    }

    auto& iterator_to_order = std::get<1>(iter_lookup->second);
    Order& order_to_modify = *iterator_to_order;

    if (new_quantity >= order_to_modify.quantity) {
        auto& side = order_to_modify.isBuy ? buys : sells;

        auto price_level = side.find(order_to_modify.priceTick);
        if (price_level == side.end()) {
            std::cout << "Error occured finding the price level or order to modify.";
            return;
        }
        auto& price_level_list = price_level->second;

        bool is_order_buy = order_to_modify.isBuy;
        long long order_price = order_to_modify.priceTick;

        price_level_list.erase(iterator_to_order); // this is the iterator pointing to the order itself 
        order_lookup.erase(iter_lookup); // this is the iterator to the tuple [price, iter] stored in the unordered map

        add_limit_order(is_order_buy, order_price, new_quantity, timestamp);
    }
    else {
        order_to_modify.quantity = new_quantity;
        order_to_modify.tsLastUpdateUs = timestamp;
    }
}

/**
 * @brief Gets the best bid (highest buy price in the market along with the list of orders in that price)
 * @return returns an iterator pointing to the highest bid in the market as a <price, list<Order>> pair
 */
 std::map<long long, std::list<Order>>::reverse_iterator OrderBook::get_best_bid() {
    auto bestBuy = buys.rbegin(); 
    return bestBuy;
}

/**
 * @brief Gets the best ask (lowest sell price in the market along with the list of orders in that price)
 * @return returns an iterator pointing to the lowest ask in the market as a <price, list<Order>> pair
 */
 std::map<long long, std::list<Order>>::iterator OrderBook::get_best_ask() {
    auto bestSell = sells.begin();
    return bestSell;
}

void OrderBook::snapshot() {
    std::cout << "**************************" << std::endl;

    std::cout << "Buys:" << std::endl;
    for (auto iter_buy_map = buys.rbegin(); iter_buy_map != buys.rend(); ++iter_buy_map) {
        double price_shown = ((iter_buy_map->first) * Order::tick_size);
        std::cout << "Orders at price " << price_shown << "$ : [";
        auto& buy_list_at_price = iter_buy_map->second;
        for (auto iter_buy_list = (buy_list_at_price).begin(); iter_buy_list != buy_list_at_price.end(); ++iter_buy_list) {
            Order& order_at_iter = *iter_buy_list;
            std::cout << "id=" << order_at_iter.id << ":" << order_at_iter.quantity;

            if (&order_at_iter != &(buy_list_at_price.back())) {
                std::cout << ", ";
            }
        }

        std::cout << "]" << std::endl;
    }

    std::cout << "**************************" << std::endl;

    std::cout << "Sells:"<< std::endl;
    for (auto iter_sell_map = sells.begin(); iter_sell_map != sells.end(); ++iter_sell_map) {
        double price_shown = ((iter_sell_map->first) * Order::tick_size);
        std::cout << "Orders at price " << price_shown << "$ : [";
        auto& sell_list_at_price = iter_sell_map->second;
        for (auto iter_sell_list = (sell_list_at_price).begin(); iter_sell_list != sell_list_at_price.end(); ++iter_sell_list) {
            Order& order_at_iter = *iter_sell_list;
            std::cout << "id=" << order_at_iter.id << ":" << order_at_iter.quantity;

            if (&order_at_iter != &sell_list_at_price.back()) {
                std::cout << ", ";
            }
        }

        std::cout << "]" << std::endl;
    }

    std::cout << "**************************";
}