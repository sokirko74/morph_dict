"""
Splits the merged paradigms of aot.ru with the variants -ие/-ье (воскресение, воскресенье) into two parts of a word,
as they are shown in Wiktionary: the old flexia model keeps the series -ие, the series -ье becomes the next part.
The genitive plural -ий belongs to both series. The forms of a word do not change, this is checked for every word.
The result is json in the wrong format, it should be saved by `slf_convert --to-mrd` with an empty slf file.
"""
import argparse
import json
from collections import Counter
from typing import Dict, List, Set, Tuple

UNKNOWN_ACCENT_MODEL = 0xffff - 1
UNKNOWN_ACCENT = 255


def gramcodes(ending: dict) -> List[str]:
    codes = ending["gramcode"]
    return [codes[i:i + 2] for i in range(0, len(codes), 2)]


def is_merged(model: dict) -> bool:
    endings = model["endings"]
    if not endings[0]["flexia"] == "ИЕ":
        return False
    return any(e["flexia"] == "ЬЕ" and set(gramcodes(e)) & set(gramcodes(endings[0])) for e in endings)


def split_indices(model: dict) -> Tuple[List[int], List[int]]:
    first, second = [], []
    for i, ending in enumerate(model["endings"]):
        if ending["flexia"].startswith("Ь"):
            second.append(i)
        else:
            first.append(i)
            if ending["flexia"] == "ИЙ":
                second.append(i)
    return first, second


def add_item(items: List, item) -> int:
    if item in items:
        return items.index(item)
    items.append(item)
    return len(items) - 1


def word_lines(lemma: dict, models: List[dict], accents: List[List[int]]) -> Set[Tuple[str, str, int]]:
    result = set()
    for flexia_no, accent_no in [(lemma["f"], lemma["a"])] + [tuple(p) for p in lemma.get("n", [])]:
        for i, ending in enumerate(models[flexia_no]["endings"]):
            accent = UNKNOWN_ACCENT if accent_no == UNKNOWN_ACCENT_MODEL else accents[accent_no][i]
            for code in gramcodes(ending):
                result.add((ending["flexia"], code, accent))
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()
    with open(args.input) as inp:
        data = json.load(inp)
    models, accents, lemmas = data["flexia_models"], data["accent_models"], data["lemmas"]

    merged = {no for no, model in enumerate(models) if is_merged(model)}
    words = [lemma for lemma in lemmas if lemma["f"] in merged]
    before = [word_lines(lemma, models, accents) for lemma in words]
    splits = {no: split_indices(models[no]) for no in merged}

    second_models: Dict[int, int] = {}
    for no, (_, second) in splits.items():
        endings = [models[no]["endings"][i] for i in second]
        for other_no, other in enumerate(models):
            if other["endings"] == endings:
                second_models[no] = other_no
                break
        else:
            models.append({"endings": endings})
            second_models[no] = len(models) - 1

    accent_pairs: Dict[Tuple[int, int], Tuple[int, int]] = {}
    for lemma in words:
        no, accent_no = lemma["f"], lemma["a"]
        if accent_no == UNKNOWN_ACCENT_MODEL:
            lemma["n"] = [[second_models[no], UNKNOWN_ACCENT_MODEL]]
            continue
        key = (no, accent_no)
        if key not in accent_pairs:
            first, second = splits[no]
            accent_pairs[key] = (add_item(accents, [accents[accent_no][i] for i in first]),
                                 add_item(accents, [accents[accent_no][i] for i in second]))
        lemma["a"] = accent_pairs[key][0]
        lemma["n"] = [[second_models[no], accent_pairs[key][1]]]

    for no, (first, _) in splits.items():
        models[no]["endings"] = [models[no]["endings"][i] for i in first]

    for lemma, lines in zip(words, before):
        if word_lines(lemma, models, accents) != lines:
            raise Exception(f"the forms of {lemma['l']} have changed")

    with open(args.output, "w") as outp:
        json.dump(data, outp, ensure_ascii=False)
    print(f"models: {sorted(merged)}, words: {len(words)}, second parts: {second_models}")
    print(f"flexia models: {len(models)}, accent models: {len(accents)}, new accent pairs: {len(accent_pairs)}")
    print(Counter(lemma["f"] for lemma in words))


if __name__ == "__main__":
    main()
