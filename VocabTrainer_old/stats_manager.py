import json
import os
from typing import Dict, Any

STATS_FILE = "stats.json"
TEMP_FILE = STATS_FILE + ".tmp"

def load_stats() -> Dict[str, Any]:
    if not os.path.exists(STATS_FILE):
        return {}
    try:
        with open(STATS_FILE, "r", encoding="utf-8") as f:
            data = json.load(f)
            if isinstance(data, dict):
                return data
            return {}
    except (json.JSONDecodeError, OSError):
        # jos tiedosto on tyhjä tai rikki, palauta tyhjä dict
        return {}

def save_stats(stats: Dict[str, Any]) -> None:
    # Kirjoitetaan ensin temp-tiedostoon ja korvataan atomisesti
    try:
        with open(TEMP_FILE, "w", encoding="utf-8") as f:
            json.dump(stats, f, ensure_ascii=False, indent=2)
            f.flush()
            os.fsync(f.fileno())
        os.replace(TEMP_FILE, STATS_FILE)
    except OSError:
        # jos atominen korvaus epäonnistuu, yritetään tavallisesti
        with open(STATS_FILE, "w", encoding="utf-8") as f:
            json.dump(stats, f, ensure_ascii=False, indent=2)

def stats_key(card: dict) -> str:
    return f"{card['front']}|{card['back']}"
