import json

def load_index():
    with open("index.json", "r", encoding="utf-8") as f:
        return json.load(f)

def load_text_vocab(filename):
    vocab_name = None
    vocab = {}
    current_group = None

    with open(filename, "r", encoding="utf-8") as f:
        lines = [line.rstrip("\n") for line in f]

    lines = [line.strip() for line in lines if line.strip()]

    if lines[0].startswith("\ufeff"):
        lines[0] = lines[0].replace("\ufeff", "")

    vocab_name = lines[0]
    vocab[vocab_name] = {}

    for line in lines[1:]:
        if "=" not in line:
            current_group = line
            vocab[vocab_name][current_group] = []
            continue

        ita, fin = line.split("=", 1)
        vocab[vocab_name][current_group].append({
            "ita": ita.strip(),
            "fin": fin.strip()
        })

    return vocab_name, vocab[vocab_name]

def load_vocab_files(index_data):
    vocab = {}
    for file in index_data["vocab_files"]:
        name, groups = load_text_vocab(file)
        vocab[name] = groups
    return vocab
