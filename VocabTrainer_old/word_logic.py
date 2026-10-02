import random
from stats_manager import stats_key

def build_word_list(vocab_groups, selected_groups, direction):
    """
    vocab_groups = sanaston groups-rakenne
    selected_groups = lista ryhmien nimistä
    direction = "fi-it" tai "it-fi"
    """

    words = []

    for group in selected_groups:
        grp = vocab_groups[group]

        if direction == "fi-it":
            # backward: suomi -> [italia1, italia2...]
            for fin, ita_list in grp["backward"].items():
                for ita in ita_list:
                    card = {
                        "front": fin,
                        "back": ita,
                        "group": group
                    }
                    words.append((fin, ita, card))

        else:  # it-fi
            # forward: italia -> [suomi1, suomi2...]
            for ita, fin_list in grp["forward"].items():
                for fin in fin_list:
                    card = {
                        "front": ita,
                        "back": fin,
                        "group": group
                    }
                    words.append((ita, fin, card))

    return words


def is_mastered(card, stats):
    key = stats_key(card)
    s = stats.get(key, {"correct": 0, "wrong": 0})
    return s["correct"] > 0


import random
from stats_manager import stats_key

def weighted_choice(words, stats, recent_keys=None, alpha=3.0, epsilon=0.001):
    """
    words: lista (front, back, card)
    stats: pysyvät statit
    recent_keys: set tai None; jos set, näitä avaimia ei oteta huomioon (cooldown)
    alpha: kuinka paljon 'wrong' kasvattaa painoa
    epsilon: minimipaino estämään täydellisen dominoinnin
    """

    new_unseen = []
    presented_unanswered = []
    wrong_words = []
    not_mastered_any = []
    mastered = []

    # Rakennetaan prioriteettipoolit, suodatetaan recent_keys heti alussa
    for front, back, card in words:
        key = stats_key(card)

        # Jos recent_keys on annettu ja tämä kortti on siellä, jätä se pois
        if recent_keys and key in recent_keys:
            continue

        s = stats.get(key, {"correct": 0, "wrong": 0})
        presented = s.get("presented", False)
        answered = s.get("answered", False)
        correct = s.get("correct", 0)
        wrong = s.get("wrong", 0)

        if correct == 0 and wrong == 0:
            if not presented:
                new_unseen.append((front, back, card))
            else:
                if not answered:
                    presented_unanswered.append((front, back, card))
                else:
                    not_mastered_any.append((front, back, card))
        elif wrong > 0:
            wrong_words.append((front, back, card))
        else:
            mastered.append((front, back, card))

    # 1) Uudet, joita ei ole koskaan esitetty
    if new_unseen:
        return random.choice(new_unseen)

    # 2) Esitetyt mutta vastaamattomat
    if presented_unanswered:
        return random.choice(presented_unanswered)

    # 3) Väärin menneet prioriteettina (heuristiikka niiden sisällä)
    if wrong_words:
        pool = wrong_words
        weights = []
        for front, back, card in pool:
            key = stats_key(card)
            s = stats.get(key, {"correct": 0, "wrong": 0})
            c = s.get("correct", 0)
            w = s.get("wrong", 0)
            weight = epsilon + (1.0 + alpha * w) / (1.0 + c)
            weights.append(weight)
        if not weights or sum(weights) == 0:
            return random.choice(pool)
        return random.choices(pool, weights=weights, k=1)[0]

    # 4) Jos ei vääriä, mutta on kortteja joita ei ole osattu (correct == 0)
    if not_mastered_any:
        return random.choice(not_mastered_any)

    # 5) Lopulta: kaikki kerran osattu -> heuristiikka koko joukossa
    pool = []
    weights = []
    for front, back, card in words:
        key = stats_key(card)
        if recent_keys and key in recent_keys:
            continue
        s = stats.get(key, {"correct": 0, "wrong": 0})
        c = s.get("correct", 0)
        w = s.get("wrong", 0)
        weight = epsilon + (1.0 + alpha * w) / (1.0 + c)
        pool.append((front, back, card))
        weights.append(weight)

    if not pool:
        # kaikki kortit olivat recent_keysissa -> palauta satunnainen ilman estoa
        return random.choice(words)

    return random.choices(pool, weights=weights, k=1)[0]
