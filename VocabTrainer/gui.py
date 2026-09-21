import tkinter as tk
from tkinter import ttk
from word_logic import build_word_list, weighted_choice, pick_variant
from stats_manager import save_stats, stats_key

class FlashcardApp:
    def __init__(self, root, vocab):
        self.root = root
        self.vocab = vocab
        self.stats = {}
        self.words = []
        self.current = None
        self.current_full = None

        self.root.title("Harjoituskortit")
        self.root.geometry("900x600")

        self.setup_ui()

    def setup_ui(self):
        left = tk.Frame(self.root, width=250)
        left.pack(side="left", fill="y", padx=10, pady=10)

        tk.Label(left, text="Sanasto", font=("Arial", 14)).pack()
        self.vocab_var = tk.StringVar()
        self.vocab_menu = ttk.Combobox(left, textvariable=self.vocab_var)
        self.vocab_menu.pack(fill="x")

        tk.Label(left, text="Suunta", font=("Arial", 14)).pack(pady=(20, 0))
        self.direction_var = tk.StringVar(value="fi-it")
        tk.Radiobutton(left, text="Suomi → Italia", variable=self.direction_var, value="fi-it").pack(anchor="w")
        tk.Radiobutton(left, text="Italia → Suomi", variable=self.direction_var, value="it-fi").pack(anchor="w")
        self.direction_var.trace_add("write", self.on_direction_change)

        tk.Label(left, text="Aihealueet", font=("Arial", 14)).pack(pady=(20, 0))
        self.group_vars = {}
        self.group_frame = tk.Frame(left)
        self.group_frame.pack(fill="x")

        self.vocab_menu.bind("<<ComboboxSelected>>", self.update_groups)

        tk.Button(left, text="Aloita harjoittelu", command=self.start_training).pack(pady=20)

        self.card_frame = tk.Frame(self.root)
        self.card_frame.pack(expand=True)

        self.card_label = tk.Label(self.card_frame, text="", font=("Arial", 32), wraplength=600)
        self.card_label.pack(pady=40)

        self.entry = tk.Entry(self.card_frame, font=("Arial", 20))
        self.entry.pack()

        tk.Button(self.card_frame, text="Tarkista", command=self.check_answer).pack(pady=10)
        tk.Button(self.card_frame, text="En tiedä", command=self.mark_wrong).pack()
        tk.Button(self.card_frame, text="Seuraava", command=self.next_card).pack(pady=20)

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

    def start_training(self):
        selected_vocab = self.vocab_var.get()
        direction = self.direction_var.get()

        selected_groups = [g for g, v in self.group_vars.items() if v.get()]
        self.words = build_word_list(self.vocab[selected_vocab], selected_groups, direction)
        self.next_card()

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

    def check_answer(self):
        if not self.current or not self.current_full:
            return

        front, back = self.current
        user = self.entry.get().strip()
        key = stats_key(self.current_full)

        self.stats.setdefault(key, {"correct": 0, "wrong": 0})

        if user.lower() == back.lower():
            self.stats[key]["correct"] += 1
            self.card_label.config(text=f"✔️ Oikein!\n{front} = {back}")
        else:
            self.stats[key]["wrong"] += 1
            self.card_label.config(text=f"❌ Väärin.\nOikea vastaus:\n{back}")

        save_stats(self.stats)

    def mark_wrong(self):
        if not self.current or not self.current_full:
            return

        key = stats_key(self.current_full)
        self.stats.setdefault(key, {"correct": 0, "wrong": 0})
        self.stats[key]["wrong"] += 1
        save_stats(self.stats)

        self.card_label.config(text=f"🤔 Oikea vastaus:\n{self.current[1]}")
