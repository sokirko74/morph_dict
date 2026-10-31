import re
from typing import Dict, List, Optional

# the innermost template parameter: `{{{name}}}` or `{{{name|default}}}`
PARAM_PATTERN = re.compile(r"\{\{\{([^{}|]*)(?:\|([^{}]*))?\}\}\}")
COMMENT_PATTERN = re.compile(r"<!--.*?-->", re.DOTALL)
TOKEN_PATTERN = re.compile(r"\x00(\d+)\x00")


def remove_comments(text: str) -> str:
    return COMMENT_PATTERN.sub("", text)


def expand_params(text: str, params: Dict[str, str]) -> str:
    """
    Substitutes template parameters the way MediaWiki does it: `{{{name|default}}}`
    becomes the value of `name` if it was passed (even an empty one), otherwise `default`.
    Parameters that were not passed and have no default are left intact.
    Inserted values are not expanded again, so they may contain parameters of the calling template.
    :param text: wiki text of a template argument
    :param params: arguments passed to the template
    :return: the text with parameters substituted
    """
    text = remove_comments(text)
    inserted = []

    def replace(match):
        name, default = match.group(1).strip(), match.group(2)
        if name in params:
            value = remove_comments(params[name])
        elif default is not None:
            return default
        else:
            value = match.group(0)
        inserted.append(value)
        return f"\x00{len(inserted) - 1}\x00"

    while PARAM_PATTERN.search(text):
        text = PARAM_PATTERN.sub(replace, text)
    while TOKEN_PATTERN.search(text):
        text = TOKEN_PATTERN.sub(lambda m: inserted[int(m.group(1))], text)
    return text


# the innermost template or parser function call, but not a parameter `{{{...}}}`
CALL_PATTERN = re.compile(r"(?<!\{)\{\{([^{}]*)\}\}")
# marks a value that must not be used, e.g. an incorrect form `{{incorrect|...}}` or a dash `{{-}}`
ABSENT = "\x03"
# marks a form with a stylistic label (archaic, colloquial), such forms are not used either
LABEL = "\x08"
# marks an empty argument, so that `#if` can see that it is empty
EMPTY = "\x05"
# marks a difficult (but existing) form `{{difficult|...}}`
DIFFICULT = "\x07"
# a stem inserted into a form, it is replaced with the segmented stem at the end: `\x01основа\x04пры́га\x02`
STEM_MARKER = re.compile(r"\x01([^\x02\x04]*)\x04([^\x02]*)\x02")


def stem_marker(name: str, stem: str) -> str:
    return f"\x01{name}\x04{stem}\x02"


def plain_text(text: str) -> str:
    """
    The text with stems instead of their markers, for string functions.
    """
    return STEM_MARKER.sub(r"\2", text).replace(EMPTY, "").replace(DIFFICULT, "")


def string_function(name: str, parts: List[str]) -> Optional[str]:
    """
    `{{Str crop|пры́га|1}}` drops the last characters, `{{Str rightc|пры́га|1}}` keeps them.
    """
    if len(parts) < 2 or not parts[1].strip().isdigit():
        return None
    text, count = plain_text(parts[0]), int(parts[1])
    if name == "Str crop":
        return text[:-count] if count else text
    return text[-count:] if count else ""


def is_empty(text: str) -> bool:
    return not text.replace(EMPTY, "").strip()


def split_arguments(text: str) -> List[str]:
    """
    Splits call arguments by `|`, except the ones inside links: `[[новее|нове́е]]`.
    """
    parts, current, depth, i = [], [], 0, 0
    while i < len(text):
        if text.startswith("[[", i):
            depth += 1
            current.append("[[")
            i += 2
        elif text.startswith("]]", i) and depth:
            depth -= 1
            current.append("]]")
            i += 2
        else:
            if text[i] == "|" and not depth:
                parts.append("".join(current))
                current = []
            else:
                current.append(text[i])
            i += 1
    parts.append("".join(current))
    return parts


def evaluate_call(body: str) -> Optional[str]:
    """
    Evaluates the few templates and parser functions used in inflection templates.
    :param body: the text between `{{` and `}}`, without nested calls
    :return: the result or None if the call is not supported
    """
    name, _, rest = body.partition(":") if body.startswith("#") else body.partition("|")
    name = name.strip()
    parts = split_arguments(rest)
    if name == "#if":
        cond, then, other = (parts + ["", ""])[:3]
        return then if not is_empty(cond) else other
    if name == "#ifeq":
        left, right, then, other = (parts + ["", "", ""])[:4]
        return then if left.replace(EMPTY, "").strip() == right.replace(EMPTY, "").strip() else other
    if name == "#switch":
        value, cases = parts[0].replace(EMPTY, "").strip(), parts[1:]
        default, matched = "", False
        for index, case in enumerate(cases):
            key, has_result, result = case.partition("=")
            if not has_result:
                # the last case without `=` is the default, the others fall through to the next result
                if index == len(cases) - 1:
                    return case
                matched = matched or key.strip() == value
            elif matched or key.strip() == value:
                return result
            elif key.strip() == "#default":
                default = result
        return default
    if name in ("incorrect", "-"):
        return ABSENT
    # a stylistic label of the next form: `{{устар.|-}}&nbsp;ея́`, `{{прост.|-}} ей`
    if name.endswith(".") and parts[:1] == ["-"]:
        return LABEL
    if name in ("Str crop", "Str rightc"):
        return string_function(name, parts)
    # a link and a superscript mark (`{{^|*}}`) of a form, a category
    if name == "aslinks":
        return parts[0]
    if name in ("^", "Категория"):
        return ""
    # a special form
    if name in ("особ.ф.", "nobr"):
        return rest
    if name == "difficult":
        return rest + DIFFICULT if rest.strip() else rest
    return None


def evaluate_wiki(text: str, params: Dict[str, str]) -> str:
    """
    Substitutes parameters and evaluates supported calls (see `evaluate_call`) until nothing changes.
    Unsupported calls and parameters that were not passed are left intact.
    """
    unsupported = []

    def replace(match):
        result = evaluate_call(match.group(1))
        if result is None:
            unsupported.append(match.group(0))
            return f"\x06{len(unsupported) - 1}\x06"
        return result

    def hide_param(match):
        unsupported.append(match.group(0))
        return f"\x06{len(unsupported) - 1}\x06"

    while True:
        new_text = CALL_PATTERN.sub(replace, expand_params(text, params))
        if new_text == text:
            # parameters that were not passed may block the calls around them, e.g.
            # `{{#if:|{{{степень}}}|...}}`: MediaWiki evaluates only the chosen branch
            new_text = PARAM_PATTERN.sub(hide_param, text)
            if new_text == text:
                break
        text = new_text
    while "\x06" in text:
        text = re.sub(r"\x06(\d+)\x06", lambda m: unsupported[int(m.group(1))], text)
    return text
