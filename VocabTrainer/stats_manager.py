import json
import os

def load_stats():
    if not os.path.exists("stats.json"):
        return {}
    with open("stats.json", "r", encoding="utf-8") as f:
        return json.load(f)

def save_stats(stats):
    with open("stats.json", "w", encoding="utf-8") as f:
        json.dump(stats, f, ensure_ascii=False, indent=2)

def stats_key(card):
    return f"{card['ita']}|{card['fin']}"
