from typing import List, Union, Tuple

from constants import SEGMENT_SEPARATOR
from src.utils.etc import clean_string


def longest_common_prefix(forms: List[str]) -> str:
    forms = [form.replace("ё", "е") for form in forms]

    if not forms:
        return ""
    if len(forms) == 1:
        return forms[0]

    prefix = []
    shortest = min(forms)
    longest = max(forms)
    for i in range(len(shortest)):
        short_char, long_char = shortest[i], longest[i]
        if short_char == long_char:
            prefix.append(short_char)
        else:
            break
    return "".join(prefix)


def process_alternated_stem(stem, longest_prefix_len, sep_positions, sep):
    stem_left_part = restore_seps(stem[:longest_prefix_len], sep_positions, sep)
    stem_right_part = stem[longest_prefix_len:]
    return stem_left_part.strip(sep) + sep + stem_right_part


def segment_stem(stem: str, segmented_lemma: str, *, sep: str = SEGMENT_SEPARATOR) -> Union[str, None]:
    """
    Given a segmented lemma and a whole stem attempt to segment the stem.
    :param stem: the stem to segment
    :param segmented_lemma: the lemma with marked morpheme boundaries
    :param sep: the symbol used to mark the morpheme boundaries in the lemma
    :return: the stem with morpheme boundaries marked with provided separator symbol, it may also
    be impossible to carry out such segmentation using provided data, in which case None is returned
    """
    # not considering hyphens by convention
    stem = clean_string(stem).replace("-", "")
    whole_lemma, sep_positions = process_seps(segmented_lemma, sep)
    # in this case we already have segmented stem; a stem of the same length may differ: `аватарок`, `аватарка`
    if stem == whole_lemma:
        return segmented_lemma
    # this means that there are no stem alternations
    if whole_lemma.startswith(stem):
        # if a given lemma is not segmented, then the stem also isn't
        if sep not in segmented_lemma:
            return segmented_lemma[:len(stem)]
        # otherwise, restore segmentation
        else:
            return restore_seps(stem, sep_positions, sep)
    # this means that there are stem alternations
    else:
        # first we find the longest common part of the stem and the lemma
        longest_prefix = longest_common_prefix([whole_lemma, stem])
        # ...
        if not longest_prefix:
            return None
        # if there's a non-empty prefix, then we can segment it
        # then append the rest of the stem to it
        else:
            return process_alternated_stem(stem, len(longest_prefix), sep_positions, sep)


def restore_seps(stem: str, sep_positions: List[int], sep: str) -> str:
    """
    Given a stem and a list of indices of separators put the separator symbols
    in the appropriate positions.
    :param stem: an unsegmented stem
    :param sep_positions: a list of indices of separators in ascending order
    :param sep: the separator symbol
    :return: the provided stem with morpheme boundaries marked using the separator symbol
    """
    # if the list is empty, then there's nothing more to do
    if not sep_positions:
        return stem
    # the list must reversed, so we can pop elements in the correct order
    sep_positions = sep_positions[::-1]
    # the first separator position
    cur_pos = sep_positions.pop()
    result = []
    for i, c in enumerate(stem):
        result.append(c)
        # if there should be a separator after this character
        if i == cur_pos:
            result.append(sep)
            # if there are still separators remaining
            if sep_positions:
                cur_pos = sep_positions.pop()
            else:
                cur_pos = None
    return "".join(result).strip(sep)


def process_seps(segmented_lemma: str, sep: str) -> Tuple[str, List[int]]:
    """
    Given a segmented lemma extract the positions of separators in it.
    :param segmented_lemma: a lemma with morpheme boundaries marked by the separator symbol
    :param sep: the separator symbol
    :return: the given lemma without separators and a list of separator indices
    """
    whole_lemma = []
    sep_positions = []
    index = -1
    for c in segmented_lemma:
        if c == sep:
            sep_positions.append(index)
        else:
            index += 1
            whole_lemma.append(c)
    return "".join(whole_lemma), sep_positions


def some_stems_filled(stems: dict) -> bool:
    """
    Returns True if stems dictionary is not empty,
    and some of its values are not empty.
    :param stems: a dictionary containing word stem or stems
    :return: a boolean value
    """
    if stems and any(stems.values()):
        return True
    else:
        return False


def all_stems_filled(stems: dict) -> bool:
    """
    Returns True if stems dictionary is not empty
    and all its values are not empty.
    :param stems: a dictionary containing word stem or stems
    :return: a boolean value
    """
    if stems and all(stems.values()):
        # assert "основа" in stems.keys()
        return True
    else:
        return False
