"""
Extracts morphological data from the Russian pages cached by `cache_dump.py`
(the cache is built first if it is missing).
Articles and templates can be re-extracted separately:
    python extract_from_dump.py --templates
"""
from multiprocessing import Pool, cpu_count
from timeit import default_timer as timer
from typing import List
import argparse
import json
import pathlib
import re

import wikitextparser as wtp

from config import DUMP_PATH
from constants import WRITE_PATHS, BROKEN_ARTICLES, CACHE_PATH
from cache_dump import cache_dump
from src.utils.wiki import find_ru_section, parse_ru_section, clean_template_name, parse_template_page, \
    remove_no_include


HEADER = re.compile(r"^(=+)[^=].*?(=+)\s*$")
MORPHO_SECTION = "Морфологические и синтаксические свойства"


def keep_morpho_sections(text: str) -> str:
    """
    Parsing the whole section is slow, while only the morphological sections are needed.
    Keeps them and the headers of levels 1 and 2 (homonyms are level 2 sections).
    """
    result = []
    morpho_level = None
    for line in text.splitlines():
        header = HEADER.match(line)
        if header:
            level = min(len(header.group(1)), len(header.group(2)))
            if morpho_level and level <= morpho_level:
                morpho_level = None
            if MORPHO_SECTION in line:
                morpho_level = level
            if level <= 2 or morpho_level:
                result.append(line)
        elif morpho_level:
            result.append(line)
    return "\n".join(result)


def process_article(page: dict) -> List[dict]:
    """
    Parses the Russian section of an article; homonyms give several results.
    """
    if page["title"] in BROKEN_ARTICLES:
        return []
    wiki_data = find_ru_section(wtp.parse(keep_morpho_sections(page["text"])))
    if not wiki_data:
        return []
    result = []
    for parsed_wiki_data in parse_ru_section(wiki_data):
        parsed_wiki_data["id"] = page["id"]
        parsed_wiki_data["title"] = page["title"]
        parsed_wiki_data["type"] = "article"
        parsed_wiki_data["is_proper"] = page["is_proper"]
        parsed_wiki_data["is_obscene"] = page["is_obscene"]
        parsed_wiki_data["has_homonyms"] = page["has_homonyms"]
        result.append(parsed_wiki_data)
    return result


def process_template(page: dict) -> List[dict]:
    """
    Template pages contain the inflection tables, template redirects are aliases for their names.
    """
    title = clean_template_name(page["title"])
    if page["type"] == "template_redirect":
        # here we don't actually need any wiki data from the page body
        return [{"id": page["id"],
                 "title": title,
                 "redirect_title": clean_template_name(page["redirect_title"]),
                 "type": "template_redirect"}]
    text = remove_no_include(page["text"])
    wiki_data = wtp.parse(text)
    parsed_wiki_data = parse_template_page(wiki_data) if wiki_data else None
    # some tables are written in the wiki table markup (`{| ... |}`) instead of calling a table template
    if "{|" in text:
        parsed_wiki_data = parsed_wiki_data or {"template": {}}
        parsed_wiki_data["wikitable"] = text
    if not parsed_wiki_data:
        return []
    parsed_wiki_data["id"] = page["id"]
    parsed_wiki_data["title"] = title
    parsed_wiki_data["type"] = "template"
    return [parsed_wiki_data]


def process_page(page: dict) -> List[dict]:
    if page["type"] == "article":
        return process_article(page)
    return process_template(page)


def read_cache(path: str, types: set):
    with open(path) as file:
        for line in file:
            page = json.loads(line)
            if page["type"] in types:
                yield page


def parse_args():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("--articles", action="store_true", help="extract articles only")
    arg_parser.add_argument("--templates", action="store_true", help="extract templates and their redirects only")
    arg_parser.add_argument("--rebuild-cache", action="store_true", help="read the dump again")
    arg_parser.add_argument("--processes", type=int, default=cpu_count())
    return arg_parser.parse_args()


if __name__ == "__main__":
    start = timer()
    args = parse_args()
    if args.rebuild_cache or not pathlib.Path(CACHE_PATH).exists():
        cache_dump(DUMP_PATH, pathlib.Path(CACHE_PATH))

    stages = []
    if args.articles or not args.templates:
        stages.append("article")
    if args.templates or not args.articles:
        stages += ["template", "template_redirect"]

    # only the files of the selected stages are rewritten
    outputs = {stage: open(WRITE_PATHS[stage], "w") for stage in stages}
    with Pool(args.processes) as pool:
        results = pool.imap_unordered(process_page, read_cache(CACHE_PATH, set(stages)), chunksize=200)
        for index, records in enumerate(results):
            if index % 10000 == 0:
                print("\r" + f"Processed {index} pages...", end="")
            for record in records:
                outputs[record["type"]].write(json.dumps(record, ensure_ascii=False) + "\n")
    for file in outputs.values():
        file.close()

    print("\n" + f"Time elapsed: {timer() - start:.1f}")
