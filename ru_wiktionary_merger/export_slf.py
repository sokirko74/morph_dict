"""
Writes the generated paradigms (result/*.jsonl) as one slf file of MorphWizard, see src/slf.py.
The slf file is loaded into a dictionary of aot.ru with `slf_to_mrd` (morph_dict).
"""
import argparse
import json
import os
from collections import Counter

from src.slf import TParadigm, adjective_paradigm, noun_paradigm, other_paradigms, surname_paradigm, \
    verb_paradigm

SOURCES = ["nouns", "adjectives", "verbs", "other", "surnames"]


def article_paradigms(source: str, article: dict):
    if source == "nouns":
        paradigm, reason = noun_paradigm(article)
        return [paradigm] if paradigm else [], reason
    if source == "adjectives":
        return [adjective_paradigm(article)], ""
    if source == "verbs":
        paradigm, reason = verb_paradigm(article)
        return [paradigm] if paradigm else [], reason
    if source == "surnames":
        return [surname_paradigm(article)], ""
    return other_paradigms(article)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input-folder", default="result")
    parser.add_argument("--output", default="result/wiktionary.slf")
    parser.add_argument("--failures", default="result/slf_failures.tsv")
    args = parser.parse_args()
    counts = Counter()
    with open(args.output, "w") as outp, open(args.failures, "w") as failures:
        failures.write("source\ttitle\ttemplate\treason\n")
        for source in SOURCES:
            path = os.path.join(args.input_folder, source + ".jsonl")
            if not os.path.exists(path):
                print(f"no file {path}")
                continue
            for line in open(path):
                article = json.loads(line)
                template = article["morpho"].get("template", "")
                paradigms, reason = article_paradigms(source, article)
                for paradigm in paradigms:
                    paradigm: TParadigm
                    if paradigm.skipped:
                        failures.write(f"{source}\t{article['title']}\t{template}\t"
                                       f"пропущены формы {', '.join(paradigm.skipped)}\n")
                    if not paradigm.lines:
                        reason = "нет форм"
                        continue
                    homonym = f" {article['homonym']}" if article.get("homonym") else ""
                    outp.write(paradigm.to_slf(f"{source} {article['title']}{homonym} {{{{{template}}}}}"))
                    counts[source] += 1
                if reason:
                    failures.write(f"{source}\t{article['title']}\t{template}\t{reason}\n")
                    counts[source + " failures"] += 1
    for name, count in counts.items():
        print(f"{name}: {count}")


if __name__ == "__main__":
    main()
