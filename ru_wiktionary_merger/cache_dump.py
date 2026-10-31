"""
The first, slow step: reads the compressed dump once and stores only the pages we need
in `tmp/cache/pages.jsonl`. For articles only the Russian section is kept.
`extract_from_dump.py` then works with this file, so it can be re-run quickly.
"""
from bz2 import BZ2File
from shutil import which
from timeit import default_timer as timer
import json
import pathlib
import re
import subprocess

from lxml.etree import iterparse

from config import DUMP_PATH
from constants import CACHE_PATH, ARTICLE_NAMESPACE, TEMPLATE_NAMESPACE
from src.utils.wiki import is_obscene, has_ru_homonyms
from src.utils.xml import is_article_title_ru, is_template_title_ru, is_article_title_proper

# a level 1 header: `= {{-ru-}} =`, but not `== ... ==`
LEVEL1_HEADER = re.compile(r"^=(?!=).*[^=]=(?!=)\s*$", re.MULTILINE)

# templates without "ru" in the title that the Russian inflection templates call
EXTRA_TEMPLATES = {"Шаблон:" + name for name in (
    "прил", "Гл-блок", "Гл-блок2наст", "спряжения", "Str crop", "Str rightc", "Str left", "Str right", "aslinks", "^",
    "incorrect", "-", "особ.ф.", "difficult")}
# inflection templates of numerals, pronouns and surnames, not all of them have "ru" in the title: `числ-5`, `Мс-п6*a`
EXTRA_TEMPLATE_PREFIXES = tuple("Шаблон:" + name for name in ("числ", "Мс", "падежи", "мест", "Фам", "фам"))


def cut_ru_section(text: str):
    """
    Returns the Russian language section of an article, or None if there is not exactly one.
    """
    headers = list(LEVEL1_HEADER.finditer(text))
    ru = [i for i, header in enumerate(headers) if "{{-ru-}}" in header.group(0)]
    if len(ru) != 1:
        return None
    start = headers[ru[0]].start()
    end = headers[ru[0] + 1].start() if ru[0] + 1 < len(headers) else len(text)
    return text[start:end]


def open_dump(path: str):
    # lbzip2 decompresses in parallel, it is several times faster than the bz2 module
    if which("lbzip2"):
        process = subprocess.Popen(["lbzip2", "-dc", path], stdout=subprocess.PIPE, bufsize=1 << 20)
        return process.stdout
    return BZ2File(path)


def cache_dump(dump_path: str, cache_path: pathlib.Path):
    cache_path.parent.mkdir(parents=True, exist_ok=True)
    counts = {"article": 0, "template": 0, "template_redirect": 0}
    with open_dump(dump_path) as dump, open(cache_path, "w") as out:
        context = iterparse(dump, events=("end",), tag="{*}page")
        for index, (_, page) in enumerate(context):
            if index % 100000 == 0:
                print("\r" + f"Processed {index} pages...", end="")
            ns_uri = page.tag[:-len("page")]
            namespace = page.findtext(ns_uri + "ns")
            title = page.findtext(ns_uri + "title")
            redirect = page.find(ns_uri + "redirect")
            text = page.findtext(f"{ns_uri}revision/{ns_uri}text") or ""
            record = None
            if namespace == ARTICLE_NAMESPACE and redirect is None and is_article_title_ru(title):
                ru_section = cut_ru_section(text)
                if ru_section:
                    record = {"type": "article", "title": title, "text": ru_section,
                              "is_proper": is_article_title_proper(title),
                              "is_obscene": is_obscene(text), "has_homonyms": has_ru_homonyms(text)}
            elif namespace == TEMPLATE_NAMESPACE and (is_template_title_ru(title) or title in EXTRA_TEMPLATES
                                                    or title.startswith(EXTRA_TEMPLATE_PREFIXES)):
                if redirect is None:
                    record = {"type": "template", "title": title, "text": text}
                else:
                    record = {"type": "template_redirect", "title": title, "redirect_title": redirect.get("title")}
            if record:
                record["id"] = int(page.findtext(ns_uri + "id"))
                out.write(json.dumps(record, ensure_ascii=False) + "\n")
                counts[record["type"]] += 1
            # free memory: the page and everything parsed before it
            page.clear()
            while page.getprevious() is not None:
                del page.getparent()[0]
    print("\n" + f"Cached: {counts}")


if __name__ == "__main__":
    start = timer()
    cache_dump(DUMP_PATH, pathlib.Path(CACHE_PATH))
    print(f"Time elapsed: {timer() - start:.1f}")
