import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import numpy as np
from collections import defaultdict
from collections import Counter

def parse_input(filepath):
    with open(filepath) as f:
        lines = f.read().splitlines()

    D, I, S, V, F = map(int, lines[0].split())

    streets = {
        name: (int(B), int(E), int(L))
        for B, E, name, L in (line.split() for line in lines[1:1+S])
    }

    cars = [
        line.split()[1:]
        for line in lines[1+S:1+S+V]
    ]

    return D, I, S, V, F, streets, cars


def compute_metrics(D, streets, cars):
    street_usage = Counter(s for path in cars for s in path)

    # Car slack: how many seconds of waiting a car can afford
    car_min_travel = [sum(streets[s][2] for s in path) for path in cars]
    car_slack = [D - t for t in car_min_travel]

    # Intersection incoming street count
    incoming = defaultdict(list)    # intersection -> list of street names
    for name, (B, E, L) in streets.items():
        incoming[E].append(name)

    # Intersection contention score: sum(usage/L) for all incoming streets
    contention = {}
    for node, street_list in incoming.items():
        score = sum(street_usage[s] / streets[s][2] for s in street_list)
        contention[node] = score

    # Street length vs usage (for scatter)
    street_lengths = [streets[s][2] for s in streets]
    street_usages  = [street_usage[s] for s in streets]

    return street_usage, car_slack, car_min_travel, incoming, contention, street_lengths, street_usages


BG      = "#1a1a2e"
PANEL   = "#16213e"
ACCENT  = "#0f3460"
CYAN    = "#00d4ff"
MAGENTA = "#e94560"
YELLOW  = "#f5a623"
GREEN   = "#4ecca3"
WHITE   = "#e0e0e0"
GRAY    = "#888888"

def style_ax(ax, title):
    ax.set_facecolor(PANEL)
    ax.tick_params(colors=WHITE, labelsize=8)
    ax.xaxis.label.set_color(WHITE)
    ax.yaxis.label.set_color(WHITE)
    ax.title.set_color(WHITE)
    ax.set_title(title, fontsize=10, fontweight="bold", pad=8)
    for spine in ax.spines.values():
        spine.set_edgecolor(ACCENT)


def plot_street_usage_histogram(ax, street_usage, streets):
    """How many streets have X cars using them."""
    usages = [street_usage.get(name, 0) for name in streets]
    max_u = max(usages) if usages else 1
    ax.hist(usages, bins=min(40, max_u + 1), color=CYAN, edgecolor=BG, linewidth=0.4)
    ax.set_xlabel("Cars using the street")
    ax.set_ylabel("Number of streets")
    style_ax(ax, "Street Usage Distribution")
    unused = sum(1 for u in usages if u == 0)
    ax.axvline(0.5, color=MAGENTA, linewidth=1, linestyle="--")
    ax.text(0.98, 0.95, f"{unused} streets unused",
            transform=ax.transAxes, color=MAGENTA,
            ha="right", va="top", fontsize=8)


def plot_top_streets(ax, street_usage, n=20):
    """Top N busiest streets by car count."""
    top = sorted(street_usage.items(), key=lambda x: -x[1])[:n]
    names  = [t[0] for t in top]
    counts = [t[1] for t in top]
    colors = plt.cm.YlOrRd(np.linspace(0.4, 1.0, len(names)))
    ax.barh(range(len(names)), counts, color=colors)
    ax.set_yticks(range(len(names)))
    ax.set_yticklabels(names, fontsize=7)
    ax.invert_yaxis()
    ax.set_xlabel("Cars")
    style_ax(ax, f"Top {n} Busiest Streets")


def plot_car_slack_histogram(ax, car_slack, D):
    """Distribution of car slack (D - minimum travel time)."""
    at_risk = sum(1 for s in car_slack if s < 0)
    tight   = sum(1 for s in car_slack if 0 <= s < 10)
    ax.hist(car_slack, bins=40, color=GREEN, edgecolor=BG, linewidth=0.4)
    ax.axvline(0,  color=MAGENTA, linewidth=1.5, linestyle="--", label="Deadline")
    ax.axvline(10, color=YELLOW,  linewidth=1,   linestyle=":",  label="10s margin")
    ax.set_xlabel("Slack (seconds of waiting affordable)")
    ax.set_ylabel("Number of cars")
    style_ax(ax, "Car Slack Distribution  (D - min travel time)")
    ax.legend(fontsize=7, facecolor=PANEL, labelcolor=WHITE, framealpha=0.7)
    ax.text(0.02, 0.95,
            f"Impossible: {at_risk}  |  Tight (<10s): {tight}",
            transform=ax.transAxes, color=YELLOW,
            ha="left", va="top", fontsize=8)


def plot_top_contention(ax, contention, n=20):
    """Top N intersections by contention score."""
    top = sorted(contention.items(), key=lambda x: -x[1])[:n]
    nodes  = [f"#{t[0]}" for t in top]
    scores = [t[1] for t in top]
    colors = plt.cm.plasma(np.linspace(0.3, 1.0, len(nodes)))
    ax.barh(range(len(nodes)), scores, color=colors)
    ax.set_yticks(range(len(nodes)))
    ax.set_yticklabels(nodes, fontsize=8)
    ax.invert_yaxis()
    ax.set_xlabel("Contention score  Σ(usage / L)")
    style_ax(ax, f"Top {n} Most Contested Intersections")


def plot_length_vs_usage(ax, street_lengths, street_usages):
    """Scatter: street length vs usage. Short+busy = worst bottlenecks."""
    sc = ax.scatter(street_lengths, street_usages,
                    c=street_usages, cmap="YlOrRd",
                    s=12, alpha=0.7, linewidths=0)
    ax.set_xlabel("Street length L (seconds)")
    ax.set_ylabel("Cars using street")
    style_ax(ax, "Street Length vs Usage  (top-left = bottlenecks)")
    cb = plt.colorbar(sc, ax=ax)
    cb.ax.yaxis.set_tick_params(color=WHITE)
    cb.ax.tick_params(labelcolor=WHITE)


def plot_incoming_streets_histogram(ax, incoming):
    """How many intersections have N incoming streets"""
    counts = [len(v) for v in incoming.values()]
    max_c = max(counts) if counts else 1
    # Cut off x-axis at 99th percentile to avoid long empty tail
    cutoff = int(np.percentile(counts, 99)) + 1
    filtered = [c for c in counts if c <= cutoff]
    ax.hist(filtered, bins=range(1, cutoff + 2), color=MAGENTA,
            edgecolor=BG, linewidth=0.4, align="left")
    ax.set_xlim(0.5, cutoff + 0.5)
    ax.set_xlabel("Number of incoming streets")
    ax.set_ylabel("Number of intersections")
    if cutoff <= 20:
        ax.set_xticks(range(1, cutoff + 1))
    else:
        ax.xaxis.set_major_locator(plt.MaxNLocator(integer=True, nbins=10))
    if cutoff < max_c:
        ax.text(0.98, 0.95, f"(axis capped at {cutoff}, max={max_c})",
                transform=ax.transAxes, color=GRAY,
                ha="right", va="top", fontsize=7, fontstyle="italic")
    style_ax(ax, "Scheduling Complexity  (incoming streets per intersection)")


input_files = ["a_example.in", "b_ocean.in", "c_checkmate.in", "d_daily_commute.in", "e_etoile.in", "f_forever_jammed.in"]

def process_file(filepath):
    D, I, S, V, F, streets, cars = parse_input(filepath)
    (street_usage, car_slack, car_min_travel,
     incoming, contention,
     street_lengths, street_usages) = compute_metrics(D, streets, cars)

    title = filepath.split("/")[-1]
    stem = title.rsplit(".", 1)[0]

    fig = plt.figure(figsize=(18, 12), facecolor=BG)
    fig.suptitle(
        f"Hash Code 2021 — {title}",
        color=WHITE, fontsize=14, fontweight="bold", y=0.98
    )

    fig.text(
        0.5, 0.945,
        f"duration = {D}  |  intersections = {I}  |  streets = {S}"
        f"  |  cars = {V}  |  bonus = {F}",
        color=GRAY, fontsize=9, ha="center", va="top", fontstyle="italic"
    )

    gs = gridspec.GridSpec(
        2, 3, figure=fig,
        hspace=0.45, wspace=0.35,
        left=0.06, right=0.97, top=0.88, bottom=0.07
    )

    axes = [fig.add_subplot(gs[i, j]) for i in range(2) for j in range(3)]
    ax1, ax2, ax3, ax4, ax5, ax6 = axes

    plot_street_usage_histogram(ax1, street_usage, streets)
    plot_top_streets(ax2, street_usage, n=20)
    plot_car_slack_histogram(ax3, car_slack, D)
    plot_top_contention(ax4, contention, n=20)
    plot_length_vs_usage(ax5, street_lengths, street_usages)
    plot_incoming_streets_histogram(ax6, incoming)

    fig.savefig(
        f"visualizers/{stem}_dashboard.png",
        dpi=150, bbox_inches="tight", facecolor=BG
    )
    plt.close(fig)


for filename in input_files:
    process_file(f"input_files/{filename}")