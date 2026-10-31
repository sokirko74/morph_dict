"""
Surnames with the header `{{Фам ru|основа=Хво́рост}}` and no table: the template `Фам ru ...`
is chosen by the ending of the surname, the way surnames are declined in Russian.
"""
from typing import Optional, Tuple

from src.utils.etc import clean_string

SIBILANTS = "жшчщ"
VELARS = "кгх"
VOWELS = "аеёиоуыэюя"


def cut_ending(word: str, count: int) -> str:
    """
    Removes the last `count` letters, the stress marks of the rest are kept: `Ивано́ва`, 1 -> `Ивано́в`.
    """
    cut = len(word)
    while count and cut:
        cut -= 1
        if clean_string(word[cut]):
            count -= 1
    return word[:cut]


def guess_surname_template(surname: str) -> Optional[Tuple[str, str]]:
    """
    :param surname: the masculine form, possibly with stress marks
    :return: the template and the stem for it, or None if the ending is not clear (`-й` after a consonant)
    """
    plain = clean_string(surname).lower()
    if len(plain) < 2:
        return None
    last, before = plain[-1], plain[-2]
    if plain.endswith(("ов", "ев", "ёв", "ин", "ын")):
        return "Фам ru ин ов", surname
    if plain.endswith(("ых", "их")) or last in "оеёэиыую":
        return "Фам ru неизм", surname
    if plain.endswith("ой"):
        return "Фам ru прил 1b", cut_ending(surname, 2)
    if plain.endswith("ый"):
        return "Фам ru прил 1a", cut_ending(surname, 2)
    if plain.endswith("ий"):
        third = plain[-3:-2]
        if third and third in VELARS:
            return "Фам ru прил 3a", cut_ending(surname, 2)
        if third and third in SIBILANTS:
            return "Фам ru прил 4a", cut_ending(surname, 2)
        # Ukrainian surnames: `Шарий`, `Шария`, the feminine one is not declined
        return "Фам ru m 6a", cut_ending(surname, 1)
    if last == "а":
        if before in VELARS:
            return "Фам ru f 3a", cut_ending(surname, 1)
        if before in SIBILANTS:
            return "Фам ru f 4a", cut_ending(surname, 1)
        if before == "ц":
            return "Фам ru f 5a", cut_ending(surname, 1)
        return "Фам ru f 1a", cut_ending(surname, 1)
    if last == "я":
        return ("Фам ru f 7a" if before == "и" else "Фам ru f 2a"), cut_ending(surname, 1)
    if last == "й":
        return ("Фам ru m 6a", cut_ending(surname, 1)) if before in VOWELS else None
    if last == "ь":
        return "Фам ru m 2a", cut_ending(surname, 1)
    if last in SIBILANTS:
        return "Фам ru m 4a", surname
    if last == "ц":
        return "Фам ru m 5a", surname
    if last in VELARS:
        return "Фам ru m 3a", surname
    return "Фам ru m 1a", surname
