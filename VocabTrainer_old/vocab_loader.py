# word_loader.py
# Korjattu varianttimoottori: rekursiivinen ()-purku ja []-laajennus
# Käyttää: load_text_vocab(filename) ja load_vocab_files(index_data)

import json
from typing import List, Tuple

# ============================================================
#  APUFUNKTIOT
# ============================================================

def split_options(s: str) -> List[str]:
    """Jaa vaihtoehdot /-merkin mukaan ja trimmaa välilyönnit.
       EI poista duplikaatteja, säilyttää alkuperäisen järjestyksen ja määrän."""
    return [p.strip() for p in s.split("/") if p.strip() != ""]

def _normalize_spaces(text: str) -> str:
    return " ".join(text.split()).strip()

# ------------------------------------------------------------
# Etsi viimeinen avausmerkki ja sen vastaava sulku
# Skipataan avausmerkit, jotka ovat hakasulkeiden sisällä (tällöin [] käsitellään atomina)
# ------------------------------------------------------------
def find_matching(text: str, open_char: str, close_char: str):
    """Etsii viimeisen open_char ja sen vastaavan close_char huomioiden sisäkkäisyydet.
       Palauttaa (start, end) tai None jos ei löydy."""
    scan_pos = text.rfind(open_char)
    while scan_pos != -1:
        # tarkista onko tämä open_char hakasulkeiden sisällä
        lb = text.rfind('[', 0, scan_pos)
        if lb != -1:
            # etsi vastaava ']' alkaen lb:stä
            depth = 0
            found_close = None
            for i in range(lb + 1, len(text)):
                if text[i] == '[':
                    depth += 1
                elif text[i] == ']':
                    if depth == 0:
                        found_close = i
                        break
                    depth -= 1
            if found_close is not None and scan_pos < found_close:
                # tämä open_char on hakasulkeiden sisällä -> etsi aikaisempi open_char
                scan_pos = text.rfind(open_char, 0, scan_pos)
                continue
        # etsi vastaava sulku huomioiden sisäkkäisyydet
        depth = 0
        for j in range(scan_pos + 1, len(text)):
            c = text[j]
            if c == open_char:
                depth += 1
            elif c == close_char:
                if depth == 0:
                    return scan_pos, j
                depth -= 1
        # jos ei löydy vastaavaa sulkua, etsi aikaisempi open_char
        scan_pos = text.rfind(open_char, 0, scan_pos)
    return None

def find_parentheses(text: str):
    return find_matching(text, "(", ")")

def find_brackets(text: str):
    return find_matching(text, "[", "]")

# ============================================================
#  REKURSIIVINEN ()-PURKU
# ============================================================

def expand_one_parenthesis(left: str, right: str, pos_left, pos_right) -> List[Tuple[str, str]]:
    """Laajentaa yhden löydetyn ()-parin (vain vasemmalla, vain oikealla tai molemmilla)."""
    # VAIN VASEMMALLA
    if pos_left and not pos_right:
        start, end = pos_left
        inside = left[start+1:end]
        before = left[:start]
        after = left[end+1:]
        variants = []
        variants.append((_normalize_spaces(before + after), right))            # exclude
        variants.append((_normalize_spaces(before + inside + after), right))  # include
        return variants

    # VAIN OIKEALLA
    if pos_right and not pos_left:
        start, end = pos_right
        inside = right[start+1:end]
        before = right[:start]
        after = right[end+1:]
        variants = []
        variants.append((left, _normalize_spaces(before + after)))
        variants.append((left, _normalize_spaces(before + inside + after)))
        return variants

    # MOLEMMILLA -> synkronoitu include/exclude 1:1
    if pos_left and pos_right:
        start_l, end_l = pos_left
        start_r, end_r = pos_right
        inside_l = left[start_l+1:end_l]
        inside_r = right[start_r+1:end_r]
        before_l = left[:start_l]
        after_l = left[end_l+1:]
        before_r = right[:start_r]
        after_r = right[end_r+1:]
        variants = []
        # exclude both
        variants.append((_normalize_spaces(before_l + after_l), _normalize_spaces(before_r + after_r)))
        # include both (paritetaan sellaisenaan)
        variants.append((_normalize_spaces(before_l + inside_l + after_l), _normalize_spaces(before_r + inside_r + after_r)))
        return variants

    return [(left, right)]

def expand_all_parentheses(left: str, right: str) -> List[Tuple[str, str]]:
    """Rekursiivinen laajennus: etsi viimeinen sopiva '(' (ei hakasulkeiden sisällä),
       laajenna ja kutsu rekursiivisesti jokaiselle syntyneelle parille."""
    pos_l = find_parentheses(left)
    pos_r = find_parentheses(right)
    if not pos_l and not pos_r:
        return [(left, right)]
    # laajenna juuri löydetty kohta
    variants = expand_one_parenthesis(left, right, pos_l, pos_r)
    result = []
    for l, r in variants:
        # rekursiivinen kutsu jatkaa kunnes ei enää sulkeita
        result.extend(expand_all_parentheses(l, r))
    # poista duplikaatit säilyttäen järjestyksen
    seen = set()
    out = []
    for p in result:
        if p not in seen:
            seen.add(p)
            out.append(p)
    return out

# ============================================================
#  REKURSIIVINEN []-PURKU
# ============================================================

def expand_one_bracket(left: str, right: str, pos_left, pos_right) -> List[Tuple[str, str]]:
    """Laajentaa yhden []-osion (vain vasemmalla, vain oikealla tai molemmilla)."""
    if pos_left and not pos_right:
        start, end = pos_left
        inside = left[start+1:end]
        before = left[:start]
        after = left[end+1:]
        opts = split_options(inside)
        return [(_normalize_spaces(before + o + after), right) for o in opts]

    if pos_right and not pos_left:
        start, end = pos_right
        inside = right[start+1:end]
        before = right[:start]
        after = right[end+1:]
        opts = split_options(inside)
        return [(left, _normalize_spaces(before + o + after)) for o in opts]

    if pos_left and pos_right:
        start_l, end_l = pos_left
        start_r, end_r = pos_right
        inside_l = left[start_l+1:end_l]
        inside_r = right[start_r+1:end_r]
        before_l = left[:start_l]
        after_l = left[end_l+1:]
        before_r = right[:start_r]
        after_r = right[end_r+1:]
        opts_l = split_options(inside_l)
        opts_r = split_options(inside_r)
        variants = []
        if len(opts_l) == len(opts_r):
            for o_l, o_r in zip(opts_l, opts_r):
                variants.append((_normalize_spaces(before_l + o_l + after_l), _normalize_spaces(before_r + o_r + after_r)))
        else:
            for o_l in opts_l:
                for o_r in opts_r:
                    variants.append((_normalize_spaces(before_l + o_l + after_l), _normalize_spaces(before_r + o_r + after_r)))
        return variants

    return [(left, right)]

def expand_all_brackets(left: str, right: str) -> List[Tuple[str, str]]:
    """Rekursiivinen []-laajennus: etsi viimeinen '[' ja laajenna, jatka rekursiivisesti."""
    pos_l = find_brackets(left)
    pos_r = find_brackets(right)
    if not pos_l and not pos_r:
        return [(left, right)]
    variants = expand_one_bracket(left, right, pos_l, pos_r)
    result = []
    for l, r in variants:
        result.extend(expand_all_brackets(l, r))
    # poista duplikaatit
    seen = set()
    out = []
    for p in result:
        if p not in seen:
            seen.add(p)
            out.append(p)
    return out

# ============================================================
#  YHDISTETTY VARIANTTIGENEROINTI
# ============================================================

def expand_variants_pair(left: str, right: str) -> List[Tuple[str, str]]:
    """
    1) Laajenna kaikki ()-variaatiot (rekursiivisesti).
    2) Laajenna hakasulkeet [] (rekursiivisesti).
    3) Suorita vielä toinen ()-kierros tuloksille, jotta []-sisäiset () laajenevat.
    """
    # 1) ensin kaikki ()-variaatiot
    pairs = expand_all_parentheses(left, right)

    # 2) sitten hakasulkeet [] (vaihtoehdot)
    after_brackets = []
    for l, r in pairs:
        after_brackets.extend(expand_all_brackets(l, r))

    # 3) suorita vielä ()-laajennus tuloksille, jotta []-sisäiset () laajenevat
    final = []
    for l, r in after_brackets:
        final.extend(expand_all_parentheses(l, r))

    # poista duplikaatit säilyttäen järjestyksen
    seen = set()
    result = []
    for p in final:
        if p not in seen:
            seen.add(p)
            result.append(p)
    return result

# ============================================================
#  SANASTON LATAUS
# ============================================================

def load_index():
    with open("index.json", "r", encoding="utf-8") as f:
        return json.load(f)

def load_text_vocab(filename: str):
    """Lataa tekstimuotoisen sanaston tiedostosta.
       Tiedoston ensimmäinen rivi on sanaston nimi, seuraavat rivit ryhmiä ja pareja.
       Esimerkki:
         VocabName
         Group1
         vasen = oikea
         ...
    """
    with open(filename, "r", encoding="utf-8") as f:
        lines = [line.strip() for line in f if line.strip()]

    if not lines:
        raise ValueError("Tiedosto on tyhjä: " + filename)

    if lines[0].startswith("\ufeff"):
        lines[0] = lines[0].replace("\ufeff", "")

    vocab_name = lines[0]
    groups = {}
    current_group = None

    for line in lines[1:]:
        if "=" not in line:
            current_group = line
            groups[current_group] = {
                "name": current_group,
                "forward": {},
                "backward": {}
            }
            continue

        left, right = line.split("=", 1)
        left = left.strip()
        right = right.strip()

        pairs = expand_variants_pair(left, right)

        for l, r in pairs:
            groups[current_group]["forward"].setdefault(l, [])
            if r not in groups[current_group]["forward"][l]:
                groups[current_group]["forward"][l].append(r)

            groups[current_group]["backward"].setdefault(r, [])
            if l not in groups[current_group]["backward"][r]:
                groups[current_group]["backward"][r].append(l)

    return vocab_name, groups

def load_vocab_files(index_data):
    vocab = {}
    for file in index_data["vocab_files"]:
        name, groups = load_text_vocab(file)
        vocab[name] = groups
    return vocab