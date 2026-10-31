import json
import re
from typing import Tuple, Optional
import pathlib

from constants import WRITE_PATHS, SEGMENT_SEPARATOR, PROCESSED_DIR, ADJ_FULL_FORMS, ADJ_SHORT_FORMS, ADJ_FLAGS, \
    ADJ_TABLE_TEMPLATE, ADJ_COMPARATIVE, \
    VERB_TABLE_TEMPLATE, VERB_IGNORED_CELLS, PRONOUN_NUMERAL_PREFIXES, CASE_TABLES, \
    SURNAME_PREFIXES
from src.expansion import expand_params
from src.segmentation import remove_duplicate_seps
from src.utils.etc import determine_pos, read_redirects, is_usable_template, basic_json_read, clean_string

# question marks at the end of templates are probably safe to remove in articles
# however, it is not clear what is their purpose exactly


def separate_form_template(form_template: str) -> Optional[Tuple[str, str]]:
    ending = re.findall(r"[^\}]+$", form_template)
    if ending:
        ending = ending[0]
    else:
        ending = ""
    stem_name = re.findall(r"основа-?\d?", form_template)
    if not stem_name:
        return None
    else:
        return stem_name[0], clean_string(ending)


def process_template(template_page: dict):
    pos = determine_pos(template_page, page_type="template")
    result = {}
    for key, value in template_page["template"].items():
        if "основа" in value:
            if "<br>" in value:
                value = value.split("<br>")
            elif "<br />" in value:
                value = value.split("<br />")
            elif " " in value:
                value = value.split()
            else:
                value = [value]

            if "if" not in value and "основа" in value[0]:
                if pos == "noun":
                    result[key] = separate_form_template(value[0])
    return result


def process_ending_noun(ending, *, sep: str = SEGMENT_SEPARATOR):
    if ending == "еняток":
        return sep + "ен" + sep + "ят" + sep + "ок"
    if ending == "енят":
        return sep + "ен" + sep + "ят"
    if ending == "енятки":
        return sep + "ен" + sep + "ят" + sep + "к" + sep + "и"
    if ending == "еняткам":
        return sep + "ен" + sep + "ят" + sep + "к" + sep + "ам"
    if ending == "енятками":
        return sep + "ен" + sep + "ят" + sep + "к" + sep + "ам" + "и"
    if ending == "енятках":
        return sep + "ен" + sep + "ят" + sep + "к" + sep + "ах"
    if ending == "енята":
        return sep + "ен" + sep + "ят" + sep + "а"
    if ending == "енятам":
        return sep + "ен" + sep + "ят" + sep + "ам"
    if ending == "енятами":
        return sep + "ен" + sep + "ят" + sep + "ам" + sep + "и"
    if ending == "енятах":
        return sep + "ен" + sep + "ят" + sep + "ах"
    if ending == "яьми":
        return sep + "ями"
    if ending == "еви":
        return sep + "ев" + "и"

    if len(ending) > 2 and ending[:2] == "ён":
        return sep + ending[:2] + sep + ending[2:]
    if len(ending) == 1:
        if ending in "ьъ":
            return ending
        else:
            return sep + ending
    elif len(ending) > 2 and ending[:2] == "иц":
        return "и" + "ц" + sep + ending[2:]
    else:
        if ending[0] in "ьи":
            return ending[0] + sep + ending[1:]
        elif ending[0] in "кцн":
            return sep + ending[0] + sep + ending[1:]
        else:
            return sep + ending


def process_ending(ending: str, pos: str, *, sep: str = SEGMENT_SEPARATOR) -> str:
    if not ending:
        return ending
    if ending[-1] == '̈е':  # broken unicode
        ending[-1] = "ё"

    if pos == "noun":
        ending = process_ending_noun(ending, sep=sep)

    if len(ending) > 2 and ending[-2:] in ("ся", "сь"):
        ending = ending[:-2] + sep + ending[-2:]

    ending = remove_duplicate_seps(ending, sep=sep)

    return ending


def resolve_table_template(title: str, table_name: str, templates: dict, redirects: dict,
                           seen: tuple = ()) -> Optional[dict]:
    """
    Inflection templates call each other, e.g. `прил ru 1*a` -> `прил ru 1a` -> `прил`, where
    only the last one is the table with the endings. Follows the chain and substitutes the
    arguments passed on each step, so that the result is expressed in terms of article parameters.
    :param title: a template name
    :param table_name: the table template, `прил` or `Гл-блок`
    :param templates: all extracted templates by name
    :param redirects: template redirects
    :param seen: the templates already visited, to stop on cycles
    :return: arguments of the table template or None if the chain is broken
    """
    title = redirects.get(title, title)
    template = templates.get(title)
    if template is None or title in seen or "call" not in template:
        return None
    call = redirects.get(template["call"], template["call"])
    if call == table_name:
        table = dict(template["template"])
        if "степень" in template.get("header", {}):
            table["степень"] = template["header"]["степень"]
        return table
    base = resolve_table_template(call, table_name, templates, redirects, seen + (title,))
    if base is None:
        return None
    return {key: expand_params(value, template["template"]) for key, value in base.items()}


redirects = read_redirects(WRITE_PATHS["template_redirect"])

full_noun_templates = []
broken_noun_templates = []

noun_templates = basic_json_read(WRITE_PATHS["template"])

destination = pathlib.Path(PROCESSED_DIR)
destination.mkdir(parents=True, exist_ok=True)

with open(destination / "templates.jsonl", "w") as file:
    for template in noun_templates:
        if determine_pos(template, page_type="template") == "noun":
            template_id = template["id"]
            template_title = template["title"]
            if is_usable_template(template):
                temp = process_template(template)
                final_temp = {}
                for key, value in temp.items():
                    final_temp[key] = {value[0]: process_ending(value[1], "noun")}
                if final_temp:
                    file.write(json.dumps({"id": template_id,
                                           "title": template_title,
                                           "template": final_temp}, ensure_ascii=False) + "\n")


    # adjective templates contain no endings themselves, they are resolved to the table template
    templates_by_title = {template["title"]: template for template in noun_templates}
    for template in noun_templates:
        if determine_pos(template, page_type="template") == "adj":
            table = resolve_table_template(template["title"], ADJ_TABLE_TEMPLATE, templates_by_title, redirects)
            if table and "nom-sg-m" in table:
                keep = ADJ_FULL_FORMS + ADJ_SHORT_FORMS + ADJ_FLAGS + [ADJ_COMPARATIVE]
                file.write(json.dumps({"id": template["id"],
                                       "title": template["title"],
                                       "pos": "adj",
                                       "template": {key: table[key] for key in keep if key in table}},
                                      ensure_ascii=False) + "\n")

    # verb templates pass all cells to the table `Гл-блок`, the cells are evaluated for each article
    for template in noun_templates:
        if determine_pos(template, page_type="template") == "verb":
            table = resolve_table_template(template["title"], VERB_TABLE_TEMPLATE, templates_by_title, redirects)
            # perfective templates have no `Инфинитив`, it is the title of the article
            if table and ("Инфинитив" in table or "Он (прош.)" in table or "Он/она/оно (прош.)" in table):
                file.write(json.dumps({"id": template["id"],
                                       "title": template["title"],
                                       "pos": "verb",
                                       "template": {key: value for key, value in table.items()
                                                    if key not in VERB_IGNORED_CELLS}},
                                      ensure_ascii=False) + "\n")

    # pronoun, numeral and surname templates: `мест ru п1a` -> `прил`, `мест ru кто` -> `падежи ru 1`,
    # `Фам ru ин ов` -> `Фам-блок`; some of them are tables in the wiki markup: `числ ru 2`
    for template in noun_templates:
        # the tables themselves are filled in articles: `{{падежи ru 1|nom=он|...}}`
        if template["title"].startswith(PRONOUN_NUMERAL_PREFIXES + SURNAME_PREFIXES) \
                and template["title"] not in CASE_TABLES:
            table = resolve_table_template(template["title"], ADJ_TABLE_TEMPLATE, templates_by_title, redirects)
            if table and "nom-sg-m" in table:
                keep = ADJ_FULL_FORMS + ADJ_SHORT_FORMS + ADJ_FLAGS + [ADJ_COMPARATIVE]
                result = {"pos": "adj", "template": {key: table[key] for key in keep if key in table}}
            else:
                for table_name, cells in CASE_TABLES.items():
                    table = resolve_table_template(template["title"], table_name, templates_by_title, redirects)
                    if table:
                        result = {"pos": "cases", "table": table_name,
                                  "template": {key: table[key] for key in cells if key in table}}
                        break
                else:
                    if "wikitable" not in template:
                        continue
                    result = {"pos": "wikitable", "text": template["wikitable"]}
            file.write(json.dumps({"id": template["id"], "title": template["title"], **result},
                                  ensure_ascii=False) + "\n")
