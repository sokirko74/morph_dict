"""
Verb paradigms: the cells of the table template `Гл-блок` are mapped to grammatical names,
participles are declined here, since the table gives only their initial forms.
"""
import re
from typing import Dict, List, Optional

from constants import SEGMENT_SEPARATOR

# cells of `Гл-блок`; the non-past is the present for imperfective verbs and the future for perfective ones
VERB_PERSON_CELLS: Dict[str, str] = {
    "Я": "npst-sg-1",
    "Ты": "npst-sg-2",
    "Он/она/оно": "npst-sg-3",
    "Мы": "npst-pl-1",
    "Вы": "npst-pl-2",
    "Они": "npst-pl-3",
    "Ты (повел.)": "imp-sg-2",
    "Вы (повел.)": "imp-pl-2",
    "imper-1p": "imp-pl-1",
    "Инфинитив": "inf",
}

# past forms by gender and number, see `past_form_name`
VERB_PAST_CELLS: Dict[str, Optional[str]] = {
    "Он (прош.)": "pst-sg-m",
    "Она (прош.)": "pst-sg-f",
    "Оно (прош.)": "pst-sg-n",
    "Они (прош.)": "pst-pl",
    # older templates give all genders in one cell: `вня́л<br />вняла́<br />вня́ло`
    "Он/она/оно (прош.)": None,
    "Я (прош.)": None,
    "Ты (прош.)": None,
    "Мы (прош.)": "pst-pl",
    "Вы (прош.)": "pst-pl",
}

VERB_GERUND_CELLS: Dict[str, str] = {
    "ДеепрНаст": "ger-prs",
    "ДеепрПрош": "ger-pst",
}

VERB_PARTICIPLE_CELLS: Dict[str, str] = {
    "ПричНаст": "ptcp-prs-act",
    "ПричПрош": "ptcp-pst-act",
    "ПричСтрад": "ptcp-prs-pass",
    "ПричСтрадПрош": "ptcp-pst-pass",
}

# full forms: the names are the same as for adjectives, see ADJ_FULL_FORMS
HARD_ENDINGS: Dict[str, List[str]] = {
    "nom-sg-m": ["ый"], "gen-sg-m": ["ого"], "dat-sg-m": ["ому"], "ins-sg-m": ["ым"], "prp-sg-m": ["ом"],
    "nom-sg-n": ["ое"], "gen-sg-n": ["ого"], "dat-sg-n": ["ому"], "ins-sg-n": ["ым"], "prp-sg-n": ["ом"],
    "nom-sg-f": ["ая"], "gen-sg-f": ["ой"], "dat-sg-f": ["ой"], "ins-sg-f": ["ой", "ою"], "prp-sg-f": ["ой"],
    "nom-pl": ["ые"], "gen-pl": ["ых"], "dat-pl": ["ым"], "ins-pl": ["ыми"], "prp-pl": ["ых"],
    "acc-sg-m-a": ["ого"], "acc-sg-m-n": ["ый"], "acc-sg-n": ["ое"], "acc-sg-f": ["ую"],
    "acc-pl-a": ["ых"], "acc-pl-n": ["ые"],
}
# after a sibilant: `-щий`, `-вший`
SOFT_ENDINGS: Dict[str, List[str]] = {
    "nom-sg-m": ["ий"], "gen-sg-m": ["его"], "dat-sg-m": ["ему"], "ins-sg-m": ["им"], "prp-sg-m": ["ем"],
    "nom-sg-n": ["ее"], "gen-sg-n": ["его"], "dat-sg-n": ["ему"], "ins-sg-n": ["им"], "prp-sg-n": ["ем"],
    "nom-sg-f": ["ая"], "gen-sg-f": ["ей"], "dat-sg-f": ["ей"], "ins-sg-f": ["ей", "ею"], "prp-sg-f": ["ей"],
    "nom-pl": ["ие"], "gen-pl": ["их"], "dat-pl": ["им"], "ins-pl": ["ими"], "prp-pl": ["их"],
    "acc-sg-m-a": ["его"], "acc-sg-m-n": ["ий"], "acc-sg-n": ["ее"], "acc-sg-f": ["ую"],
    "acc-pl-a": ["их"], "acc-pl-n": ["ие"],
}
SHORT_ENDINGS: Dict[str, str] = {"srt-sg-m": "", "srt-sg-f": "а", "srt-sg-n": "о", "srt-pl": "ы"}


def normalize_cell(expr: str) -> str:
    """
    Variants in a cell are separated not only by `<br>` and commas, as in adjective tables:
    `чтя́т / чту́т`, `колы́шусь // колыха́юсь`, `до́хнул (дох)`; links are replaced with their text.
    """
    expr = re.sub(r"<br\s*/?>", ",", expr)
    expr = re.sub(r"<[^<>]*>", "", expr)
    expr = re.sub(r"\[\[(?:[^\]|]*\|)?([^\]]*)\]\]", r"\1", expr)
    return re.sub(r"[()/]", ",", expr)


def past_form_name(form: str) -> str:
    """
    The gender and number of a past form by its ending: `нёс`, `несла́`, `несло́`, `несли́`.
    """
    plain = form.replace(SEGMENT_SEPARATOR, "")
    if plain.endswith(("ся", "сь")):
        plain = plain[:-2]
    return {"а": "pst-sg-f", "о": "pst-sg-n", "и": "pst-pl"}.get(plain[-1:], "pst-sg-m")


def gerund_form_name(form: str) -> str:
    plain = form.replace(SEGMENT_SEPARATOR, "")
    return "ger-pst" if re.search(r"(в|вши|вшись|ши|шись)$", plain) else "ger-prs"


def participle_form_name(form: str) -> Optional[str]:
    """
    The kind of a participle by its suffix, for the cell `Прич` of old articles.
    """
    plain = form.replace(SEGMENT_SEPARATOR, "")
    if plain.endswith("ся"):
        plain = plain[:-2]
    if re.search(r"[щ]ий$", plain):
        return "ptcp-prs-act"
    if re.search(r"ший$", plain):
        return "ptcp-pst-act"
    if re.search(r"мый$", plain):
        return "ptcp-prs-pass"
    if re.search(r"(нный|тый)$", plain):
        return "ptcp-pst-pass"
    return None


# the suffixes of participles, a form of another shape is probably an error in the article: `пережида́йнный`
PARTICIPLE_SHAPES: Dict[str, re.Pattern] = {
    "ptcp-prs-act": re.compile(r"[уюая]щий(ся)?$"),
    "ptcp-pst-act": re.compile(r"ший(ся)?$"),
    "ptcp-prs-pass": re.compile(r"[еоиё]мый$"),
    "ptcp-pst-pass": re.compile(r"([аяеёиоуыю]нн|т)ый$"),
}


def is_participle_shape(form: str, name: str) -> bool:
    return bool(PARTICIPLE_SHAPES[name].search(form.replace(SEGMENT_SEPARATOR, "")))


def decline_participle(form: str, passive: bool) -> Optional[Dict[str, List[str]]]:
    """
    Declines a participle given in the nominative masculine: `прочи́танный`, `пры́гающий`, `умыва́ющийся`.
    Passive participles get short forms: `прочитан`, `прочтён`, `прочтена`.
    :return: forms by name (the names of ADJ_FULL_FORMS and ADJ_SHORT_FORMS) or None if it is not a participle
    """
    reflexive = form.endswith("ся")
    base = form[:-2] if reflexive else form
    plain = base.replace(SEGMENT_SEPARATOR, "")
    if plain.endswith("ый"):
        endings = HARD_ENDINGS
    elif plain.endswith("ий") and plain[-3:-2] in ("ш", "щ"):
        endings = SOFT_ENDINGS
    else:
        return None
    stem = base[:-2]
    if not stem.endswith(SEGMENT_SEPARATOR):
        stem += SEGMENT_SEPARATOR
    suffix = "ся" if reflexive else ""
    result = {name: [stem + ending + suffix for ending in variants] for name, variants in endings.items()}
    if passive and not reflexive:
        short = stem.rstrip(SEGMENT_SEPARATOR)
        # `прочитанн` -> `прочитан`
        if short.replace(SEGMENT_SEPARATOR, "").endswith("нн"):
            short = short[:-1]
        # the stressed suffix `-ённ` keeps `ё` only in the masculine form: `прочтён`, `прочтена́`
        others = re.sub(r"ё(\|?н)$", r"е\1", short)
        for name, ending in SHORT_ENDINGS.items():
            result[name] = [(others if ending else short) + ending]
    return result
