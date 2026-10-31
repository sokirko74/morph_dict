"""
Inflection tables written in the wiki table markup (`{| ... |}`) instead of calling a table template,
e.g. the numeral templates `числ ru 2`, `числ-собир1b`. The table is turned into a grid (with `rowspan`
and `colspan`), the case of a row and the gender and number of a column are recognized by their labels.
"""
import re
from typing import Dict, List, Optional, Tuple

SPAN = re.compile(r"(colspan|rowspan)\s*=\s*\"?\s*(\d+)", re.IGNORECASE)
LINK = re.compile(r"\[\[(?:[^\]|]*\|)?([^\]]*)\]\]")
# the labels of the rows: `Им.`, `Рд.`, `Вн.`, `Тв.`, `Пр.` or the full names
CASE_LABEL = re.compile(r"^(?:(им|р|рд|род|д|дт|дат|в|вн|вин|тв|твор|п|пр|предл)\.|"
                        r"(именительный|родительный|дательный|винительный|творительный|предложный))")
CASE_NAMES = {"им": "nom", "р": "gen", "д": "dat", "в": "acc", "т": "ins", "п": "prp"}
# the labels of the columns
GENDER_LABELS: List[Tuple[re.Pattern, str]] = [
    (re.compile(r"(^|[\s/])м\.|муж"), "m"),
    (re.compile(r"(^|[\s/])с\.|(^|[\s/])ср\b|средн"), "n"),
    (re.compile(r"(^|[\s/])ж\.|жен"), "f"),
    (re.compile(r"мн"), "pl"),
]


def split_cell(cell: str) -> Tuple[str, str]:
    """
    Splits a cell into the attributes and the content: `colspan=2| {{{основа}}}у́х`,
    a `|` inside a link or a template is not a separator.
    """
    depth = 0
    for i, c in enumerate(cell):
        if cell.startswith(("[[", "{{"), i):
            depth += 1
        elif cell.startswith(("]]", "}}"), i) and depth:
            depth -= 1
        elif c == "|" and not depth:
            return cell[:i], cell[i + 1:]
    return "", cell


def split_row(line: str, separator: str) -> List[str]:
    cells, depth, start, i = [], 0, 0, 0
    while i < len(line):
        if line.startswith(("[[", "{{"), i):
            depth += 1
            i += 2
        elif line.startswith(("]]", "}}"), i) and depth:
            depth -= 1
            i += 2
        elif not depth and (line.startswith(separator, i) or line.startswith("||", i)):
            cells.append(line[start:i])
            i += 2
            start = i
        else:
            i += 1
    cells.append(line[start:])
    return cells


def parse_grid(text: str) -> List[List[Tuple[bool, str]]]:
    """
    The cells of the first table in the text as a grid, a cell spanning several rows or columns is repeated.
    :return: rows of (is a header, content)
    """
    start, end = text.find("{|"), text.find("|}")
    if start < 0:
        return []
    rows: List[List[Tuple[bool, str, int, int]]] = [[]]
    for line in text[start:end if end > start else len(text)].split("\n")[1:]:
        line = line.strip()
        if line.startswith("|-"):
            rows.append([])
        elif line.startswith(("!", "|")) and not line.startswith("|+"):
            header = line.startswith("!")
            for cell in split_row(line[1:], "!!" if header else "||"):
                attributes, content = split_cell(cell)
                if "=" not in attributes and attributes.strip():
                    attributes, content = "", cell
                spans = dict((name.lower(), int(value)) for name, value in SPAN.findall(attributes))
                rows[-1].append((header, content.strip(), spans.get("rowspan", 1), spans.get("colspan", 1)))
        elif rows[-1] and line:
            header, content, rowspan, colspan = rows[-1][-1]
            rows[-1][-1] = (header, content + "\n" + line, rowspan, colspan)
    grid: List[List[Tuple[bool, str]]] = []
    pending: Dict[Tuple[int, int], Tuple[bool, str]] = {}
    for row_index, row in enumerate(r for r in rows if r):
        cells, column = [], 0
        for header, content, rowspan, colspan in row:
            while (row_index, column) in pending:
                cells.append(pending.pop((row_index, column)))
                column += 1
            for i in range(colspan):
                cells.append((header, content))
                for j in range(1, rowspan):
                    pending[(row_index + j, column + i)] = (header, content)
            column += colspan
        while (row_index, column) in pending:
            cells.append(pending.pop((row_index, column)))
            column += 1
        grid.append(cells)
    return grid


def label_text(content: str) -> str:
    content = LINK.sub(r"\1", content)
    content = re.sub(r"<[^<>]*>|&nbsp;|&#160;", " ", content)
    return " ".join(content.split()).lower()


def case_of(content: str) -> Optional[str]:
    match = CASE_LABEL.search(label_text(content))
    if not match:
        return None
    label = match.group(1) or match.group(2)
    return CASE_NAMES["им"] if label.startswith("им") else CASE_NAMES[label[0]]


def table_cells(text: str) -> Dict[str, List[str]]:
    """
    The cells of an inflection table by form names: `nom`, `acc-a`, `acc-n` for a table with one column,
    `nom-m`, `nom-f` for a table with genders (`два`), `nom-sg-m`, `acc-sg-m-a`, `acc-pl-n`, as for adjectives,
    for a table with genders and the plural. A row `В. (одуш./неодуш.)` with two values `ого́<br />ый`
    gives them to the animate and the inanimate forms.
    :return: the wiki text of the cells
    """
    grid = parse_grid(text)
    columns: Dict[int, str] = {}
    result: Dict[str, List[str]] = {}
    for row in grid:
        # the label of a row is its first cell or a header cell before the data
        leading = [i for i, (header, _) in enumerate(row) if header or i == 0]
        leading = [i for n, i in enumerate(leading) if i == n]
        cases = [(i, case_of(row[i][1])) for i in leading if case_of(row[i][1])]
        if not cases:
            # a header row: the labels of the columns, `ед. ч.` above `м.`
            if result:
                continue
            for i, (_, content) in enumerate(row):
                columns[i] = (columns.get(i, "") + " " + label_text(content)).strip()
            continue
        label_index, case = cases[0]
        animacy = None
        data_start = label_index + 1
        # `одуш.` / `неод.` in the next header cell (or a cell with only this label) or in the label itself
        for i in range(label_index, len(row)):
            text_i = label_text(row[i][1])
            # a label with `colspan` is repeated in the next columns
            if i > label_index and row[i] == row[label_index]:
                data_start = i + 1
                continue
            if i > label_index and not row[i][0] and not re.fullmatch(r"(одуш|неодуш|неод)[а-яё]*\.?", text_i):
                break
            if "одуш" in text_i and "неод" not in text_i:
                animacy = "a"
            elif "неод" in text_i and ("одуш" not in text_i or text_i.startswith("неод")):
                animacy = "n"
            if "одуш" in text_i and "неод" in text_i and not text_i.startswith("неод"):
                animacy = "both"
            data_start = i + 1
        has_plural = any(GENDER_LABELS[-1][0].search(label) for label in columns.values())
        for i in range(data_start, len(row)):
            header, content = row[i]
            # a column of another style: `разговорная форма`
            if header or "разговорн" in columns.get(i, ""):
                continue
            genders = [gender for pattern, gender in GENDER_LABELS if pattern.search(columns.get(i, ""))]
            names = [f"{case}-sg-{gender}" if has_plural and gender != "pl" else f"{case}-{gender}"
                     for gender in genders] or [case]
            values = re.split(r"<br\s*/?>", content) if animacy == "both" else [content]
            for name in names:
                if case != "acc":
                    targets = [(name, content)]
                elif animacy in ("a", "n"):
                    targets = [(f"{name}-{animacy}", content)]
                elif animacy == "both" and len(values) == 2:
                    targets = [(f"{name}-a", values[0]), (f"{name}-n", values[1])]
                else:
                    targets = [(f"{name}-a", content), (f"{name}-n", content)]
                for target, value in targets:
                    if value not in result.setdefault(target, []):
                        result[target].append(value)
    # the same animate and inanimate forms are one form, except for the masculine and the plural,
    # as for adjectives: `acc-sg-f`, but `acc-sg-m-a`, `acc-sg-m-n`
    for name in [name for name in result if name.endswith("-a")]:
        base = name[:-2]
        if result.get(base + "-n") == result[name] and not base.endswith(("-m", "-pl")):
            result[base] = result.pop(name)
            del result[base + "-n"]
    return result
