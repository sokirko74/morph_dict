from string import digits

from constants import SEGMENT_SEPARATOR
from src.utils.etc import clean_string


def clean_segments(segments: dict, *, sep: str = SEGMENT_SEPARATOR) -> str:
    """
    Transforms a raw segments dictionary into a list of cleaned segments.
    :param segments: a raw segments dictionary
    :param sep: a string used to separate joined segments
    :return: a string where morpheme boundaries are represented using `|` symbol
    """
    return sep.join(clean_string(value)
                    for key, value in sorted(segments.items())
                    if key in digits and value)


def remove_duplicate_seps(segmented_string: str, *, sep: str = SEGMENT_SEPARATOR):
    sep_flag = False
    filtered = []
    for c in segmented_string:
        if c != sep:
            filtered.append(c)
            if sep_flag:
                sep_flag = False
        else:
            if not sep_flag:
                sep_flag = True
                filtered.append(c)
    return "".join(filtered)
