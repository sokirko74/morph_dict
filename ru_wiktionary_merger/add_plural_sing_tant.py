"""
Adds the plural to nouns in -ие/-ье that have it in Wiktionary, but not in aot.ru, and marks them with sing_tant:
the plural is morphologically possible, but is not really used. The stress is on the stem in all new forms,
words with another stress are skipped. The lemmas are selected from result/compare/pairs.tsv by the differences.
The result should be saved by `slf_convert --to-mrd` with an empty slf file.
"""
import argparse
import json
from collections import Counter
from typing import Dict, List, Optional

DIFFERENCES = "N acc,neu,pl: -Я / 0 | N dat,neu,pl: -ЯМ / 0 | N gen,neu,pl: -Й / 0 | N ins,neu,pl: -ЯМИ / 0 | " \
              "N neu,nom,pl: -Я / 0 | N neu,pl,prp: -ЯХ / 0"
UNKNOWN_ACCENT_MODEL = 0xffff - 1
UNKNOWN_ACCENT = 255
VOWELS = "АЕЁИОУЫЭЮЯ"
# flexia model without the plural -> flexia model with it
MODELS = {64: 2760, 77: 2280, 2765: 139}
# common gramcode -> the same with sing_tant
SING_TANT = {"MM": "MP", "MP": "MP", "MJ": "MJ"}


def add_item(items: List, item) -> int:
    if item in items:
        return items.index(item)
    items.append(item)
    return len(items) - 1


def vowels(s: str) -> int:
    return sum(1 for c in s if c in VOWELS)


def full_accent_model(data: dict, old_flexia: int, old_accent: int, new_flexia: int) -> Optional[int]:
    if old_accent == UNKNOWN_ACCENT_MODEL:
        return old_accent
    old_endings = data["flexia_models"][old_flexia]["endings"]
    accents = data["accent_models"][old_accent]
    if all(a == UNKNOWN_ACCENT for a in accents):
        return add_item(data["accent_models"], [UNKNOWN_ACCENT] * len(data["flexia_models"][new_flexia]["endings"]))
    # an accent is the number of vowels after the stress, on the stem it is the same without the vowels of the ending
    stem_vowels = {a - vowels(e["flexia"]) for a, e in zip(accents, old_endings)}
    if len(stem_vowels) != 1 or min(stem_vowels) < 0:
        return None
    return add_item(data["accent_models"], [min(stem_vowels) + vowels(e["flexia"])
                                            for e in data["flexia_models"][new_flexia]["endings"]])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--pairs", default="result/compare/pairs.tsv")
    args = parser.parse_args()
    with open(args.input) as inp:
        data = json.load(inp)
    selected = {line.split("\t")[0] for line in open(args.pairs) if line.rstrip("\n").split("\t")[4] == DIFFERENCES}
    for no, new_no in MODELS.items():
        old = [(e["flexia"], e["gramcode"]) for e in data["flexia_models"][no]["endings"]]
        new = [(e["flexia"], e["gramcode"]) for e in data["flexia_models"][new_no]["endings"]]
        assert new[:len(old)] == old, no

    counts, skipped = Counter(), []
    for lemma in data["lemmas"]:
        if lemma["l"].replace("Ё", "Е") not in selected:
            continue
        parts = [[lemma["f"], lemma["a"]]] + lemma.get("n", [])
        if any(f not in MODELS for f, _ in parts) or lemma.get("t") not in SING_TANT:
            skipped.append(lemma["l"] + " models")
            continue
        new_parts = [[MODELS[f], full_accent_model(data, f, a, MODELS[f])] for f, a in parts]
        if any(a is None for _, a in new_parts):
            skipped.append(lemma["l"] + " stress")
            continue
        lemma["f"], lemma["a"] = new_parts[0]
        if len(new_parts) > 1:
            lemma["n"] = new_parts[1:]
        if lemma["t"] != SING_TANT[lemma["t"]]:
            counts["marked with sing_tant: " + lemma["l"]] += 1
            lemma["t"] = SING_TANT[lemma["t"]]
        counts["changed"] += 1
    with open(args.output, "w") as outp:
        json.dump(data, outp, ensure_ascii=False)
    print(len(selected), counts, "skipped", skipped)


if __name__ == "__main__":
    main()
