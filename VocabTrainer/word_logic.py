import random
from stats_manager import stats_key

def pick_variant(text):
    if "/" not in text:
        return text
    return random.choice([p.strip() for p in text.split("/")])

def build_word_list(vocab, selected_groups, direction):
    words = []
    for group in selected_groups:
        for card in vocab[group]:
            if direction == "fi-it":
                words.append((card["fin"], card["ita"], card))
            else:
                words.append((card["ita"], card["fin"], card))
    return words

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
