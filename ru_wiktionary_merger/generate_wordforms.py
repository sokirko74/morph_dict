from src.expansion import evaluate_wiki, expand_params, remove_comments, plain_text, is_empty, ABSENT, EMPTY, \
    DIFFICULT, LABEL, STEM_MARKER, stem_marker
from src.adjectives import derive_stems, index_candidates
from src.wikitable import table_cells
from src.surnames import guess_surname_template
from src.verbs import VERB_PERSON_CELLS, normalize_cell, past_form_name, decline_participle, \
    is_participle_shape
from src.generation import segment_stem
from src.segmentation import remove_duplicate_seps, clean_segments
from src.utils.etc import determine_pos, basic_json_read, read_redirects, replace_redirect, basic_filter, \
    clean_string
from constants import WRITE_PATHS, SEGMENT_SEPARATOR, ADJ_FULL_FORMS, ADJ_SHORT_FORMS, ADJ_COMPARATIVE, \
    VERB_TABLE_TEMPLATE, INVARIABLE_TEMPLATES, PRONOUN_NUMERAL_PREFIXES, CASE_TABLES, SURNAME_PREFIXES, PARTICLES, \
    ABBREVIATION_ROLES

from typing import List, Dict, Optional, Tuple
import html
import json
import argparse
import re
from unicodedata import normalize

VARIANT_SEPARATOR = re.compile(r"<br\s*/?>|,|//")
LATIN_LETTER = re.compile(r"[a-zA-Z]")
PAGE_NAME = re.compile(r"\{\{\s*(?:subst:)?PAGENAME\s*\}\}")
# `й` after a consonant is impossible, it comes from a wrong template in the article: `ощети́нйся`
IMPOSSIBLE_LETTERS = re.compile(r"[бвгджзклмнпрстфхцчшщ]й")
WIKI_LINK = re.compile(r"\[\[([^\]|]*\|)?([^\]]*)\]\]")
# values of `степень` meaning there is no comparative, see the header template `прил ru`
NO_COMPARATIVE = {"", "~", "-", "−", "—", "затрудн", "трудно", "затруднительно"}


def map_templates(temps: List[dict]) -> Dict[str, dict]:
    result = {}
    for t in temps:
        result[t["title"]] = t
    return result


def clean_markers(text: str) -> str:
    return STEM_MARKER.sub(lambda m: "{{{" + m.group(1) + "}}}", text).replace(EMPTY, "")


def clean_morpheme(morpheme: str) -> str:
    # `clean_string` strips hyphens, but in compounds they belong to morphemes: `квантово-|физическ`
    cleaned = clean_string(morpheme)
    if cleaned and morpheme.strip().endswith("-"):
        cleaned += "-"
    return cleaned


def clean_stem_markup(stem: str) -> str:
    # a stem copied with the template markup: `основа={{{основа|Брессон}}}`, it would be substituted forever
    return expand_params(remove_comments(stem), {}).strip()


def normalize_word(word: str) -> str:
    return clean_string(word).replace("ё", "е").lower()


class TWiktionary:
    def __init__(self, args):
        self.args = args
        self.articles = None
        self.templates = None
        self.redirects = None
        self.articles_by_title = None
        self.failures = []

    def read_db(self):
        self.templates = map_templates(basic_json_read("tmp/processed/templates.jsonl"))
        articles = basic_json_read(self.args.input_articles)
        self.redirects = read_redirects(WRITE_PATHS["template_redirect"])
        self.articles = [replace_redirect(self.normalize_article(article), self.redirects) for article in articles]

    def process_article(self, article, outp):
        article_template = article["morpho"]["template"]
        if article_template in self.templates.keys():
            stems: dict = article["processed_stems"]
            if stems:
                template: dict = self.templates[article_template]
                form_templates = template["template"]
                generated_forms: dict = dict()
                for form, temp in form_templates.items():
                    for name, value in stems.items():
                        if name in temp.keys():
                            # a stem that does not fit the segmentation of the lemma is taken as is: `ребёночек`
                            if value is None:
                                value = clean_string(article["morpho"]["stems"][name])
                            generated_forms[form] = remove_duplicate_seps(f"{value}{temp[name]}")
                article["generated_forms"] = generated_forms
                outp.write(json.dumps(article, ensure_ascii=False) + "\n")

    @staticmethod
    def normalize_article(article: dict) -> dict:
        """
        `основа={{PAGENAME}}` is the title of the article, `основа={{{основа|Брессон}}}` is `Брессон`,
        the template name may have a comment: `Фам ru иностр-м<!-- 1a ~ 0-->`.
        """
        morpho = article["morpho"]
        if morpho.get("template"):
            morpho["template"] = remove_comments(morpho["template"]).strip()
        for group in ("stems", "info"):
            for name, value in morpho.get(group, {}).items():
                if "PAGENAME" in value:
                    morpho[group][name] = PAGE_NAME.sub(article["title"], value)
        for name, value in morpho.get("stems", {}).items():
            if "{{{" in value:
                morpho["stems"][name] = clean_stem_markup(value)
        return article

    def add_failure(self, article, reason):
        title = article["title"]
        if "homonym" in article:
            title += f" ({article['homonym']})"
        self.failures.append((title, article["morpho"]["template"], reason))

    @staticmethod
    def restore_hyphens(segmented: str, plain: str) -> str:
        """
        `segment_stem` drops hyphens, put them back: `квантовофизическ` -> `квантово-физическ`.
        """
        result, i = [], 0
        for c in segmented:
            if c != SEGMENT_SEPARATOR:
                while i < len(plain) and plain[i] == "-":
                    # keep the hyphen at the end of the previous morpheme: `квантово-|физическ`
                    if result and result[-1] == SEGMENT_SEPARATOR:
                        result.insert(len(result) - 1, "-")
                    else:
                        result.append("-")
                    i += 1
                i += 1
            result.append(c)
        return "".join(result) + plain[i:]

    def segmented_lemma(self, article) -> str:
        lemma = clean_string(article["title"])
        # `+∅` gives an empty last segment: `как|ов|`
        segmented = clean_segments(article["segments"]).strip(SEGMENT_SEPARATOR) if article["segments"] else ""
        return segmented if segmented.replace(SEGMENT_SEPARATOR, "") == lemma else lemma

    def segment_stems(self, stems: dict, processed_stems: dict) -> Dict[str, str]:
        """
        Stems with morpheme boundaries from `морфо-ru`, or plain ones if the segmentation does not fit.
        """
        segmented_stems = {}
        for name, stem in stems.items():
            plain = clean_string(stem)
            segmented = processed_stems.get(name) or plain
            # the segmentation of `морфо-ru` may disagree with the stem, e.g. `суботн|е-` for `субботне-`
            if segmented.replace(SEGMENT_SEPARATOR, "").replace("-", "") != plain.replace("-", ""):
                segmented = plain
            segmented_stems[name] = (self.restore_hyphens(segmented, plain)
                                    if "-" in plain and "-" not in segmented else segmented)
        return segmented_stems

    @staticmethod
    def multiword_stem(stems: dict) -> Optional[str]:
        """
        A stem of several words (`не ш` for `не шёл`), the forms would not be single words.
        """
        for stem in stems.values():
            stem = WIKI_LINK.sub(r"\2", clean_stem_markup(stem))
            if " " in stem and not re.search(r"<br|[,/]", stem):
                return stem
        return None

    @staticmethod
    def stem_params(stems: dict) -> Dict[str, str]:
        """
        Stems are passed to templates as markers, to be replaced with segmented stems in the forms.
        A stem with several variants (`дурён, ду́рен`) is passed as is, like MediaWiki does it.
        """
        params = {}
        for name, stem in stems.items():
            stem = WIKI_LINK.sub(r"\2", clean_stem_markup(stem))
            if not clean_string(stem):
                params[name] = EMPTY
            elif re.search(r"<br|[,/]", re.sub(r"<(?!br)[^<>]*>", "", stem)):
                params[name] = stem
            # a stem of several words gives analytic forms: `ко̀е <span>(предлог)</span> ко́`, they are dropped
            elif len(re.sub(r"<[^<>]*>", "", stem).split()) > 1:
                params[name] = ABSENT
            else:
                params[name] = stem_marker(name, stem)
        return params

    @staticmethod
    def build_adjective_forms(expr: str, segmented_stems: dict) -> Tuple[List[str], Optional[str]]:
        """
        Turns an evaluated form expression into segmented word forms, one per variant (`твёрдой<br>твёрдою`).
        :return: the forms and the problem: unresolved parameters or calls, if any
        """
        forms = []
        problem = None
        # `&nbsp;` must not get into the form: `clean_string` would turn the Latin `p` into Cyrillic `р`
        expr = html.unescape(expr)
        # the label belongs to the next form: `{{устар.|-}} свящу́`
        expr = re.sub(LABEL + r"\s*", ABSENT, expr)
        # forms are single words, a space separates variants built from stems: `{{{основа}}}ой {{{основа}}}ою`;
        # other words with a space are analytic forms: `ни о ком`
        variants = []
        for variant in VARIANT_SEPARATOR.split(expr):
            words = variant.split()
            if len(words) > 1 and not all(STEM_MARKER.search(word) or ABSENT in word for word in words):
                problem = problem or "несколько слов: " + clean_markers(variant).strip()
                continue
            variants += words
        for variant in variants:
            if not variant.strip() or ABSENT in variant or EMPTY in variant:
                continue
            # a form without stems is fine if it was given literally in the article, e.g. `srt-sg-m=вели́к`
            if "{{" in variant:
                problem = problem or "не раскрылось: " + clean_markers(variant)
                continue
            variant = STEM_MARKER.sub(lambda m: segmented_stems[m.group(1)] + SEGMENT_SEPARATOR, variant)
            form = SEGMENT_SEPARATOR.join(clean_morpheme(part) for part in variant.split(SEGMENT_SEPARATOR))
            form = remove_duplicate_seps(form).strip(SEGMENT_SEPARATOR)
            if form and form not in forms:
                forms.append(form)
        return forms, problem

    def build_comparatives(self, expr: str, segmented_stems: dict) -> Tuple[List[str], Optional[str]]:
        """
        Interprets the comparative the way the header template `прил ru` does it.
        """
        value = expr.replace(EMPTY, "").strip()
        if value in NO_COMPARATIVE:
            return [], None
        if value == "1":
            if "основа" not in segmented_stems:
                return [], None
            marker = stem_marker("основа", "")
            expr = f"{marker}ее, {marker}ей"
        # wiki markup: `[[дешевле|деше́вле]] ''и'' [[подешевле|подеше́вле]] ''(в значении цены)''`
        expr = WIKI_LINK.sub(r"\2", expr)
        expr = re.sub(r"''.*?''|\(.*?\)", ",", expr)
        variants = [v for v in VARIANT_SEPARATOR.split(expr) if v.strip()]
        multiword = [v.strip() for v in variants if " " in clean_markers(v).strip()]
        forms, problem = self.build_adjective_forms(
            ",".join(v for v in variants if " " not in clean_markers(v).strip()), segmented_stems)
        if multiword and not problem:
            problem = "не разобрано: " + ", ".join(multiword)
        return forms, problem

    def indeclinable_adjective_forms(self, article) -> dict:
        """
        `прил ru 0`: all full forms are the lemma, there are no short forms.
        """
        return {name: [self.segmented_lemma(article)] for name in ADJ_FULL_FORMS}

    def find_lua_template(self, article) -> Tuple[Optional[str], Optional[str], Optional[str]]:
        """
        `{{прил-ru|безымя́нный|1*a}}` is rendered by a Lua module, the table is not in the dump.
        Finds the template `прил ru <index>` for it.
        :return: the lemma, the index and the template name
        """
        info = article["morpho"]["info"]
        lemma = (info.get("1") or info.get("слово") or "").strip()
        index = remove_comments(info.get("2") or info.get("индекс") or "").strip()
        # sometimes the parameters are swapped: `{{прил-ru|1*a/c'|зна́тный}}`
        if len(clean_string(index)) > 3 and re.search(r"\d", lemma):
            lemma, index = index, lemma
        lemma = lemma or article["title"]
        candidates = index_candidates(index)
        if clean_string(lemma).endswith("ся"):
            candidates = [c + "-ся" for c in candidates] + candidates
        for candidate in candidates:
            name = self.redirects.get("прил ru " + candidate, "прил ru " + candidate)
            if name in self.templates:
                return lemma, index, name
        return lemma or article["title"], index, None

    def generate_adjective_forms(self, article) -> Tuple[Optional[dict], Optional[str], List[str]]:
        # MediaWiki ignores comments in template names: `прил ru <!-- 1a/c'-->`
        article_template = remove_comments(article["morpho"]["template"]).strip()
        if article_template == "прил ru":
            return None, "не указан тип по Зализняку (шаблон «прил ru» без параметров)", []
        if article_template == "прил ru 0":
            article["indeclinable"] = True
            return self.indeclinable_adjective_forms(article), None, []
        if article_template == "прил ru сравн":
            return None, "сравнительная степень («прил ru сравн»), не склоняется", []
        params = dict(article["morpho"]["info"])
        if article_template == "прил-ru":
            lemma, index, article_template = self.find_lua_template(article)
            if not article_template:
                return None, f"Lua-модуль «прил-ru»: нет шаблона для индекса «{index}»", []
            stems = derive_stems(lemma, index)
            if not stems:
                return None, f"Lua-модуль «прил-ru»: не удалось выделить основу из «{lemma}»", []
            article["morpho"]["lua_template"] = article_template
            article["morpho"]["stems"] = stems
            segmented = clean_segments(article["segments"]) if article["segments"] else ""
            processed_stems = {name: segment_stem(stem, segmented) if segmented else None
                               for name, stem in stems.items()}
            # `-ся` is added to full forms by the table template `прил` if the template does not know it
            if clean_string(lemma).endswith("ся") and "ся" not in article_template:
                params["суфф"] = "ся"
            # `~` in the index means there is no comparative
            if "~" in index and not params.get(ADJ_COMPARATIVE, "").strip():
                params[ADJ_COMPARATIVE] = "~"
        else:
            if article_template not in self.templates:
                return None, f"шаблон «{article_template}» не найден или не сводится к таблице «прил»", []
            stems = article["morpho"]["stems"]
            processed_stems = article["processed_stems"]
        if not clean_string(stems.get("основа", "")):
            return None, "пустая основа", []
        if self.multiword_stem(stems):
            return None, f"основа из нескольких слов: «{self.multiword_stem(stems)}»", []
        segmented_stems = self.segment_stems(stems, processed_stems)
        params.update(self.stem_params(stems))
        template = {key: evaluate_wiki(value, params)
                    for key, value in self.templates[article_template]["template"].items()}
        if "lua_template" in article["morpho"]:
            # forms given explicitly in the article win over the template: `srt-sg-m=хо́лоден`
            for name in ADJ_FULL_FORMS + ADJ_SHORT_FORMS:
                if params.get(name, "").strip():
                    template[name] = evaluate_wiki(params[name], params)
        flags = {key: clean_markers(template.get(key, "")).strip() for key in ("краткая", "nom", "non", "nof", "nol")}

        # see the table template `прил`: short forms are shown if `краткая` is not empty,
        # `✕` and `⊠` mark all of them as incorrect, `?` and `−` only the masculine one
        form_names = [] if flags["nol"] else list(ADJ_FULL_FORMS)
        if flags["краткая"] and flags["краткая"] not in ("✕", "⊠"):
            form_names += ADJ_SHORT_FORMS[1:] if flags["краткая"] in ("?", "−") else ADJ_SHORT_FORMS
        for flag, gender in (("nom", "-m"), ("non", "-n"), ("nof", "-f")):
            if flags[flag]:
                form_names = [f for f in form_names if gender not in f]

        raw_suffix = template.get("суфф", "").strip()
        suffix = clean_string(raw_suffix)
        if suffix and raw_suffix.startswith("-"):
            suffix = "-" + suffix
        generated_forms = {}
        warnings = []
        for name in form_names:
            forms, problem = self.build_adjective_forms(template.get(name, ""), segmented_stems)
            if forms:
                if name in ADJ_FULL_FORMS and suffix:
                    # `прил ru 4a` adds the suffix to `-ей` itself: `{{{основа}}}ей{{{суфф|}}}<br>{{{основа}}}ею`
                    forms = [form if form.endswith(suffix) else form + suffix for form in forms]
                generated_forms[name] = forms
            if problem:
                warnings.append(f"форма {name} не построена, {problem}")
        if not generated_forms:
            return None, "формы не построились", warnings
        comparatives, problem = self.build_comparatives(template.get(ADJ_COMPARATIVE, ""), segmented_stems)
        if comparatives:
            generated_forms["comp"] = comparatives
        if problem:
            warnings.append(f"сравнительная степень, {problem}")
        # substantivized adjectives (`гончая`) and short-only ones (`должен`) have another initial form
        lemmas = [form.replace(SEGMENT_SEPARATOR, "")
                  for name in ("nom-sg-m", "nom-sg-f", "nom-sg-n", "nom-pl", "srt-sg-m")
                  for form in generated_forms.get(name, [])]
        if normalize_word(article["title"]) not in map(normalize_word, lemmas):
            return None, f"построенная начальная форма «{lemmas[0] if lemmas else ''}» ≠ заголовку", []
        if any(LATIN_LETTER.search(normalize("NFD", stem)) for stem in stems.values()):
            warnings.append("латинские буквы в основе: " + ", ".join(stems.values()))
        return generated_forms, None, warnings

    def process_adjective(self, article, outp):
        generated_forms, problem, warnings = self.generate_adjective_forms(article)
        if problem:
            self.add_failure(article, problem)
        else:
            for warning in warnings:
                self.add_failure(article, "предупреждение: " + warning)
            article["generated_forms"] = generated_forms
            outp.write(json.dumps(article, ensure_ascii=False) + "\n")

    @staticmethod
    def verb_layout(cells: dict) -> Tuple[List[Tuple[str, Optional[str]]], dict]:
        """
        Which cells of `Гл-блок` are shown and what forms they are, see the templates `Гл-блок` and `спряжения`.
        :return: pairs (cell, form name), the name is None for a past cell with all genders;
            and the properties of the verb
        """
        def flag(name):
            return plain_text(cells.get(name, "")).strip()

        aspect = {"с": "pf", "сов": "pf", "н": "impf", "несов": "impf"}.get(flag("вид"), "both")
        impersonal, frequentative = bool(flag("безличный")), bool(flag("многократный"))
        transitive = flag("НП") in ("", "2")
        layout = [("Инфинитив", "inf")]
        # verbs of repeated action (`хаживать`) have no personal forms, impersonal ones have only the 3rd person
        if not frequentative:
            persons = ["Он/она/оно"] if impersonal else ["Я", "Ты", "Он/она/оно", "Мы", "Вы", "Они"]
            layout += [(cell, VERB_PERSON_CELLS[cell]) for cell in persons]
        all_genders = "Он/она/оно (прош.)" if flag("Он/она/оно (прош.)") else None
        if impersonal:
            layout.append((all_genders or "Оно (прош.)", "pst-sg-n"))
        else:
            layout += [(all_genders, None)] if all_genders else \
                [("Он (прош.)", "pst-sg-m"), ("Она (прош.)", "pst-sg-f"), ("Оно (прош.)", "pst-sg-n")]
            layout.append(("Мы (прош.)", "pst-pl"))
            layout += [("Ты (повел.)", "imp-sg-2"), ("Вы (повел.)", "imp-pl-2")]
            if aspect == "pf":
                if not frequentative:
                    layout.append(("Мы (повел.)", "imp-pl-1"))
                # perfective verbs use other cells, `ПричСтрад` is the past passive participle for them
                layout.append(("Прич" if flag("Прич") else "ПричПрош", "ptcp-pst-act"))
                if transitive and not flag("ПричСтрадНет"):
                    layout.append(("ПричСтрад", "ptcp-pst-pass"))
                layout.append(("Деепр" if flag("Деепр") else "ДеепрПрош", "ger-pst"))
            else:
                if not frequentative:
                    layout += [("ПричНаст", "ptcp-prs-act")]
                layout.append(("ПричПрош", "ptcp-pst-act"))
                if transitive:
                    if not frequentative:
                        layout.append(("ПричСтрад", "ptcp-prs-pass"))
                    if not flag("ПричСтрадНет"):
                        layout.append(("ПричСтрадПрош", "ptcp-pst-pass"))
                if not frequentative:
                    layout.append(("ДеепрНаст", "ger-prs"))
                layout.append(("ДеепрПрош", "ger-pst"))
        properties = {"aspect": aspect, "transitive": transitive}
        if impersonal:
            properties["impersonal"] = True
        if frequentative:
            properties["frequentative"] = True
        return layout, properties

    @staticmethod
    def difficult_cells(cells: dict) -> set:
        """
        The cells marked as difficult by the flags of `Гл-блок` (`ПовелЗатрудн`) or by `{{difficult|...}}`.
        """
        result = {cell for cell, value in cells.items() if DIFFICULT in value}
        flags = {"Ед1лЗатрудн": ["Я"], "ПовелЗатрудн": ["Ты (повел.)", "Вы (повел.)"],
                 "ДеепрНастЗатрудн": ["ДеепрНаст"], "ПричСтрадЗатрудн": ["ПричСтрад", "ПричСтрадПрош"]}
        for flag, flag_cells in flags.items():
            if plain_text(cells.get(flag, "")).strip():
                result.update(flag_cells)
        return result

    def generate_verb_forms(self, article) -> Tuple[Optional[dict], Optional[str], List[str]]:
        article_template = remove_comments(article["morpho"]["template"]).strip()
        if article_template == "гл ru":
            return None, "не указан тип по Зализняку (шаблон «гл ru» без параметров)", []
        params = dict(article["morpho"]["info"])
        stems = article["morpho"]["stems"]
        segmented_stems = {}
        if article_template == VERB_TABLE_TEMPLATE:
            # the table is filled in the article itself
            cells = {key: evaluate_wiki(value, params) for key, value in params.items()}
        else:
            if article_template not in self.templates:
                return None, f"шаблон «{article_template}» не найден или не сводится к таблице «Гл-блок»", []
            if not clean_string(stems.get("основа", "")):
                return None, "пустая основа", []
            if self.multiword_stem(stems):
                return None, f"основа из нескольких слов: «{self.multiword_stem(stems)}»", []
            segmented_stems = self.segment_stems(stems, article["processed_stems"])
            params.update(self.stem_params(stems))
            cells = {key: evaluate_wiki(value, params)
                     for key, value in self.templates[article_template]["template"].items()}
        if is_empty(plain_text(cells.get("Инфинитив", ""))):
            cells["Инфинитив"] = self.segmented_lemma(article)
        layout, properties = self.verb_layout(cells)
        difficult_cells = self.difficult_cells(cells)
        generated_forms, difficult, warnings = {}, set(), []

        def add(name, forms):
            for form in forms:
                if form not in generated_forms.setdefault(name, []):
                    generated_forms[name].append(form)

        for cell, name in layout:
            forms, problem = self.build_adjective_forms(normalize_cell(cells.get(cell, "")), segmented_stems)
            if problem:
                warnings.append(f"клетка «{cell}» не построена, {problem}")
            for form in forms:
                if IMPOSSIBLE_LETTERS.search(form.replace(SEGMENT_SEPARATOR, "")):
                    warnings.append(f"клетка «{cell}»: невозможная форма «{form}»")
                form_name = name or past_form_name(form)
                if form_name.startswith("ptcp"):
                    if not is_participle_shape(form, form_name):
                        # `тя́нутый` in the cell of the present passive participle
                        other = form_name.replace("prs", "pst") if "prs" in form_name else None
                        if other and is_participle_shape(form, other):
                            warnings.append(f"клетка «{cell}»: «{form}» — причастие прошедшего времени")
                            form_name = other
                        else:
                            warnings.append(f"клетка «{cell}»: подозрительная форма причастия «{form}»")
                    declined = decline_participle(form, form_name.endswith("pass"))
                    if declined is None:
                        warnings.append(f"клетка «{cell}»: «{form}» не склоняется как причастие")
                        continue
                    for case, case_forms in declined.items():
                        add(f"{form_name}-{case}", case_forms)
                else:
                    add(form_name, [form])
                if cell in difficult_cells:
                    difficult.add(form_name)
        if not generated_forms:
            return None, "формы не построились", warnings
        infinitives = [form.replace(SEGMENT_SEPARATOR, "") for form in generated_forms.get("inf", [])]
        if normalize_word(article["title"]) not in map(normalize_word, infinitives):
            return None, f"построенный инфинитив «{infinitives[0] if infinitives else ''}» ≠ заголовку", []
        if any(LATIN_LETTER.search(normalize("NFD", stem)) for stem in stems.values()):
            warnings.append("латинские буквы в основе: " + ", ".join(stems.values()))
        article.update(properties)
        if difficult:
            article["difficult"] = sorted(difficult)
        return generated_forms, None, warnings

    def process_verb(self, article, outp):
        generated_forms, problem, warnings = self.generate_verb_forms(article)
        if problem:
            self.add_failure(article, problem)
        else:
            for warning in warnings:
                self.add_failure(article, "предупреждение: " + warning)
            article["generated_forms"] = generated_forms
            outp.write(json.dumps(article, ensure_ascii=False) + "\n")

    def generate_invariable_forms(self, article) -> Tuple[Optional[dict], Optional[str], List[str]]:
        """
        Adverbs, particles, prepositions etc.: the only form is the title; adverbs may have a comparative.
        """
        info = article["morpho"]["info"]
        generated_forms = {"inv": [self.segmented_lemma(article)]}
        warnings = []
        comparative = ",".join(info[name] for name in (ADJ_COMPARATIVE, "степень2") if info.get(name, "").strip())
        comparatives, problem = self.build_comparatives(evaluate_wiki(comparative, {}), {})
        if comparatives:
            generated_forms["comp"] = comparatives
        if problem:
            warnings.append(f"сравнительная степень, {problem}")
        # `{{adv ru|или=частица}}`: the word is also another part of speech
        also = [clean_markers(info[name]).strip() for name in ("или", "или1", "или2") if info.get(name, "").strip()]
        if also:
            article["also"] = also
        return generated_forms, None, warnings

    def generate_case_forms(self, article, cells: dict, names: Dict[str, str]) \
            -> Tuple[Optional[dict], Optional[str], List[str]]:
        """
        A case table, see CASE_TABLES: `падежи ru 1` (`nom`, `gen`, ...), `падежи` (`nom-sg`, ...),
        `падежи ru m n f pl` (`nom-m`, ...), `Фам-блок`, or a table in the wiki markup.
        :param names: the form names of the cells
        """
        generated_forms, warnings = {}, []
        segmented_stems = self.segment_stems(article["morpho"]["stems"], article.get("processed_stems", {}))
        for cell, name in names.items():
            forms, problem = self.build_adjective_forms(normalize_cell(cells.get(cell, "")), segmented_stems)
            # several cells may give one form: the plural of both genders in `Фам-блок-мж`
            for form in forms:
                if form not in generated_forms.setdefault(name, []):
                    generated_forms[name].append(form)
            if not generated_forms.get(name):
                generated_forms.pop(name, None)
            if problem:
                warnings.append(f"форма {name} не построена, {problem}")
        if not generated_forms:
            return None, "формы не построились", warnings
        lemmas = [form.replace(SEGMENT_SEPARATOR, "")
                  for name in ("nom", "nom-sg", "nom-sg-m", "nom-m", "nom-pl") for form in generated_forms.get(name, [])]
        if lemmas and normalize_word(article["title"]) not in map(normalize_word, lemmas):
            return None, f"построенная начальная форма «{lemmas[0]}» ≠ заголовку", []
        return generated_forms, None, warnings

    @staticmethod
    def pronoun_numeral_pos(article) -> Optional[str]:
        """
        The part of speech by the template or by the header after a table given in the article.
        """
        names = [article["morpho"]["template"]] + article["morpho"].get("section_templates", [])
        if any(name.startswith("числ") for name in names):
            return "num"
        if any(name.startswith(("мест", "Мс")) for name in names):
            # `мест ru 1a-н`: possessive adjectives with the pronominal declension, `мамин`
            kind = article["morpho"]["info"].get("часть речи", "")
            if kind.startswith("Прилагательное") or article["morpho"]["template"] in ("мест ru 1a-н", "мест ru 1a-в"):
                return "padj"
            return "pron"
        return None

    def generate_pronoun_numeral_forms(self, article) -> Tuple[Optional[dict], Optional[str], List[str]]:
        article_template = remove_comments(article["morpho"]["template"]).strip()
        params = dict(article["morpho"]["info"])
        template = self.templates.get(article_template)
        if template and template.get("pos") == "adj":
            return self.generate_adjective_forms(article)
        if template and template.get("pos") in ("cases", "wikitable"):
            params.update(self.stem_params(article["morpho"]["stems"]))
            if template["pos"] == "cases":
                cells = {key: evaluate_wiki(value, params) for key, value in template["template"].items()}
                return self.generate_case_forms(article, cells, CASE_TABLES[template["table"]])
            cells = {name: "<br>".join(values)
                     for name, values in table_cells(evaluate_wiki(template["text"], params)).items()}
            return self.generate_case_forms(article, cells, {name: name for name in cells})
        if article_template in CASE_TABLES:
            # the table is filled in the article itself
            return self.generate_case_forms(article, {key: evaluate_wiki(value, params)
                                                      for key, value in params.items()},
                                            CASE_TABLES[article_template])
        if article_template not in self.templates and "{|" not in article_template:
            return self.generate_untabled_forms(article)
        return None, f"шаблон «{article_template}» не найден или не сводится к таблице", []

    def find_paradigm(self, title: str) -> Optional[dict]:
        """
        The forms of another word, a pronoun or an adjective: the base of `который-то`.
        """
        if self.articles_by_title is None:
            self.articles_by_title = {}
            for article in self.articles:
                self.articles_by_title.setdefault(article["title"], []).append(article)
        for article in self.articles_by_title.get(title, []):
            article_template = remove_comments(article["morpho"].get("template") or "").strip()
            if determine_pos(article) == "adj":
                forms, problem, _ = self.generate_adjective_forms(json.loads(json.dumps(article)))
            elif article_template.startswith(PRONOUN_NUMERAL_PREFIXES):
                forms, problem, _ = self.generate_pronoun_numeral_forms(json.loads(json.dumps(article)))
            else:
                continue
            # an indeclinable word without a table is not a base
            if forms and "inv" not in forms:
                return forms
        return None

    def generate_untabled_forms(self, article) -> Tuple[Optional[dict], Optional[str], List[str]]:
        """
        A pronoun or a numeral with a header but without a table: `который-то` is declined as `который`,
        `какой-никакой` as `какой` twice; other words are taken as indeclinable, with `no_table`.
        """
        parts = article["title"].split("-")
        if len(parts) == 2:
            first = self.find_paradigm(parts[0])
            if first and parts[1] in PARTICLES:
                forms = {name: [form.replace(SEGMENT_SEPARATOR, "") + "-" + parts[1] for form in variants]
                         for name, variants in first.items() if name != "comp"}
                return forms, None, [f"формы построены по слову «{parts[0]}» с частицей"]
            second = self.find_paradigm(parts[1])
            if first and second:
                forms = {name: [first[name][0].replace(SEGMENT_SEPARATOR, "") + "-" +
                                second[name][0].replace(SEGMENT_SEPARATOR, "")]
                         for name in first if name in second and name != "comp"}
                return forms, None, [f"формы построены по словам «{parts[0]}» и «{parts[1]}»"]
        article["no_table"] = True
        return {"inv": [self.segmented_lemma(article)]}, None, ["нет таблицы склонения, взята одна форма"]

    @staticmethod
    def abbreviation_role(article) -> dict:
        """
        `{{abbrev|роль=существительного мужского рода единственного числа}}`: what the abbreviation is used as.
        """
        role = clean_markers(article["morpho"]["info"].get("роль", "")).strip()
        result = {"role": role} if role else {}
        for pattern, key, value in ABBREVIATION_ROLES:
            if pattern in role:
                result.setdefault(key, value)
        return result

    def process_other(self, article, outp):
        article_template = remove_comments(article["morpho"]["template"]).strip()
        if article_template in INVARIABLE_TEMPLATES:
            pos = INVARIABLE_TEMPLATES[article_template]
            generated_forms, problem, warnings = self.generate_invariable_forms(article)
        elif article_template == "abbrev":
            pos = "abbr"
            article.update(self.abbreviation_role(article))
            generated_forms, problem, warnings = {"inv": [clean_string(article["title"])]}, None, []
        elif article_template.startswith(PRONOUN_NUMERAL_PREFIXES):
            pos = self.pronoun_numeral_pos(article)
            if pos is None:
                return
            generated_forms, problem, warnings = self.generate_pronoun_numeral_forms(article)
        else:
            return
        if problem:
            self.add_failure(article, problem)
            return
        for warning in warnings:
            self.add_failure(article, "предупреждение: " + warning)
        article["pos"] = pos
        article["generated_forms"] = generated_forms
        outp.write(json.dumps(article, ensure_ascii=False) + "\n")

    def process_surname(self, article, outp):
        generated_forms, problem, warnings = self.generate_surname_forms(article)
        if problem:
            self.add_failure(article, problem)
            return
        for warning in warnings:
            self.add_failure(article, "предупреждение: " + warning)
        article["pos"] = "surname"
        article["generated_forms"] = generated_forms
        outp.write(json.dumps(article, ensure_ascii=False) + "\n")

    def generate_surname_forms(self, article) -> Tuple[Optional[dict], Optional[str], List[str]]:
        """
        Surnames: the templates `Фам ...` call the table `Фам-блок` (`им-м`, `им-ж`, `им-мн`, ...).
        """
        article_template = remove_comments(article["morpho"]["template"]).strip()
        params = dict(article["morpho"]["info"])
        warnings = []
        if article_template in ("Фам ru", "Фам"):
            # the stem in the article is the whole surname with the stress (`Хво́рост`) or already a stem (`Капи́ц`)
            surname = clean_stem_markup(article["morpho"]["stems"].get("основа", ""))
            if normalize_word(surname) != normalize_word(article["title"]):
                surname = article["title"]
            return self.guessed_surname_forms(article, surname)
        template = self.templates.get(article_template)
        if template and template.get("pos") == "cases":
            if not clean_string(article["morpho"]["stems"].get("основа", "")) and not params.get("1"):
                return None, "пустая основа", []
            params.update(self.stem_params(article["morpho"]["stems"]))
            cells = {key: evaluate_wiki(value, params) for key, value in template["template"].items()}
            forms, problem, case_warnings = self.generate_case_forms(article, cells, CASE_TABLES[template["table"]])
            return forms, problem, warnings + case_warnings
        if article_template == "Фам-блок":
            return self.generate_case_forms(article, {key: evaluate_wiki(value, params)
                                                      for key, value in params.items()}, CASE_TABLES["Фам-блок"])
        return None, f"шаблон «{article_template}» не найден или не сводится к таблице «Фам-блок»", []

    def guessed_surname_forms(self, article, surname: str) -> Tuple[Optional[dict], Optional[str], List[str]]:
        """
        A surname without a table: the template is chosen by the ending (`src/surnames.py`),
        both parts of a double surname are declined: `Гапонов-Грехов`, `Гапонова-Грехова`.
        """
        parts_forms, guessed = [], []
        for part in surname.split("-"):
            template_and_stem = guess_surname_template(part)
            template = self.templates.get(template_and_stem[0]) if template_and_stem else None
            if not template:
                return None, f"нет таблицы склонения, тип не подобран по окончанию «{part}»", []
            stems = {"основа": template_and_stem[1]}
            params = dict(article["morpho"]["info"])
            params.update(self.stem_params(stems))
            segmented_stems = self.segment_stems(stems, {})
            forms = {}
            for cell, name in CASE_TABLES[template["table"]].items():
                built, _ = self.build_adjective_forms(
                    normalize_cell(evaluate_wiki(template["template"].get(cell, ""), params)), segmented_stems)
                for form in built:
                    if form not in forms.setdefault(name, []):
                        forms[name].append(form)
            parts_forms.append({name: values for name, values in forms.items() if values})
            guessed.append(template_and_stem[0])
        generated_forms = {name: ["-".join(variants[0] for variants in combination)]
                           for name in parts_forms[0]
                           for combination in [[forms.get(name) for forms in parts_forms]]
                           if all(combination)}
        if len(parts_forms) == 1:
            generated_forms = parts_forms[0]
        lemmas = [form.replace(SEGMENT_SEPARATOR, "") for form in generated_forms.get("nom-sg-m", [])]
        if normalize_word(article["title"]) not in map(normalize_word, lemmas):
            return None, f"построенная начальная форма «{lemmas[0] if lemmas else ''}» ≠ заголовку", []
        article["morpho"]["guessed_template"] = " + ".join(guessed)
        return generated_forms, None, [f"нет таблицы склонения, шаблон «{' + '.join(guessed)}» подобран по окончанию"]

    def write_failures(self):
        with open(self.args.failures, "w") as outp:
            outp.write("title\ttemplate\treason\turl\n")
            for title, template, reason in sorted(self.failures, key=lambda f: (f[2], f[0])):
                url = "https://ru.wiktionary.org/wiki/" + title.split(" (")[0]
                outp.write(f"{title}\t{template}\t{reason}\t{url}\n")

    # the process of attaching corresponding stems to their endings
    def generate_word_forms(self):
        with open(self.args.output_articles, "w") as outp:
            for article in self.articles:
                if self.args.pos == "surname":
                    # surnames are proper names, the filter would drop them
                    template = article["morpho"].get("template") or ""
                    if template.startswith(SURNAME_PREFIXES) and not article["is_obscene"]:
                        self.process_surname(article, outp)
                    continue
                if not basic_filter(article):
                    continue
                pos = determine_pos(article)
                if pos == "noun" and self.args.pos == "noun":
                    self.process_article(article, outp)
                elif pos == "adj" and self.args.pos == "adj":
                    self.process_adjective(article, outp)
                elif self.args.pos == "verb" and (pos == "verb" or
                                                  article["morpho"].get("template") == VERB_TABLE_TEMPLATE):
                    self.process_verb(article, outp)
                elif self.args.pos == "other":
                    self.process_other(article, outp)
        if self.args.pos != "noun":
            self.write_failures()


def parse_args():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('--output-articles', type=str,
                            dest="output_articles",
                            default='processed_articles.jsonl')
    arg_parser.add_argument('--input-articles', type=str,
                            dest="input_articles",
                            default='tmp/processed/articles.jsonl')
    arg_parser.add_argument('--pos', type=str, choices=["noun", "adj", "verb", "other", "surname"], default="noun")
    arg_parser.add_argument('--failures', type=str, default='adj_failures.tsv',
                            help="words whose forms could not be built (with --pos adj|verb|other|surname)")

    args = arg_parser.parse_args()
    return args


if __name__ == '__main__':
    wkt = TWiktionary(parse_args())
    wkt.read_db()
    wkt.generate_word_forms()
