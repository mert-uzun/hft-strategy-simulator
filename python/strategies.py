"""
Strategy Parameter Profiles for HFT Ping-Pong Simulator

This module contains predefined strategy configurations and utilities
for creating custom strategies. Each strategy is a dictionary of parameters
that can be passed to SimulationEngine.

Strategy Parameters:
    quote_size (int): Number of shares per ping order
    tick_offset (int): Distance from mid price for ping orders (in ticks)
    max_inv (int): Maximum inventory before reducing exposure
    cancel_threshold (int): Ticks away from mid before cancelling orders
    cooldown_us (int): Microseconds between requotes
"""

from typing import Dict, Any

# =============================================================================
# PREDEFINED STRATEGY PROFILES
# =============================================================================

AGGRESSIVE = {
    "name": "Aggressive",
    "description": "Tight spreads, high frequency, large positions. Higher risk/reward.",
    "quote_size": 5,
    "tick_offset": 1,
    "max_inv": 20,
    "cancel_threshold": 1,
    "cooldown_us": 1000,  # 1ms
}

BALANCED = {
    "name": "Balanced",
    "description": "Moderate parameters. Good starting point for most conditions.",
    "quote_size": 3,
    "tick_offset": 2,
    "max_inv": 10,
    "cancel_threshold": 2,
    "cooldown_us": 5000,  # 5ms
}

PASSIVE = {
    "name": "Passive",
    "description": "Wider spreads, lower frequency, smaller positions. Lower risk.",
    "quote_size": 1,
    "tick_offset": 3,
    "max_inv": 5,
    "cancel_threshold": 3,
    "cooldown_us": 10000,  # 10ms
}

# Collection of all predefined strategies
STRATEGIES = {
    "aggressive": AGGRESSIVE,
    "balanced": BALANCED,
    "passive": PASSIVE,
}


# =============================================================================
# CUSTOM STRATEGY BUILDER
# =============================================================================

def create_strategy(
    name: str,
    quote_size: int = 3,
    tick_offset: int = 2,
    max_inv: int = 10,
    cancel_threshold: int = 2,
    cooldown_us: int = 5000,
    description: str = "",
) -> Dict[str, Any]:
    """
    Create a custom strategy configuration.

    Args:
        name: Name identifier for this strategy
        quote_size: Number of shares per ping order (default: 3)
        tick_offset: Ticks from mid for ping orders (default: 2)
        max_inv: Maximum inventory limit (default: 10)
        cancel_threshold: Ticks before cancel trigger (default: 2)
        cooldown_us: Microseconds between requotes (default: 5000)
        description: Optional description of the strategy

    Returns:
        Strategy configuration dictionary
    
    Example:
        >>> my_strategy = create_strategy(
        ...     name="Ultra Aggressive",
        ...     quote_size=10,
        ...     tick_offset=1,
        ...     max_inv=50,
        ...     cancel_threshold=1,
        ...     cooldown_us=500,
        ...     description="Very high frequency, large size"
        ... )
    """
    return {
        "name": name,
        "description": description,
        "quote_size": quote_size,
        "tick_offset": tick_offset,
        "max_inv": max_inv,
        "cancel_threshold": cancel_threshold,
        "cooldown_us": cooldown_us,
    }


def register_strategy(strategy: Dict[str, Any]) -> None:
    """
    Register a custom strategy to the global STRATEGIES dict.

    Args:
        strategy: Strategy dict created via create_strategy()
    
    Example:
        >>> my_strat = create_strategy("Custom", quote_size=7)
        >>> register_strategy(my_strat)
        >>> STRATEGIES["custom"]  # Now accessible
    """
    key = strategy["name"].lower().replace(" ", "_")
    STRATEGIES[key] = strategy


def list_strategies() -> None:
    """Print all available strategies with their parameters."""
    print("\n" + "=" * 60)
    print("AVAILABLE STRATEGIES")
    print("=" * 60)
    for key, strat in STRATEGIES.items():
        print(f"\n[{key}] {strat['name']}")
        if strat.get("description"):
            print(f"  {strat['description']}")
        print(f"  quote_size={strat['quote_size']}, tick_offset={strat['tick_offset']}, "
              f"max_inv={strat['max_inv']}, cancel_threshold={strat['cancel_threshold']}, "
              f"cooldown_us={strat['cooldown_us']}")
    print("\n" + "=" * 60)


# =============================================================================
# SIMULATION CONFIG DEFAULTS
# =============================================================================

DEFAULT_SIM_CONFIG = {
    "starting_timestamp_us": 1,
    "ending_timestamp_us": 10_000_000,  # 10 seconds
    "step_us": 100,
    "starting_mid_price": 10000,
    "start_spread": 2,
    "start_vol": 1.0,
    "min_volatility": 0.5,
    "start_fill_prob": 0.3,
}


def create_sim_config(
    starting_timestamp_us: int = 1,
    ending_timestamp_us: int = 10_000_000,
    step_us: int = 100,
    starting_mid_price: int = 10000,
    start_spread: int = 2,
    start_vol: float = 1.0,
    min_volatility: float = 0.5,
    start_fill_prob: float = 0.3,
) -> Dict[str, Any]:
    """
    Create a simulation configuration.

    Args:
        starting_timestamp_us: Start time in microseconds (default: 0)
        ending_timestamp_us: End time in microseconds (default: 10,000,000 = 10s)
        step_us: Time step in microseconds (default: 100)
        starting_mid_price: Initial mid price in ticks (default: 10000)
        start_spread: Initial bid-ask spread in ticks (default: 2)
        start_vol: Market volatility parameter (default: 1.0)
        start_fill_prob: Base fill probability (default: 0.3)

    Returns:
        Simulation configuration dictionary
    """
    return {
        "starting_timestamp_us": starting_timestamp_us,
        "ending_timestamp_us": ending_timestamp_us,
        "step_us": step_us,
        "starting_mid_price": starting_mid_price,
        "start_spread": start_spread,
        "start_vol": start_vol,
        "min_volatility": min_volatility,
        "start_fill_prob": start_fill_prob,
    }
