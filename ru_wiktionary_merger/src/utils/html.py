from typing import Dict, Union

import pandas as pd
import requests
from bs4 import Tag, BeautifulSoup as Soup

from constants import NUMBER_MAP, CASE_MAP, USER_AGENT
from src.utils.etc import clean_string


def prepare_html_table(table: Tag) -> str:
    for br in table.find_all("br"):
        br.replace_with("\n")
    table = str(table).replace(u"\xa0", " ")
    return table


def parse_noun_table(table: Tag) -> Dict[str, str]:
    table = prepare_html_table(table)
    table = pd.read_html(table, flavor="bs4")[0]
    table = table.rename(columns={**{"падеж": "case"}, **NUMBER_MAP}).set_index("case").rename(index=CASE_MAP)
    return table_to_dict(table)


def request_text(title: str) -> str:
    url = get_url(title)
    html = requests.get(url, headers=USER_AGENT)
    return html.text


def table_to_dict(df: pd.DataFrame) -> Dict[str, str]:
    result = {}
    for col in df.columns:
        for index, item in df[col].items():
            if " " in item:
                item = item.split(" ")
                item = [clean_string(option) for option in item]
            else:
                item = clean_string(item)
            result[f"{index}-{col}"] = item
    return result


def get_morpho_table(html: str) -> Union[pd.DataFrame, None]:
    # this is just the first meaning
    # homonymy is not considered at the moment
    table = pd.read_html(html, attrs={'class': 'morfotable ru'})[0]
    return table


def get_url(title: str) -> str:
    """
    Generate an api url for a given title.
    :param title: a word to look up
    :return: an url
    """
    # using the api in this format is essentially equivalent to simply requesting a regular page
    # also, using the `parse` api is considerably slower
    # keeping it in case of discovering some useful application for it
    # return f"https://ru.wiktionary.org/w/api.php?action=parse&page={title}&prop=text&formatversion=2&format=json"

    # so, ultimately it's much easier to parse the regular article
    return f"https://ru.wiktionary.org/wiki/{title}"


def process_title(title: str) -> Union[Dict[str, str], None]:
    html = request_text(title)
    parsed = Soup(html, features="lxml").find_all(class_="morfotable ru")
    if len(parsed) > 0:
        table = parse_noun_table(parsed[0])
        return table
