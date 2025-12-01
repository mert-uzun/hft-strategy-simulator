#pragma once

#include "Trade.h"
#include <list>

class TradeLog {
private:
  std::list<Trade> trades;

public:
  TradeLog();
  long long add_trade(long long buyId, long long sellId, long long priceTick,
                      int quantity, long long timestampUs, bool was_instant);
  void show_trades();
  std::list<Trade> &get_trades() { return trades; }
};