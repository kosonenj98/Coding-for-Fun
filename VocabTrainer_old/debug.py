# debug_variants.py
# Käyttö: python debug_variants.py
# Tulostaa kaikki forward/backward -parit valituille sanastoille ja aihealueille.

import json
from vocab_loader import load_index, load_vocab_files

def debug_print_all(index_path="index.json"):
    with open(index_path, "r", encoding="utf-8") as f:
        index_data = json.load(f)

    vocab = load_vocab_files(index_data)

    for vocab_name, groups in vocab.items():
        print(f"\n=== Sanasto: {vocab_name} ===\n")

        for group_name, group in groups.items():
            print(f"--- Aihealue: {group_name} ---\n")

            print("FORWARD (ita → fin):")
            for ita, fins in group["forward"].items():
                for fin in fins:
                    print(f"  {ita}  →  {fin}")

            print("\nBACKWARD (fin → ita):")
            for fin, itas in group["backward"].items():
                for ita in itas:
                    print(f"  {fin}  →  {ita}")

            print("\n")

if __name__ == "__main__":
    debug_print_all()
