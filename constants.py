from typing import Dict, List

# TODO Move to config yaml
WRITE_PATHS: Dict[str, str] = {
    "template": "tmp/templates.jsonl",
    "article": "tmp/articles.jsonl",
    "template_redirect": "tmp/template_redirects.jsonl"
}

PROCESSED_DIR = "tmp/processed/"

# Russian pages from the dump, see cache_dump.py
CACHE_PATH = "tmp/cache/pages.jsonl"

SEGMENT_SEPARATOR: str = "|"

SPECIAL_TEMPLATES: List[str] = [
    "сущ-ru"
]

BROKEN_ARTICLES: List[str] = [
    "ханьжа"
]

INVARIABLE_POS: List[str] = [
    "adv",
    "ger"
]

IGNORE_POS: List[str] = [
    "intrj",
    "conj",
    "fam"
]

POS_MAP: Dict[str, str] = {
    "сущ": "noun",
    # "падежи": "noun",
    "прил": "adj",
    "гл": "verb",
    "прич": "participle",
    "деепр": "ger",
    "adv": "adv",
    "interj": "intrj",
    "part": "particle",
    "мест": "padj",  # possessive
    "predic": "pred",
    "conj": "conj",
    "числ": "num",
    "prep": "prep",
    # not really parts of speech, it's just for consistency's sake
    "Фам": "fam",
    "собств.": "prop",  # proper name
    "abbrev": "abbr",
    "onomatop": "ono",
    "intro": "par",  # parenthesis
    "не так": "broken",  # ungrammatical, ignore this
}

CASE_MAP: Dict[str, str] = {
    "Им.": "nom",
    "Р.": "gen",
    "Д.": "dat",
    "В.": "acc",
    "Тв.": "ins",
    "Пр.": "prp"
}

CASE_PRONOUN_MAP: Dict[str, str] = {
    "Кто/что? (ед)": "nom-sg",
    "Кто/что? (мн)": "nom-pl",
    "Нет кого/чего? (ед)": "gen-sg",
    "Нет кого/чего? (мн)": "gen-pl",
    "Кому/чему? (ед)": "dat-sg",
    "Кому/чему? (мн)": "gat-pl",
    "Кого/что? (ед)": "acc-sg",
    "Кого/что? (мн)": "acc-pl",
    "Кем/чем? (ед)": "ins-sg",
    "Кем/чем? (мн)": "ins-pl",
    "О ком/чём? (ед)": "prp-sg",
    "О ком/чём? (мн)": "prp-pl"
}

NUMBER_MAP: Dict[str, str] = {
    "ед. ч.": "sg",
    "мн. ч.": "pl"
}

GENDER_MAP: Dict[str, str] = {
    "муж. р.": "m",
    "ср. р.": "n",
    "жен. р.": "f"
}

VERB_FORM_MAP = {
    "Я": "sfp-present",
    "Я (прош.)": "sg-fp-past",
    "Мы": "pl-fp-present",
    "Мы (прош.)": "pl-fp-past",
    "Ты": "sg-sp-present",
    "Ты (прош.)": "sg-sp-past",
    "Ты (повел.)": "sg-sp-imperative",
    "Вы": "pl-sp-present",
    "Вы (прош.)": "pl-sp-past",
    "Вы (повел.)": "pl-sp-imperative",
    "Он/она/оно": "sg-tp-mfn-present",
    "Он/она/оно (прош.)": "sg-tp-mfn-past",
    "Они": "pl-tp-present",
    "Они (прош.)": "pl-tp-past",
    "ПричНаст": "participle-present",
    "ПричПрош": "participle-past",
    "ДеепрНаст": "ger-present",
    "ДеепрПрош": "ger-past",
    "ПричСтрад": "ger-present-passive",
    "ПричСтрадПрош": "get-past-passive",
    "Будущее": "future"

}

VERB_TP = [
    "sg-tp-m-past",
    "sg-tp-f-past",
    "sg-tp-n-past"
]

# full forms of adjectives as named in the table template `прил`
ADJ_FULL_FORMS: List[str] = [
    f"{case}-{number}" for case in ("nom", "gen", "dat", "ins", "prp")
    for number in ("sg-m", "sg-n", "sg-f", "pl")
] + ["acc-sg-m-a", "acc-sg-m-n", "acc-sg-n", "acc-sg-f", "acc-pl-a", "acc-pl-n"]

ADJ_SHORT_FORMS: List[str] = ["srt-sg-m", "srt-sg-n", "srt-sg-f", "srt-pl"]

# template parameters controlling which adjective forms are shown
ADJ_FLAGS: List[str] = ["краткая", "nom", "non", "nof", "nol", "anim", "суфф"]

# the parameter of the header template `прил ru` with the comparative forms
ADJ_COMPARATIVE: str = "степень"

# the table template all adjective inflection templates resolve to
ADJ_TABLE_TEMPLATE: str = "прил"

# the table template all verb inflection templates resolve to
VERB_TABLE_TEMPLATE: str = "Гл-блок"

# cells of `Гл-блок` that are not word forms and are not needed to build them
VERB_IGNORED_CELLS: List[str] = ["hide-text", "слоги", "дореф", "соотв", "соотв-мн", "коммент", "шаблон-кат"]

# header templates of the parts of speech without inflection
INVARIABLE_TEMPLATES: Dict[str, str] = {
    "adv ru": "adv",
    "predic ru": "pred",
    "part ru": "particle",
    "prep ru": "prep",
    "conj ru": "conj",
    "interj ru": "intrj",
    "interj-ru": "intrj",
    "intro ru": "par",
    "onomatop ru": "ono",
}

# inflection templates of pronouns and numerals end with the adjective table `прил` or with a case table
PRONOUN_NUMERAL_PREFIXES = ("мест", "числ", "Мс", "падежи")
SURNAME_PREFIXES = ("Фам", "фам")
# particles of indefinite pronouns: `который-то`, `какой-нибудь`
PARTICLES = ("то", "либо", "нибудь", "нито", "таки")

# the role of an abbreviation (`роль=существительного мужского рода единственного числа`): key, value
ABBREVIATION_ROLES = [
    ("существительн", "abbr_pos", "noun"), ("прилагательн", "abbr_pos", "adj"), ("наречи", "abbr_pos", "adv"),
    ("глагол", "abbr_pos", "verb"), ("предлог", "abbr_pos", "prep"), ("союз", "abbr_pos", "conj"),
    ("частиц", "abbr_pos", "particle"), ("междомет", "abbr_pos", "intrj"), ("вводн", "abbr_pos", "par"),
    ("мужского", "gender", "m"), ("женского", "gender", "f"), ("среднего", "gender", "n"),
    ("единственного", "number", "sg"), ("множественного", "number", "pl"),
]
CASES: List[str] = ["nom", "gen", "dat", "acc", "ins", "prp"]
# case tables: their cells and the names of the forms
CASE_TABLES: Dict[str, Dict[str, str]] = {
    "падежи ru 1": {case: case for case in CASES},
    # the animate and the inanimate accusative: `трёх`, `три`
    "падежи ru 1 одуш": {**{case: case for case in CASES if case != "acc"}, "acc-a": "acc-a", "acc-n": "acc-n"},
    "падежи": {f"{case}-{number}": f"{case}-{number}" for case in CASES for number in ("sg", "pl")},
    "падежи ru m n f pl": {**{f"{case}-{gender}": f"{case}-sg-{gender}" for case in CASES for gender in "mnf"},
                           **{f"{case}-pl": f"{case}-pl" for case in CASES}},
    # surnames: `им-м`, `им-ж`, `им-мн`
    "Фам-блок": {f"{case_ru}-{gender_ru}": f"{case}-{gender}"
                 for case_ru, case in zip(("им", "род", "дат", "вин", "тв", "пр"), CASES)
                 for gender_ru, gender in (("м", "sg-m"), ("ж", "sg-f"), ("мн", "pl"))},
    # the plural of each gender: `им-м-мн`, `им-ж-мн`
    "Фам-блок-мж": {f"{case_ru}-{gender_ru}-{number_ru}": f"{case}-sg-{gender}" if number_ru == "ед" else f"{case}-pl"
                    for case_ru, case in zip(("им", "род", "дат", "вин", "тв", "пр"), CASES)
                    for gender_ru, gender in (("м", "m"), ("ж", "f")) for number_ru in ("ед", "мн")},
}

ARTICLE_NAMESPACE: str = "0"
TEMPLATE_NAMESPACE: str = "10"

USER_AGENT: Dict[str, str] = {
    'User-agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) '
                  'Chrome/74.0.3729.169 Safari/537.36'}
