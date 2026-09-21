import json
import random
import tkinter as tk
from tkinter import ttk
import os

# ---------------------------------------------------------
# TALLENNUS
# ---------------------------------------------------------

def load_stats():
    if not os.path.exists("stats.json"):
        return {}
    with open("stats.json", "r", encoding="utf-8") as f:
        return json.load(f)

def save_stats(stats):
    with open("stats.json", "w", encoding="utf-8") as f:
        json.dump(stats, f, ensure_ascii=False, indent=2)

# ---------------------------------------------------------
# SANASTON LATAUS
# ---------------------------------------------------------

def load_index():
    with open("index.json", "r", encoding="utf-8") as f:
        return json.load(f)

def load_text_vocab(filename):
    vocab_name = None
    vocab = {}
    current_group = None

    with open(filename, "r", encoding="utf-8") as f:
        lines = [line.rstrip("\n") for line in f]

    # Poista tyhjät rivit
    lines = [line.strip() for line in lines if line.strip()]

    if not lines:
        raise ValueError(f"Tiedosto {filename} on tyhjä.")

    # Poista BOM
    if lines[0].startswith("\ufeff"):
        lines[0] = lines[0].replace("\ufeff", "")

    # Ensimmäinen rivi = sanaston nimi
    vocab_name = lines[0]
    vocab[vocab_name] = {}

    # Loput rivit
    for line in lines[1:]:
        # Ryhmän otsikko
        if "=" not in line:
            current_group = line
            vocab[vocab_name][current_group] = []
            continue

        # Italia = Suomi
        if current_group is None:
            print(f"Varoitus: Sana '{line}' ei kuulu mihinkään ryhmään. Ohitetaan.")
            continue

        ita, fin = line.split("=", 1)
        ita = ita.strip()
        fin = fin.strip()

        vocab[vocab_name][current_group].append({
            "ita": ita,
            "fin": fin
        })

    return vocab_name, vocab[vocab_name]

def load_vocab_files(index_data):
    vocab = {}
    for file in index_data["vocab_files"]:
        name, groups = load_text_vocab(file)
        vocab[name] = groups
    return vocab

# ---------------------------------------------------------
# APUFUNKTIOT
# ---------------------------------------------------------

def build_word_list(vocab, selected_groups, direction):
    words = []
    for group in selected_groups:
        for card in vocab[group]:
            if direction == "fi-it":
                words.append((card["fin"], card["ita"], card))   # suomi → italia
            else:
                words.append((card["ita"], card["fin"], card))   # italia → suomi
    return words

def pick_variant(text):
    if "/" not in text:
        return text
    parts = [p.strip() for p in text.split("/")]
    return random.choice(parts)

def stats_key(card):
    return f"{card['ita']}|{card['fin']}"

# ---------------------------------------------------------
# PAINOTETTU VALINTA
# ---------------------------------------------------------

def weighted_choice(words, stats):
    weights = []
    for front, back, card in words:
        key = stats_key(card)
        s = stats.get(key, {"correct": 0, "wrong": 0})
        w = (s["wrong"] + 1) / (s["correct"] + s["wrong"] + 2)
        weights.append(w)

    total = sum(weights)
    probs = [w / total for w in weights]

    r = random.random()
    cumulative = 0

    for (front, back, card), p in zip(words, probs):
        cumulative += p
        if r <= cumulative:
            return front, back, card

# ---------------------------------------------------------
# SOVELLUS
# ---------------------------------------------------------

class FlashcardApp:
    def __init__(self, root, vocab):
        self.root = root
        self.vocab = vocab
        self.stats = load_stats()
        self.words = []
        self.current = None          # (front, back)
        self.current_full = None     # {"ita": ..., "fin": ...}

        self.root.title("Harjoituskortit")
        self.root.geometry("900x600")

        self.setup_ui()

    # -----------------------------------------------------
    # KÄYTTÖLIITTYMÄ
    # -----------------------------------------------------

    def setup_ui(self):
        left = tk.Frame(self.root, width=250)
        left.pack(side="left", fill="y", padx=10, pady=10)

        # Sanasto
        tk.Label(left, text="Sanasto", font=("Arial", 14)).pack()
        self.vocab_var = tk.StringVar()
        self.vocab_menu = ttk.Combobox(left, textvariable=self.vocab_var)
        self.vocab_menu["values"] = list(self.vocab.keys())
        self.vocab_menu.pack(fill="x")

        # Suunta
        tk.Label(left, text="Suunta", font=("Arial", 14)).pack(pady=(20, 0))
        self.direction_var = tk.StringVar(value="fi-it")
        tk.Radiobutton(left, text="Suomi → Italia", variable=self.direction_var, value="fi-it").pack(anchor="w")
        tk.Radiobutton(left, text="Italia → Suomi", variable=self.direction_var, value="it-fi").pack(anchor="w")

        # Reagoi suunnanvaihtoon ajonaikaisesti
        self.direction_var.trace_add("write", self.on_direction_change)

        # Ryhmät
        tk.Label(left, text="Aihealueet", font=("Arial", 14)).pack(pady=(20, 0))
        self.group_vars = {}
        self.group_frame = tk.Frame(left)
        self.group_frame.pack(fill="x")

        # Päivitä ryhmät kun sanasto vaihtuu
        self.vocab_menu.bind("<<ComboboxSelected>>", self.update_groups)

        # Aloita
        tk.Button(left, text="Aloita harjoittelu", command=self.start_training).pack(pady=20)

        # -------------------------------------------------
        # Korttinäkymä
        # -------------------------------------------------

        self.card_frame = tk.Frame(self.root)
        self.card_frame.pack(expand=True)

        self.card_label = tk.Label(self.card_frame, text="", font=("Arial", 32), wraplength=600)
        self.card_label.pack(pady=40)

        self.entry = tk.Entry(self.card_frame, font=("Arial", 20))
        self.entry.pack()

        self.check_button = tk.Button(self.card_frame, text="Tarkista", command=self.check_answer)
        self.check_button.pack(pady=10)

        self.dontknow_button = tk.Button(self.card_frame, text="En tiedä", command=self.mark_wrong)
        self.dontknow_button.pack()

        self.next_button = tk.Button(self.card_frame, text="Seuraava", command=self.next_card)
        self.next_button.pack(pady=20)

    # -----------------------------------------------------
    # Ryhmien päivitys
    # -----------------------------------------------------

    def update_groups(self, event=None):
        for widget in self.group_frame.winfo_children():
            widget.destroy()

        selected = self.vocab_var.get()
        if not selected:
            return

        groups = self.vocab[selected]
        self.group_vars = {}

        for g in groups.keys():
            var = tk.BooleanVar(value=True)
            self.group_vars[g] = var
            tk.Checkbutton(self.group_frame, text=g, variable=var).pack(anchor="w")

    # -----------------------------------------------------
    # Harjoittelun aloitus
    # -----------------------------------------------------

    def start_training(self):
        selected_vocab = self.vocab_var.get()
        direction = self.direction_var.get()

        if not selected_vocab:
            return

        selected_groups = [g for g, v in self.group_vars.items() if v.get()]

        self.words = build_word_list(self.vocab[selected_vocab], selected_groups, direction)
        self.next_card()

    # -----------------------------------------------------
    # Kortin valinta
    # -----------------------------------------------------

    def next_card(self):
        if not self.words:
            return

        front, back, card = weighted_choice(self.words, self.stats)

        front = pick_variant(front)
        back = pick_variant(back)

        self.current = (front, back)
        self.current_full = card

        self.card_label.config(text=front)
        self.entry.delete(0, tk.END)

    # -----------------------------------------------------
    # Suunnanvaihto ajonaikaisesti
    # -----------------------------------------------------

    def on_direction_change(self, *args):
        if not self.current_full:
            return

        direction = self.direction_var.get()

        if direction == "fi-it":
            new_front = pick_variant(self.current_full["fin"])
            new_back = pick_variant(self.current_full["ita"])
        else:
            new_front = pick_variant(self.current_full["ita"])
            new_back = pick_variant(self.current_full["fin"])

        self.current = (new_front, new_back)
        self.card_label.config(text=new_front)
        self.entry.delete(0, tk.END)

    # -----------------------------------------------------
    # Tarkistus
    # -----------------------------------------------------

    def check_answer(self):
        if not self.current or not self.current_full:
            return

        front, back = self.current
        user = self.entry.get().strip()

        key = stats_key(self.current_full)

        if user.lower() == back.lower():
            self.stats.setdefault(key, {"correct": 0, "wrong": 0})
            self.stats[key]["correct"] += 1
            save_stats(self.stats)
            self.card_label.config(text=f"✔️ Oikein!\n{front} = {back}")
        else:
            self.stats.setdefault(key, {"correct": 0, "wrong": 0})
            self.stats[key]["wrong"] += 1
            save_stats(self.stats)
            self.card_label.config(text=f"❌ Väärin.\nOikea vastaus:\n{back}")

    # -----------------------------------------------------
    # En tiedä
    # -----------------------------------------------------

    def mark_wrong(self):
        if not self.current or not self.current_full:
            return

        front, back = self.current
        key = stats_key(self.current_full)

        self.stats.setdefault(key, {"correct": 0, "wrong": 0})
        self.stats[key]["wrong"] += 1
        save_stats(self.stats)

        self.card_label.config(text=f"🤔 Oikea vastaus:\n{back}")

# ---------------------------------------------------------
# PÄÄOHJELMA
# ---------------------------------------------------------

if __name__ == "__main__":
    index = load_index()
    vocab = load_vocab_files(index)

    root = tk.Tk()
    app = FlashcardApp(root, vocab)

    root.protocol("WM_DELETE_WINDOW", lambda: (save_stats(app.stats), root.destroy()))
    root.mainloop()
