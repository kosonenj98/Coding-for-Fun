from vocab_loader import load_index, load_vocab_files
from stats_manager import load_stats, save_stats
from gui import FlashcardApp
import tkinter as tk

if __name__ == "__main__":
    index = load_index()
    vocab = load_vocab_files(index)

    root = tk.Tk()
    app = FlashcardApp(root, vocab)

    root.protocol("WM_DELETE_WINDOW", lambda: (save_stats(app.stats), root.destroy()))
    root.mainloop()
