import re
from typing import Dict, List, Optional

from src.utils.etc import clean_string

ADJ_ENDINGS = ("ийся", "ыйся", "ой", "ый", "ий")
SIBILANTS = "жшчщц"
VOWELS = "аеёиоуыэюя"


def split_adjective_lemma(lemma: str) -> Optional[tuple]:
    """
    Splits a (possibly stressed) adjective lemma into the stem and the ending: `безымя́нный` -> `безымя́нн`, `ый`.
    """
    plain = clean_string(lemma)
    for ending in ADJ_ENDINGS:
        if plain.endswith(ending):
            # the stem keeps the stress marks of the lemma
            cut = len(lemma)
            removed = ""
            while len(removed) < len(ending):
                cut -= 1
                removed = clean_string(lemma[cut:])
            return lemma[:cut], ending
    return None


def short_masculine_stem(stem: str, index: str) -> str:
    """
    The stem of the short masculine form with a fleeting vowel (Zaliznyak's `*`):
    `красн` -> `красен`, `крепк` -> `крепок`, `горьк` -> `горек`, `стойк` -> `стоек`;
    with the marks (1), (2) or a stressed ending `-нн` loses one `н`: `связанн` -> `связан`;
    a stressed fleeting vowel (short forms stress scheme `b`) is `ё`: `умн` -> `умён`, `смешн` -> `смешон`.
    """
    plain = clean_string(stem)
    if len(plain) < 2 or plain[-1] in VOWELS:
        return plain
    # the stress scheme of short forms: the part after `/` or the main one
    scheme = re.search(r"[abc]'*(?:/([abc]))?", index)
    stressed = bool(scheme) and (scheme.group(1) or scheme.group(0)[0]) == "b"
    if plain.endswith("нн") and (re.search(r"\([12]\)|[①②]", index) or stressed):
        return plain[:-1]
    before, last = plain[:-1], plain[-1]
    if before[-1] in "йь":
        before, vowel = before[:-1], "е"
    elif last == "к" and before[-1] not in SIBILANTS:
        vowel = "о"
    else:
        vowel = "е"
    # a stem with `ё` keeps the stress: `чёрн` -> `чёрен`
    if stressed and vowel == "е" and "ё" not in before:
        vowel = "о" if before[-1:] in tuple(SIBILANTS) else "ё"
    return before + vowel + last


def derive_stems(lemma: str, index: str) -> Optional[Dict[str, str]]:
    """
    Stems for the template `прил ru <index>` from the lemma, as they would be written in an article.
    """
    split = split_adjective_lemma(lemma)
    if not split:
        return None
    stem = split[0]
    plain = clean_string(stem)
    stems = {"основа": stem}
    if "*" in index:
        stems["основа1"] = short_masculine_stem(stem, index)
    else:
        # with a moving stress (`1a/c'`) the short forms lose `ё`: `твёрдый` -> `тверда́`
        stems["основа1"] = plain.replace("ё", "е") if "/" in index else plain
    # the stem for stressed endings of short forms: `чёрный` -> `черна́`, `решённый (2)` -> `решена́`
    if plain.endswith("нн") and re.search(r"\(2\)|②", index):
        stems["основа2"] = plain[:-1]
    else:
        stems["основа2"] = plain.replace("ё", "е")
    return stems


def index_candidates(index: str) -> List[str]:
    """
    Zaliznyak indices used with `прил-ru` that have no template of their own are reduced
    to the nearest existing ones: `1b(X)` -> `1bX`, `3a✕~` -> `3aX~`, `1a~` -> `1a`, `1*a①` -> `1*a(1)`.
    """
    # remarks after a comma: `п 1*a/b, ё`
    index = index.split(",")[0].strip()
    candidates = [index]
    # typos: Cyrillic `Х` instead of `X`, `1a*` instead of `1*a`, spaces, references `§7`
    typos = re.sub(r"[Ххx]", "X", index).replace("''", "\"")
    typos = re.sub(r"§\s*\d+|\s+[а-яё]+$", "", typos)
    typos = re.sub(r"^(п\s+)?(\d)([abc])\*", r"\1\2*\3", typos)
    typos = re.sub(r"(?<=\d)\s+(?=[abc*])", "", typos)
    if typos != index:
        candidates.append(typos)
        index = typos

    def add(value):
        if value not in candidates:
            candidates.append(value)

    current = re.sub(r"^п\s+", "", index)
    add(current)
    for old, new in (("✕", "X"), ("(X)", "X"), ("⊠", "X"), ("×", "X"), ("—", "X"),
                     ("[(1)]", "(1)"), ("[①]", "(1)"), ("①", "(1)"), ("[(2)]", "(2)"), ("②", "(2)")):
        current = current.replace(old, new)
        add(current)
    without_tilde = current.replace("~", "")
    add(without_tilde)
    add(re.sub(r"\(\d\)", "", without_tilde))
    add(re.sub(r"\(\d\)|X", "", without_tilde))
    return candidates
