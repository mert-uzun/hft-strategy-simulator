"""
HFT Strategy Simulator - Experiment Runner

Main entry point for running ping-pong strategy simulations.
All parameters are configurable via command-line arguments or programmatically.

Usage:
    python run_experiment.py                      # Run with defaults (balanced strategy)
    python run_experiment.py --strategy aggressive
    python run_experiment.py --strategy passive --duration 60000000
    python run_experiment.py --help               # See all options

Programmatic usage:
    from run_experiment import run_simulation
    from strategies import BALANCED
    results = run_simulation(strategy_config=BALANCED)
"""

import argparse
import sys
from typing import Dict, Any, Optional

# Add build directory to path for the compiled module
sys.path.insert(0, "../build")

try:
    import orderbook_wrapper as sim
except ImportError:
    print("ERROR: Could not import orderbook_wrapper.")
    print("Make sure you've built the project:")
    print("  cd build && cmake .. && make")
    sys.exit(1)

from strategies import STRATEGIES, BALANCED, DEFAULT_SIM_CONFIG, list_strategies


# =============================================================================
# SIMULATION RUNNER
# =============================================================================

def run_simulation(
    strategy_config: Optional[Dict[str, Any]] = None,
    sim_config: Optional[Dict[str, Any]] = None,
    verbose: bool = True,
) -> sim.Metrics:
    """
    Run a single simulation with the given configuration.

    Args:
        strategy_config: Strategy parameters dict (default: BALANCED)
        sim_config: Simulation parameters dict (default: DEFAULT_SIM_CONFIG)
        verbose: Whether to print progress messages

    Returns:
        Metrics object with all simulation results
    """
    # Use defaults if not provided
    strat = strategy_config or BALANCED
    conf = sim_config or DEFAULT_SIM_CONFIG

    if verbose:
        print(f"\n{'='*60}")
        print(f"Running simulation: {strat.get('name', 'Custom Strategy')}")
        print(f"{'='*60}")
        print(f"Strategy: quote_size={strat['quote_size']}, tick_offset={strat['tick_offset']}, "
              f"max_inv={strat['max_inv']}, cancel_threshold={strat['cancel_threshold']}, "
              f"cooldown_us={strat['cooldown_us']}")
        print(f"Market: duration={conf['ending_timestamp_us'] - conf['starting_timestamp_us']}us, "
              f"step={conf['step_us']}us, mid_price={conf['starting_mid_price']}, "
              f"spread={conf['start_spread']}, vol={conf['start_vol']}, fill_prob={conf['start_fill_prob']}")

    # Create and run simulation
    engine = sim.SimulationEngine(
        starting_timestamp_us=conf["starting_timestamp_us"],
        ending_timestamp_us=conf["ending_timestamp_us"],
        step_us=conf["step_us"],
        strategy_quote_size=strat["quote_size"],
        strategy_tick_offset=strat["tick_offset"],
        strategy_max_inv=strat["max_inv"],
        strategy_cancel_threshold=strat["cancel_threshold"],
        strategy_cooldown_between_requotes=strat["cooldown_us"],
        starting_mid_price=conf["starting_mid_price"],
        start_spread=conf["start_spread"],
        start_vol=conf["start_vol"],
        start_fill_prob=conf["start_fill_prob"],
    )

    if verbose:
        print("\nRunning simulation...")

    engine.run()

    if verbose:
        print("Simulation complete.\n")

    return engine.get_market_engine().get_metrics()


# =============================================================================
# METRICS DISPLAY
# =============================================================================

def print_summary(metrics: sim.Metrics, strategy_name: str = "Strategy") -> None:
    """
    Print a formatted summary of simulation results.

    Args:
        metrics: Metrics object from simulation
        strategy_name: Name for display purposes
    """
    print(f"\n{'='*60}")
    print(f"RESULTS: {strategy_name}")
    print(f"{'='*60}")

    # PnL Metrics
    print("\n--- PnL Metrics ---")
    print(f"  Total PnL (ticks):      {metrics.get_total_pnl_ticks():>12}")
    print(f"  Realized PnL (ticks):   {metrics.get_realized_pnl_ticks():>12}")
    print(f"  Unrealized PnL (ticks): {metrics.get_unrealized_pnl_ticks():>12}")
    print(f"  Fees Paid (ticks):      {metrics.fees_ticks:>12}")

    # Risk Metrics
    print("\n--- Risk Metrics ---")
    print(f"  Sharpe Ratio:           {metrics.get_sharpe_ratio():>12.4f}")
    print(f"  Max Drawdown (ticks):   {metrics.get_max_drawdown_ticks():>12}")
    print(f"  Volatility:             {metrics.get_volatility():>12.4f}")

    # Performance Metrics
    print("\n--- Performance Metrics ---")
    print(f"  Win Rate:               {metrics.get_win_rate():>12.2%}")
    print(f"  Profit Factor:          {metrics.get_profit_factor():>12.4f}")
    print(f"  Gross Profit:           {metrics.get_gross_profit():>12.4f}")
    print(f"  Gross Loss:             {metrics.get_gross_loss():>12.4f}")

    # Trading Activity
    print("\n--- Trading Activity ---")
    print(f"  Final Position:         {metrics.get_position():>12}")
    print(f"  Gross Traded Qty:       {metrics.get_gross_traded_qty():>12}")
    print(f"  Fill Ratio:             {metrics.get_fill_ratio():>12.2%}")
    print(f"  Total Slippage (ticks): {metrics.total_slippage_ticks:>12}")

    # Order Statistics
    print("\n--- Order Statistics ---")
    print(f"  Resting Attempted:      {metrics.resting_attempted_qty:>12}")
    print(f"  Resting Filled:         {metrics.resting_filled_qty:>12}")
    print(f"  Resting Cancelled:      {metrics.resting_cancelled_qty:>12}")

    # Time Series Info
    print("\n--- Time Series Data ---")
    print(f"  Data Points:            {len(metrics.timestamp_series):>12}")
    print(f"  Returns Buckets:        {len(metrics.returns_series):>12}")

    print(f"\n{'='*60}\n")


def compare_strategies(results: Dict[str, sim.Metrics]) -> None:
    """
    Print a comparison table of multiple strategy results.

    Args:
        results: Dict mapping strategy names to Metrics objects
    """
    print("\n" + "=" * 80)
    print("STRATEGY COMPARISON")
    print("=" * 80)

    # Header
    header = f"{'Metric':<25}"
    for name in results.keys():
        header += f"{name:>15}"
    print(header)
    print("-" * 80)

    # Metrics to compare
    comparisons = [
        ("Total PnL", lambda m: m.get_total_pnl_ticks()),
        ("Realized PnL", lambda m: m.get_realized_pnl_ticks()),
        ("Sharpe Ratio", lambda m: f"{m.get_sharpe_ratio():.4f}"),
        ("Max Drawdown", lambda m: m.get_max_drawdown_ticks()),
        ("Win Rate", lambda m: f"{m.get_win_rate():.2%}"),
        ("Profit Factor", lambda m: f"{m.get_profit_factor():.4f}"),
        ("Fill Ratio", lambda m: f"{m.get_fill_ratio():.2%}"),
        ("Gross Traded Qty", lambda m: m.get_gross_traded_qty()),
        ("Fees Paid", lambda m: m.fees_ticks),
    ]

    for metric_name, getter in comparisons:
        row = f"{metric_name:<25}"
        for metrics in results.values():
            value = getter(metrics)
            row += f"{value:>15}"
        print(row)

    print("=" * 80 + "\n")


# =============================================================================
# COMMAND LINE INTERFACE
# =============================================================================

def parse_args() -> argparse.Namespace:
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(
        description="HFT Ping-Pong Strategy Simulator",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python run_experiment.py                           # Balanced strategy, default config
  python run_experiment.py --strategy aggressive     # Use aggressive profile
  python run_experiment.py --compare                 # Compare all strategies
  python run_experiment.py --list                    # List available strategies
  python run_experiment.py --duration 60000000       # Run for 60 seconds
  python run_experiment.py --quote-size 5 --tick-offset 1  # Custom params
        """
    )

    # Strategy selection
    parser.add_argument(
        "--strategy", "-s",
        type=str,
        default="balanced",
        help="Strategy profile to use (default: balanced)"
    )
    parser.add_argument(
        "--list", "-l",
        action="store_true",
        help="List all available strategies and exit"
    )
    parser.add_argument(
        "--compare", "-c",
        action="store_true",
        help="Run all strategies and compare results"
    )

    # Strategy parameter overrides
    strat_group = parser.add_argument_group("Strategy Parameters (override profile)")
    strat_group.add_argument("--quote-size", type=int, help="Shares per ping order")
    strat_group.add_argument("--tick-offset", type=int, help="Ticks from mid for pings")
    strat_group.add_argument("--max-inv", type=int, help="Maximum inventory")
    strat_group.add_argument("--cancel-threshold", type=int, help="Ticks before cancel")
    strat_group.add_argument("--cooldown", type=int, help="Microseconds between requotes")

    # Simulation parameters
    sim_group = parser.add_argument_group("Simulation Parameters")
    sim_group.add_argument("--start", type=int, default=1, help="Start timestamp (us)")
    sim_group.add_argument("--duration", type=int, default=10_000_000, help="Duration (us), default: 10M (10s)")
    sim_group.add_argument("--step", type=int, default=100, help="Time step (us)")
    sim_group.add_argument("--mid-price", type=int, default=10000, help="Starting mid price (ticks)")
    sim_group.add_argument("--spread", type=int, default=2, help="Starting spread (ticks)")
    sim_group.add_argument("--volatility", type=float, default=1.0, help="Market volatility")
    sim_group.add_argument("--fill-prob", type=float, default=0.3, help="Base fill probability")

    # Output
    parser.add_argument("--quiet", "-q", action="store_true", help="Minimal output")

    return parser.parse_args()


def main() -> None:
    """Main entry point for CLI."""
    args = parse_args()

    # List strategies and exit
    if args.list:
        list_strategies()
        return

    # Build simulation config from args
    sim_config = {
        "starting_timestamp_us": args.start,
        "ending_timestamp_us": args.start + args.duration,
        "step_us": args.step,
        "starting_mid_price": args.mid_price,
        "start_spread": args.spread,
        "start_vol": args.volatility,
        "start_fill_prob": args.fill_prob,
    }

    # Compare all strategies
    if args.compare:
        results = {}
        for name, strat in STRATEGIES.items():
            metrics = run_simulation(
                strategy_config=strat,
                sim_config=sim_config,
                verbose=not args.quiet,
            )
            results[strat["name"]] = metrics
            if not args.quiet:
                print_summary(metrics, strat["name"])

        compare_strategies(results)
        return

    # Single strategy run
    if args.strategy.lower() not in STRATEGIES:
        print(f"ERROR: Unknown strategy '{args.strategy}'")
        print(f"Available: {', '.join(STRATEGIES.keys())}")
        print("Use --list to see strategy details")
        sys.exit(1)

    strategy = STRATEGIES[args.strategy.lower()].copy()

    # Apply any overrides
    if args.quote_size is not None:
        strategy["quote_size"] = args.quote_size
    if args.tick_offset is not None:
        strategy["tick_offset"] = args.tick_offset
    if args.max_inv is not None:
        strategy["max_inv"] = args.max_inv
    if args.cancel_threshold is not None:
        strategy["cancel_threshold"] = args.cancel_threshold
    if args.cooldown is not None:
        strategy["cooldown_us"] = args.cooldown

    # Run simulation
    metrics = run_simulation(
        strategy_config=strategy,
        sim_config=sim_config,
        verbose=not args.quiet,
    )

    print_summary(metrics, strategy["name"])


if __name__ == "__main__":
    main()
