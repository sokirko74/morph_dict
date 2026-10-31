from typing import Union, Dict, List
import re

from wikitextparser import Section, Template
from wikitextparser import WikiText

from constants import CASE_PRONOUN_MAP
from src.utils.etc import clean_string

# TODO Consider homonymic forms, i.e. multiple sections for one word


def find_ru_section(wiki_text: WikiText) -> Union[Section, None]:
    """
    Attempts to find Russian language section in a given
    parsed wiki text.
    :param wiki_text: parsed wiki text
    :return: None or found section
    """
    sections = wiki_text.get_sections()
    sections = [sec for sec in sections if sec.level == 1 and '{{-ru-}}' in sec.title]
    if len(sections) == 1:
        return sections[0]
    else:
        return None


def clean_template_name(template: str) -> str:
    """
    Removes unnecessary symbols from template name
    to achieve uniformity in naming.
    :param template: raw template name as a string
    :return: cleaned template name
    """
    template = template.replace("Шаблон:", "").strip("\n").strip()

    # comment = re.compile(r"<!--.*?-->")
    # template = comment.sub("", template)

    whitespace = re.compile(r"\s+")
    template = whitespace.sub(" ", template)
    # invisible marks of the text direction: `мест ru 1a-в\u200e`
    template = template.replace("\u200e", "").replace("\u200f", "")
    return template.strip()


def parse_morpho(temp: Template) -> Dict[str, Union[str, dict]]:
    """
    Extracts template name and stems for a given word from a
    parsed wikitext morphology template.
    NB: returned stems generally contain diacritical marks and
    are not normalized
    :param temp: an instance of wikitextparser Template class
    :return: a dictionary containing template name and stems
    """
    alternate = {}
    forms = {}
    temp_name = clean_template_name(temp.name)
    # some templates use some arcane script
    # to generate actual word forms, thus it's not
    # worth the effort trying to implement that
    # functionality here
    # other templates generally have arguments with the
    # word `основа` in them, so if no such arguments were
    # found, then these words will be ignored further on
    if temp_name == "падежи":
        forms = parse_raw_cases(temp)
    args = template_to_dict(temp)

    stems = {}
    info = {}
    for key, value in args.items():
        if "основа" in key:
            stems[key] = value
        else:
            info[key] = value

    # this information might still be useful
    if not stems and len(args) > 1 and "1" in args.keys() and "2" in args.keys():
        alternate["lemma"] = args["1"]
        alternate["index"] = args["2"]
    return {
        "template": temp_name,
        "stems": stems,
        "alternate": alternate,
        "forms": forms,
        "info": info
    }


def parse_raw_cases(temp: Template) -> dict:
    result = {}
    args = temp.arguments
    try:
        for arg in args:
            key = arg.name.strip("\n").strip()
            value = clean_string(arg.value)
            result[CASE_PRONOUN_MAP[key]] = value
        return result
    except KeyError:
        return result


def find_segments(templates: List[Template]) -> Union[Template, None]:
    """
    Attempts to find the morpheme division template in a list
    of parsed templates.
    :param templates: a list of templates extracted from parsed wiki text
    :return: either None or the relevant template
    """
    segments = [temp for temp in templates if "морфо-ru" in temp.name]
    if len(segments) == 1:
        return segments[0]
    else:
        return None


def parse_segments(temp: Template) -> Union[Dict[str, str], None]:
    """
    Transforms given morpheme division template into a dictionary.
    Return None if the template has no arguments.
    :param temp: morpheme division template
    :return: either None or a dictionary containing template argument
    names and their values
    """
    assert "морфо-ru" in temp.name
    args = temp.arguments
    # some templates are empty and have no arguments
    # these are useless
    if not len(args) > 0:
        return None
    segments = {segment.name: segment.value for segment in args}
    # some templates have arguments but no values for them
    # these are also useless
    if not any(segments.values()):
        return None
    return segments


def homonym_label(section: Section, index: int) -> str:
    """
    Homonyms are level 2 sections titled like `{{заголовок|I}}`; returns `I` or the ordinal number.
    """
    title = str(section.title or "")
    match = re.search(r"\{\{заголовок\|([^|}]*)", title)
    return match.group(1).strip() if match else str(index)


def parse_morpho_section(morpho_section: Section) -> Union[dict, None]:
    """
    Extracts the morphological template and morpheme segmentation from
    a `Морфологические и синтаксические свойства` section.
    :param morpho_section: a wiki text section
    :return: a dictionary with relevant information or None
    """
    if not morpho_section.templates:
        return None
    templates = morpho_section.templates
    # the morphological template should be the first one
    # in the relevant section; it cannot be identified by name
    main = templates[0]
    # a header without a table may be followed by the table: `{{мест ru}} {{падежи ru 1|nom=никто́|...}}`
    if clean_template_name(main.name) in BARE_HEADERS:
        tables = [t for t in templates[1:]
                  if t.nesting_level == 1 and is_table_template(clean_template_name(t.name))]
        main = tables[0] if tables else main
    morpho = parse_morpho(main)
    # the template with morphemes can be found by name;
    # articles without segmentation are kept with empty segments
    segments = find_segments(templates)
    segments = parse_segments(segments) if segments else None
    if not segments or len(segments) == 1 and segments.get("1") == "":
        segments = {}
    # the part of speech of a table given in the article (`{{падежи ru 1|nom=он|...}}`) is in the next template
    morpho["section_templates"] = [clean_template_name(t.name) for t in templates if t.nesting_level == 1]
    return {
        "morpho": morpho,
        "segments": segments
    }


def parse_ru_section(wiki_section: Section) -> List[dict]:
    """
    Processes the entire Russian language section of an article.
    Homonyms have their own morphological sections, each of them gives a separate result
    with a `homonym` label (`I`, `II`, ...).
    :param wiki_section: a wiki text section in Russian language
    :return: a list of dictionaries with morphological information and morpheme segmentation
    """
    homonyms = [sec for sec in wiki_section.get_sections(level=2)]
    morpho_sections = [sec for sec in wiki_section.get_sections()
                       if 'Морфологические и синтаксические свойства' in str(sec.title)]
    result = []
    for index, morpho_section in enumerate(morpho_sections, start=1):
        parsed = parse_morpho_section(morpho_section)
        if not parsed:
            continue
        if len(morpho_sections) > 1:
            # find the level 2 section containing this morphological section
            owners = [sec for sec in homonyms if sec.span[0] <= morpho_section.span[0] < sec.span[1]]
            parsed["homonym"] = homonym_label(owners[0], index) if owners else str(index)
        result.append(parsed)
    return result


# the table templates the inflection templates of adjectives, verbs, pronouns, numerals and surnames call
TABLE_TEMPLATES = ("прил", "Гл-блок", "падежи", "Фам-блок")
# headers of articles that have no inflection template
BARE_HEADERS = ("мест ru", "числ ru", "Фам ru", "Фам")


def is_table_template(name: str) -> bool:
    # `падежи ru 1`, `Гл-блок2наст`, but not the header `прил ru`
    return name == "прил" or name.startswith(TABLE_TEMPLATES[1:])


def parse_template_page(wiki_text: WikiText) -> Union[dict, None]:
    """
    Convert the entire template page to a single dictionary.
    :param wiki_text: parsed wiki text of a template page
    :return: a dict or None
    """
    templates = wiki_text.templates
    # in general, that shouldn't happen
    if not templates:
        return None
    # for our purposes we expect a template page
    # containing just one template
    # template pages not satisfying this condition
    # are likely irrelevant
    # inflection tables of adjectives and verbs are an exception: their arguments contain nested templates
    # (`{{incorrect|...}}`, `{{-}}`), and base ones (e.g. `прил ru 1a`) call the table
    # template `прил` and then the header template `прил ru`
    header = None
    if len(templates) > 1:
        top_level = [t for t in templates if t.nesting_level == 1]
        if len(top_level) > 1:
            # the header gets the parameters not shown in the table, e.g. the comparative `степень`
            headers = [t for t in top_level if clean_template_name(t.name) == "прил ru"]
            header = headers[0] if len(headers) == 1 else None
            top_level = [t for t in top_level if is_table_template(clean_template_name(t.name))]
        if len(top_level) != 1 or not clean_template_name(top_level[0].name).startswith(TABLE_TEMPLATES):
            return None
        templates = top_level
    # templates attribute always return a list
    # so we need to take out the only element
    template = templates[0]
    result = {"template": template_to_dict(template),
              "call": clean_template_name(template.name)}
    if header:
        result["header"] = template_to_dict(header)
    return result


def remove_no_include(raw_template_page: str) -> str:
    """
    wikitextparser can't parse templates with `<noinclude>`
    tags in them. This function removes these tags and their
    contents, and the tags `<includeonly>` without their contents.
    :param raw_template_page: unprocessed text body of a template page
    :return: the same text body with tags removed
    """
    # the documentation is often a `<noinclude>` block of several lines
    pattern = re.compile(r"<noinclude>.*?</noinclude>", re.DOTALL)
    lines = re.sub(pattern, "", raw_template_page).split("\n")
    output = []
    for line in lines:
        # the contents of `<includeonly>` are used when the template is called, only the tags go away:
        # `{{{основа1<includeonly>|{{{основа}}}</includeonly>}}}`
        line = re.sub(r"</?includeonly>", "", line)
        if line:
            output.append(line)
    return "\n".join(output)


def template_to_dict(template: Template) -> dict:
    """
    A simple function to convert a parsed template into a dictionary.
    :param template: parsed wiki template
    :return: a dictionary with the contents of the template
    """
    output = {}
    for arg in template.arguments:
        output[arg.name.strip("\n").strip()] = arg.value.strip("\n").strip()
    return output


def is_obscene(raw_wiki: str) -> bool:
    """
    Check whether a given page is marked as offensive.
    :param raw_wiki: the entire page text as a string
    :return: a boolean value
    """
    if "{off}" in raw_wiki or "{offensive}" in raw_wiki:
        return True
    else:
        return False


def has_ru_homonyms(raw_string: str) -> bool:
    """
    Checks whether a given page has Russian homonyms.
    :param raw_string: the entire page text as a string
    :return: a boolean value
    """
    return "{{Омонимы|ru" in raw_string
