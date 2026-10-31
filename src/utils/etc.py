import json
import re
from typing import NoReturn, Union, List
from unicodedata import normalize

from constants import POS_MAP

LATIN_HOMOGLYPHS = str.maketrans("aeopcxyAEOPCXY", "аеорсхуАЕОРСХУ")


def clean_string(seg: str) -> str:
    """
    Removes all symbols not part of the Russian alphabet from a given string and
    converts all chars into their composed unicode form.
    :param seg: a string to remove unnecessary symbols from
    :return: a cleaned string
    """
    # remove optional elements
    seg: str = seg.strip("\n").strip()
    seg = re.sub(r"\(.*?\)", "", seg)
    # decompose string to filter out stress marks
    seg = normalize("NFD", seg)
    # stressed vowels are sometimes typed with Latin letters, e.g. `ó` instead of `о́`
    seg = seg.translate(LATIN_HOMOGLYPHS)
    ru_letters = ({chr(ind) for ind in range(ord("а"), ord("я") + 1)}
                  | {chr(ind) for ind in range(ord("А"), ord("Я") + 1)}
                  | {"-"}
                  | {"\u0308", "\u0306"})  # this is diacritical marks for letters `й` and `ё`
    seg = "".join(c for c in seg if c in ru_letters)
    # convert back to composed form
    return normalize("NFC", seg).strip("-")


def empty_file(path: str) -> NoReturn:
    """
    Given a path creates an empty file
    :param path: file path
    """
    open(path, "w").close()


def basic_filter(article: dict) -> bool:
    return not (article["is_proper"] or article["is_obscene"])


def determine_pos(page: dict, *, page_type: str = "article") -> Union[str, None]:
    if page_type not in ("article", "template"):
        raise ValueError
    if page_type == "article":
        template_name: str = page["morpho"].get("template")
    else:
        template_name: str = page["title"]
    for key in POS_MAP.keys():
        if key in template_name:
            return POS_MAP[key]


def replace_redirect(article: dict, redirects: dict) -> dict:
    template_name: str = article["morpho"].get("template")
    if template_name in redirects:
        new_template = redirects[template_name]
        article["morpho"]["template"] = new_template
    return article


def read_redirects(path: str) -> dict:
    result: dict = {}
    with open(path) as file:
        for line in file:
            cur_redir = json.loads(line)
            result[cur_redir["title"]] = cur_redir["redirect_title"]
    return result


def basic_json_read(path: str) -> List[dict]:
    result = []
    with open(path) as file:
        for line in file:
            cur_elem = json.loads(line)
            result.append(cur_elem)
    return result


def is_usable_template(template: dict):
    pos = determine_pos(template, page_type="template")
    if not template["template"]:
        return False
    else:
        if pos == "noun" and "nom-sg" in template["template"]:
            return True
        # elif pos == "verb" and "Я" in template["template"]:
        #     return True
        # TODO extend for other parts of speech
        else:
            return False
