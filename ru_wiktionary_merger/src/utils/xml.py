import re

from lxml.etree import Element

from config import TAGS
from constants import ARTICLE_NAMESPACE, TEMPLATE_NAMESPACE


def is_article_title_ru(title: str) -> bool:
    """
    Checks whether article title a (possibly) Russian word
    :param title: article title as a string
    :return: a boolean value
    """
    # one-letter words are Russian too: `я`, `в`, `и`
    pattern = re.compile(r"^(?:[А-Яа-яЁё\-]+[^\-]|[А-Яа-яЁё])$")
    return bool(re.match(pattern, title))


def is_template(page: Element) -> bool:
    """
    Checks whether a given etree Element is a template page.
    :param page: an XML element
    :return: a boolean value
    """
    # template pages have their own namespace
    return page.find(TAGS["namespace"]).text == TEMPLATE_NAMESPACE


def is_template_title_ru(title: str) -> bool:
    """
    Checks whether a given template title refers to the Russian language.
    :param title: a template title as a string
    :return: a boolean value
    """
    pattern = re.compile(r"^Шаблон:.*ru.*$")
    return (bool(re.match(pattern, title))
            and "User" not in title)


def is_redirect(page: Element) -> bool:
    """
    Checks whether a given XML element is a redirect page.
    :param page: an XML element
    :return: a boolean value
    """
    if page.find(TAGS["redirect"]) is not None:
        return True
    else:
        return False


def is_article(page: Element) -> bool:
    """
    Checks whether a given XML element is an article.
    :param page: an XML element
    :return: a boolean value
    """
    # articles have their own namespace
    return page.find(TAGS["namespace"]).text == ARTICLE_NAMESPACE


def is_article_title_proper(title: str) -> bool:
    """
    Checks whether a given article title is a proper noun, adjective etc.
    :param title: an article title as a string
    :return: a boolean value
    """
    return title.istitle()


def is_element_page(elem: Element) -> bool:
    """
    Checks whether a given XML element is a page.
    :param elem: an XML element
    :return: a boolean value
    """
    return "page" in elem.tag


def get_page_id(page: Element) -> int:
    """
    Gets the page id from the element's tags.
    :param page: an XML element
    :return: an integer identifier
    """
    return int(page.find(TAGS["id"]).text)


def get_raw_wiki(page: Element) -> str:
    """
    Gets the actual contents of a page elements.
    :param page: an XML element
    :return: element's contents as a string
    """
    return page.find(TAGS["text"]).text


def get_page_title(page: Element) -> str:
    """
    Gets the page's title from the element's tags.
    :param page: an XML element
    :return: the title of the page as a string
    """
    return page.find(TAGS["title"]).text


def get_redirect_title(page: Element) -> str:
    """
    Gets the title of the page that the given element
    redirects to. This function assumes that the redirect
    check was already done previously.
    :param page: an XML element
    :return: the title of the page that the current page redirects to
    """
    return page.find(TAGS["redirect"]).get("title")
