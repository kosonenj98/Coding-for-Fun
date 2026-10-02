import tkinter as tk
from tkinter import ttk
from word_logic import build_word_list, weighted_choice
from stats_manager import save_stats, stats_key, load_stats

def normalize_answer(s: str) -> str:
    """Poistaa ympäröivät välilyönnit, loppuvälimerkit ja normalisoi välilyönnit + pienet kirjaimet."""
    if s is None:
        return ""

    # Poista kaikki erikoismerkit fraasin sisältä
    s = s.replace(",", "")
    s = s.replace(".", "")
    s = s.replace("!", "")
    s = s.replace("?", "")
    s = s.replace(";", "")
    s = s.replace(":", "")

    # Korvaa useat välilyönnit yhdellä
    s = " ".join(s.split())

    # Tee case-insensitiiviseksi
    s = s.lower()

    # Poista ympäröivät välilyönnit
    s = s.strip()

    return s

class VocabTrainerApp:
    def __init__(self, root, vocab):
        self.root = root
        self.vocab = vocab
        self.stats = load_stats()
        self.words = []
        self.current = None
        self.current_full = None
        self.current_variants = []
        self.history = []
        self.history_index = -1
        self.awaiting_next = False
        self.recent = []   # pidetään viimeisimmät avaimet listassa
        self.recent_maxlen = 3  # kuinka monta viimeistä estetään
        self.last_key = None

        # Sessio‑kohtaiset tilat (eivät tallennu levylle)
        self.session_presented = set()
        self.session_answered = set()
        self.session_correct = set()

        self.root.title("Harjoituskortit")
        self.root.geometry("900x600")

        self.setup_ui()

    def setup_ui(self):
        left = tk.Frame(self.root, width=250)
        left.pack(side="left", fill="y", padx=10, pady=10)

        tk.Label(left, text="Sanasto", font=("Arial", 14)).pack()
        self.vocab_var = tk.StringVar()
        self.vocab_menu = ttk.Combobox(left, textvariable=self.vocab_var)
        self.vocab_menu["values"] = list(self.vocab.keys())
        self.vocab_menu.pack(fill="x")
        self.vocab_menu.bind("<<ComboboxSelected>>", self.on_vocab_change)

        tk.Label(left, text="Suunta", font=("Arial", 14)).pack(pady=(20, 0))
        self.direction_var = tk.StringVar(value="fi-it")
        tk.Radiobutton(left, text="suomi → italia", variable=self.direction_var, value="fi-it").pack(anchor="w")
        tk.Radiobutton(left, text="italia → suomi", variable=self.direction_var, value="it-fi").pack(anchor="w")
        self.direction_var.trace_add("write", self.on_direction_change)

        tk.Label(left, text="Aihealueet", font=("Arial", 14)).pack(pady=(20, 0))
        self.group_vars = {}
        self.group_frame = tk.Frame(left)
        self.group_frame.pack(fill="x")

        self.card_frame = tk.Frame(self.root)
        self.card_frame.pack(expand=True)

        self.card_label = tk.Label(self.card_frame, text="", font=("Arial", 32), wraplength=600)
        self.card_label.pack(pady=40)

        entry_row = tk.Frame(self.card_frame)
        entry_row.pack()

        self.entry = tk.Entry(entry_row, font=("Arial", 20), width=40)
        self.entry.pack(side="left", fill="x", expand=True)
        self.entry.bind("<Return>", self.on_enter)

        tk.Button(entry_row, text="Tarkista", command=self.check_answer).pack(side="left", padx=10)

        button_row = tk.Frame(self.card_frame)
        button_row.pack(pady=20)

        tk.Button(button_row, text="Edellinen", command=self.prev_card).pack(side="left", padx=10)
        tk.Button(button_row, text="En tiedä", command=self.mark_wrong).pack(side="left", padx=10)
        tk.Button(button_row, text="Seuraava", command=self.next_card).pack(side="left", padx=10)

        bottom = tk.Frame(self.root)
        bottom.pack(fill="x", pady=10)

        self.progress = ttk.Progressbar(bottom, orient="horizontal", mode="determinate")
        self.progress.pack(fill="x", padx=20)

        self.progress_label = tk.Label(bottom, text="0 / 0 oikein")
        self.progress_label.pack()

    def on_enter(self, event=None):
        """Enter-näppäimen keskitetty käsittely:
           - jos odotetaan seuraavaa korttia, siirrytään seuraavaan
           - muuten tarkistetaan vastaus"""
        if getattr(self, "awaiting_next", False):
            self.next_card()
        else:
            self.check_answer()

    def on_vocab_change(self, event=None):
        self.update_groups()
        self.start_training()

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
            var.trace_add("write", self.on_group_change)
            self.group_vars[g] = var
            tk.Checkbutton(self.group_frame, text=g, variable=var).pack(anchor="w")

    def on_group_change(self, *args):
        self.start_training()

    def start_training(self):
        self.session_presented.clear()
        self.session_answered.clear()
        self.session_correct.clear()

        selected_vocab = self.vocab_var.get()
        if not selected_vocab:
            return

        direction = self.direction_var.get()
        selected_groups = [g for g, v in self.group_vars.items() if v.get()]

        self.words = build_word_list(self.vocab[selected_vocab], selected_groups, direction)

        self.history = []
        self.history_index = -1

        self.update_progress()
        self.next_card()

    def get_all_back_variants(self, front, direction, selected_vocab, group):
        groups = self.vocab[selected_vocab][group]
        variants = set()

        if direction == "fi-it":
            variants.update(groups["backward"].get(front, []))
        else:
            variants.update(groups["forward"].get(front, []))

        return sorted(variants)

    def next_card(self):
        self.awaiting_next = False

        if not self.words:
            return

        # Käytä recent-listaa suoraan; jos recent on tyhjä, anna None
        recent_keys = set(self.recent) if self.recent else None

        front, back, card = weighted_choice(self.words, self.stats, recent_keys=recent_keys) 

        # Merkitse kortti "näytetyksi" (presented) jotta weighted_choice voi erottaa uudet kortit
        key = stats_key(card)

        # lisää recentiin ja pidä pituus maxissa
        self.recent.append(key)
        if len(self.recent) > self.recent_maxlen:
            self.recent.pop(0)

        self.session_presented.add(key)

        self.stats.setdefault(key, {"correct": 0, "wrong": 0})
        if not self.stats[key].get("presented", False):
            self.stats[key]["presented"] = True
            # älä vielä aseta answered True — käyttäjän pitää vastata tai painaa "En tiedä"
            save_stats(self.stats)

        self.current_full = card
        self.current = (front, back)

        selected_vocab = self.vocab_var.get()
        direction = self.direction_var.get()
        group = card["group"]

        self.current_variants = self.get_all_back_variants(front, direction, selected_vocab, group)

        self.card_label.config(text=front)
        self.entry.delete(0, tk.END)

        self.history.append((front, back, card))
        self.history_index = len(self.history) - 1

    def prev_card(self):
        self.awaiting_next = False

        if self.history_index <= 0:
            return

        self.history_index -= 1
        front, back, card = self.history[self.history_index]

        self.current_full = card
        self.current = (front, back)

        selected_vocab = self.vocab_var.get()
        direction = self.direction_var.get()
        group = card["group"]

        self.current_variants = self.get_all_back_variants(front, direction, selected_vocab, group)

        self.card_label.config(text=front)
        self.entry.delete(0, tk.END)

    def on_direction_change(self, *args):
        selected_vocab = self.vocab_var.get()
        if selected_vocab:
            selected_groups = [g for g, v in self.group_vars.items() if v.get()]
            direction = self.direction_var.get()
            self.words = build_word_list(self.vocab[selected_vocab], selected_groups, direction)
            self.update_progress()

    def check_answer(self):
        if not self.current or not self.current_full:
            return

        front, back = self.current  # front = mitä kortissa kysyttiin (prompt)
        user = self.entry.get().strip()
        key = stats_key(self.current_full)

        self.stats.setdefault(key, {"correct": 0, "wrong": 0})

        all_vals = self.current_variants or [back]

        user_norm = normalize_answer(user)
        normalized_vals = [normalize_answer(v) for v in all_vals]

        is_correct = user_norm in normalized_vals

        if is_correct:
            self.stats[key]["correct"] += 1
            result = "✔️ Oikein!"
            # Vasemmalla näytetään aina prompt eli front (se mitä kysyttiin)
            shown_left = front
            # Oikealla näytetään käyttäjän syöte
            shown_right = user
            # Muut hyväksytyt vaihtoehdot (alkuperäisillä muodoilla), poistetaan käyttäjän vastaus
            others = [v for v in all_vals if v.lower() != user_norm]

            # Merkitse sessiossa oikein
            self.session_correct.add(key)
        else:
            self.stats[key]["wrong"] += 1
            result = "❌ Väärin!"
            # Virheessä näytetään prompt ja ensimmäinen hyväksytty vaihtoehto
            shown_left = front
            shown_right = all_vals[0]
            others = [v for v in all_vals if v != shown_right]

        # Merkitse että korttiin on vastattu (pysyvä ja sessio)
        self.stats[key]["answered"] = True
        self.session_answered.add(key)

        save_stats(self.stats)
        self.update_progress()

        extra = ""
        if others:
            extra = " (myös: " + ", ".join(others) + ")"

        pair = f"{shown_left} = {shown_right}{extra}"
        self.card_label.config(text=f"{result}\n{pair}")

        self.awaiting_next = True
        # Aseta fokus entryyn, jotta Enter toimii suoraan
        self.entry.focus_set()

    def mark_wrong(self):
        if not self.current or not self.current_full:
            return

        front, back = self.current
        key = stats_key(self.current_full)

        self.stats.setdefault(key, {"correct": 0, "wrong": 0})
        self.stats[key]["wrong"] += 1
        self.stats[key]["answered"] = True
        save_stats(self.stats)

        self.session_answered.add(key)

        all_vals = self.current_variants or [back]
        shown_back = all_vals[0]

        extra = ""
        if len(all_vals) > 1:
            others = [v for v in all_vals if v != shown_back]
            if others:
                extra = " (myös: " + ", ".join(others) + ")"

        pair = f"{front} = {shown_back}{extra}"
        self.card_label.config(text=f"🤔 En tiedä\n{pair}")

        self.awaiting_next = True
        self.entry.focus_set()

        self.update_progress()

    def update_progress(self):
        total = len(self.words)
        if total == 0:
            self.progress["maximum"] = 1
            self.progress["value"] = 0
            self.progress_label.config(text="0 / 0 oikein")
            return

        # Lasketaan vain session aikana oikein vastatut kortit
        correct = 0
        for _, _, card in self.words:
            key = stats_key(card)
            if key in self.session_correct:
                correct += 1

        self.progress["maximum"] = total
        self.progress["value"] = correct
        self.progress_label.config(text=f"{correct} / {total} oikein")
