# ru_wiktionary_merger

Generates Russian paradigms from a dump of ru.wiktionary, exports them to the slf format of MorphWizard and compares
them with the Russian dictionary of aot.ru (`../data/Russian`), to merge them into it.

It is based on [RuWiktionaryParser](https://github.com/slowwavesleep/RuWiktionaryParser) by Aleksei Dorkin
(MIT license, see [LICENSE](LICENSE)), the history of that repository is kept. The original project generated nouns,
adjectives, verbs, other parts of speech, surnames, the evaluation of wiki templates, the slf export and the comparison
were added here.

Use a virtual environment: `python -m venv venv; venv/bin/pip install -r requirements.txt`.

## Data extraction
The first step of the process is the extraction of the data from the downloaded Wiktionary dump.
The most recent version can be found [here](https://dumps.wikimedia.org/backup-index.html). You'll want
the pages-articles version of the dump which hash the following name format: 
__ruwiktionary-{date}-pages-articles.xml.bz2__. You don't need to unpack it. Just put it in a desired
directory, then specify the path in the [config.yml](config.yml) file.

### Cache the dump
```
python cache_dump.py
```
Reads the dump once (about 5 minutes, `lbzip2` is used if installed) and stores the Russian
pages in `tmp/cache/pages.jsonl`: the Russian section of each article, Russian templates and
template redirects, and a few templates they call (`EXTRA_TEMPLATES`, e.g. the verb table `Гл-блок`). The XML namespace version of the dump does not matter.

### Extract data from the dump
```
python extract_from_dump.py                # articles and templates, about 3 minutes
python extract_from_dump.py --templates    # templates only, seconds
python extract_from_dump.py --articles     # articles only
```
Works with the cache (builds it first if it is missing, `--rebuild-cache` reads the dump again).
Three files will be generated in the `tmp` folder: `articles.jsonl`, `templates.jsonl`,
and `template_redirects.jsonl`. Homonyms (several morphological sections on one page) give
separate articles with a `homonym` label taken from the section header (`I`, `II`, ...).

### Some examples of the format

#### An article
```
{
  "morpho": {
    "template": "сущ ru m ina 1a",
    "stems": {
      "основа": "перево́д"
    },
    "alternate": {},
    "forms": {},
    "info": {
      "слоги": "{{по-слогам|пе|ре|во́д}}"
    }
  },
  "segments": {
    "1": "пере-",
    "2": "вод",
    "и": "т"
  },
  "id": 3941,
  "title": "перевод",
  "type": "article",
  "is_proper": false,
  "is_obscene": false,
  "has_homonyms": false
}
```

#### A template

```
{
  "template": {
    "case": "{{{case|}}}",
    "form": "{{{form|}}}",
    "nom-sg": "{{{основа}}}я",
    "nom-pl": "{{{основа}}}и",
    "gen-sg": "{{{основа}}}и",
    "gen-pl": "{{{основа1}}}ь",
    "dat-sg": "{{{основа}}}е",
    "dat-pl": "{{{основа}}}ям",
    "acc-sg": "{{{основа}}}ю",
    "acc-pl": "{{{основа}}}и",
    "ins-sg": "{{{основа}}}ей",
    "ins-sg2": "{{{основа}}}ею",
    "ins-pl": "{{{основа}}}ями",
    "prp-sg": "{{{основа}}}е",
    "prp-pl": "{{{основа}}}ях",
    "loc-sg": "{{{М|}}}",
    "voc-sg": "{{{З|}}}",
    "prt-sg": "{{{Р|}}}",
    "П": "{{{П|}}}",
    "hide-text": "{{{hide-text|}}}",
    "слоги": "{{{слоги|}}}",
    "дореф": "{{{дореф|}}}",
    "Сч": "{{{Сч|}}}",
    "st": "{{{st|}}}",
    "pt": "{{{pt|}}}",
    "затрудн": "{{{затрудн|}}}",
    "коммент": "{{{коммент|}}}",
    "зачин": "{{{зачин|}}}",
    "клитика": "{{{клитика|}}}",
    "кат": "неодуш",
    "род": "{{{род|жен}}}",
    "скл": "1",
    "зализняк": "2*a^",
    "чередование": "1"
  },
  "id": 15763,
  "title": "сущ ru f ina 2*a^",
  "type": "template"
}
```
#### A template redirect
```
{
  "id": 836905,
  "title": "сущ ru f ina 2*a-ня(2)",
  "redirect_title": "сущ ru f ina 2*a(2)-ня",
  "type": "template_redirect"
}
```

Suppose that the extracted data in this form may have its uses. Do note, however, that homonymous
lemmas _are currently not accounted for_. This means that if a certain page has articles for several
word senses, then only the first one will be saved.

## Generating Word Forms
The main purpose of this project is to generate word forms with morpheme boundaries marked using the provided
wiki-templates and segmented lemmas. To do this, some preliminary steps are required.

*Note:* only nouns are supported at the moment.

### Process the templates
This will filter and reformat `templates.jsonl`.
```
python process_templates.py
```

### Process the articles
This will clean up and enrich `articles.json`. 
```
python process_articles.py
```

The results of the last two steps will be saved in `tmp/processed/` folder by default.

#### Generate the segmented noun word forms
This will use the processed files to generate word forms (where possible). Obscene words
and proper names are not used.
```
python generate_nouns.py
```
The output is saved to `result/processed_articles.jsonl` by default. The file should
contain approximately 500 thousand noun word forms.

## Adjectives
```
python process_templates.py
python process_articles.py
python generate_wordforms.py --pos adj --output-articles result/adjectives.jsonl --failures result/adj_failures.tsv
```
Adjective templates contain no endings, they call each other (`прил ru 1*a` -> `прил ru 1a`) and end
with the table template `прил`; `process_templates.py` resolves these chains. Each form is a list
of variants (`твёрдой`, `твёрдою`), `comp` contains the comparative. Adjectives whose forms could
not be built are listed in the failures file with the reason and a link to the article.

Indeclinable adjectives (`прил ru 0`) get the lemma in all full forms and `"indeclinable": true`.
Articles with `{{прил-ru|безымя́нный|1*a}}` are rendered by a Lua module, so their tables are not in the
dump. For them the template `прил ru <index>` is used (`src/adjectives.py`): the index is reduced to an
existing template (`1b(X)` -> `1bX`), the stems are derived from the lemma, including the short form stem
with Zaliznyak's fleeting vowel (`безымянн` -> `безымянен`, `связанн` (1) -> `связан`). The template used
is stored in `morpho.lua_template`.

## Verbs
```
python generate_wordforms.py --pos verb --output-articles result/verbs.jsonl --failures result/verb_failures.tsv
```
Verb templates (`гл ru 1a`, `гл ru 1aСВ`) pass all cells to the table template `Гл-блок`; the cells are
evaluated for each article and shown the way `Гл-блок` and `спряжения` show them: perfective verbs use the
cells `Прич`, `Деепр`, `Мы (повел.)`, and their `ПричСтрад` is the past passive participle; passive
participles are shown only for transitive verbs (`НП` is empty), impersonal verbs (`безличный`) have only the
3rd person, frequentative ones (`многократный`) have no present forms. The infinitive of perfective verbs is
the title of the article.

Form names: `inf`, `npst-sg-1` ... `npst-pl-3` (the present for imperfective verbs, the future for perfective
ones), `pst-sg-m`, `pst-sg-f`, `pst-sg-n`, `pst-pl`, `imp-sg-2`, `imp-pl-2`, `imp-pl-1`, `ger-prs`, `ger-pst`.
Participles are declined in `src/verbs.py`, since the table gives only their initial forms:
`ptcp-prs-act-`, `ptcp-pst-act-`, `ptcp-prs-pass-`, `ptcp-pst-pass-` followed by an adjective form name
(`nom-sg-m`, ..., `srt-sg-f` for the short forms of passive participles). Variants are separated in the cells
by `<br>`, commas, `/`, `//` and parentheses (`до́хнул (дох)`). The article gets `aspect` (`impf`, `pf`, `both`),
`transitive`, `impersonal`, `frequentative`, and `difficult`: the forms marked as difficult (`ПовелЗатрудн`,
`{{difficult|...}}`). Errors of the articles found on the way (a wrong template: `ощети́нйся`, a wrong stem:
`пережида́йнный`) are listed in the failures file as warnings, the forms are kept as Wiktionary shows them.

## Other parts of speech
```
python generate_wordforms.py --pos other --output-articles result/other.jsonl --failures result/other_failures.tsv
```
Each article gets `pos`: `adv`, `pred`, `particle`, `prep`, `conj`, `intrj`, `par` (parenthetical words), `ono`
(onomatopoeia) by the header templates `adv ru`, `predic ru`, ... (`INVARIABLE_TEMPLATES`), their only form is
`inv`; adverbs may have `comp` (the parameter `степень`); `also` keeps another part of speech of the word
(`{{adv ru|или=частица}}`).

Pronouns (`pron`), numerals (`num`) and possessive adjectives with the pronominal declension (`padj`, `мамин`)
have templates `мест ru ...`, `числ ...`, `Мс-...`: they end with the adjective table `прил` (the forms are named
as for adjectives) or with a case table (`CASE_TABLES`): `падежи ru 1` (`nom`, `gen`, ...), `падежи` (`nom-sg`,
...), `падежи ru m n f pl` (`nom-sg-m`, ...). The table may be given in the article itself
(`{{падежи ru 1|nom=он|gen=его́, него́|...}}`), then the part of speech is taken from the next header template
(`section_templates`). Forms with stylistic labels (`{{устар.|-}}&nbsp;ея́`) and analytic forms (`ни о ком`) are
dropped. Templates written in the wiki table markup (`числ ru 2`, `числ-собир1b`) are parsed by `src/wikitable.py`:
the case of a row and the gender of a column are recognized by their labels (`Им.`, `Вн.` + `одуш.`, `м./ср. р.`).
Pronouns with a header and no table: `который-то` is declined as `который` with the particle, `такой-сякой` as
both words; the others get the only form `inv` and `no_table`.

Abbreviations (`{{abbrev|роль=существительного мужского рода единственного числа}}`) are `abbr` with the only
form `inv`; `abbr_pos`, `gender`, `number` are taken from `роль`.

## Surnames
```
python generate_wordforms.py --pos surname --output-articles result/surnames.jsonl --failures result/surname_failures.tsv
```
Surname templates (`Фам ru ин ов`, `Фам ru m 1a`, ...) call the table `Фам-блок` (`им-м`, `им-ж`, `им-мн`, the forms
are named `nom-sg-m`, `nom-sg-f`, `nom-pl`) or `Фам-блок-мж` (the plural of each gender). Surnames are proper names,
the filter of proper names is not applied to them. A surname with the header `{{Фам ru|основа=Хво́рост}}` and no
table gets the template by its ending (`src/surnames.py`: `-ов`, `-ин` like `Иванов`, a consonant like `Хворост`,
`-о`, `-их` indeclinable, ...), both parts of a double surname are declined; the template is stored in
`morpho.guessed_template`.

## Export to slf and the comparison with aot.ru
```
python generate_wordforms.py --pos noun --output-articles result/nouns.jsonl
python export_slf.py
slf_convert --to-mrd --slf result/wiktionary.slf --mwz result/mrd/project.mwz --user wiktionary
python compare_with_aot.py
```
`export_slf.py` writes all paradigms of `result/*.jsonl` to `result/wiktionary.slf`, the slf format of MorphWizard
(`src/slf.py`): a paradigm is a list of lines `form POS grammems` with the Latin names of the Russian gramtab of aot.ru
(`N mas,sg,nom`, `PARTICIPLE pres,act,mas,sg,acc,anim`), the lemma first, the animacy of nouns and the aspect and
transitivity of verbs are in `$type_grm`. The forms are named as in aot: the non-past of a perfective verb is the
future, a biaspectual verb has both, a surname is one paradigm of both genders (`N mas,fem,pl,nom` for the plural),
an indeclinable word is one form (`N mas,0`, `A 0,anim,inanim`), a pronoun-adjective is `PA`, an ordinal numeral is
`ORD_NUM`. The comparatives of adverbs are not exported, in aot they are forms of adjectives. Stress marks are lost.

The slf file is loaded into a dictionary of MorphWizard (`result/mrd/`: `project.mwz`, `gramtab.json` of aot.ru,
`morphs.json`) by the C++ utility `slf_convert` of morph_dict (`--to-slf` writes a dictionary back to slf).

`compare_with_aot.py` compares `result/mrd/morphs.json` with `morph_dict/data/Russian/morphs.json`, `ё` is compared
as `е`. Paradigms are matched by the lemma and the POS of the lemma (homonyms by the number of shared forms), the
results are in `result/compare/`:
- `summary.tsv`: pairs, equal pairs, lemmas of one dictionary only, by POS;
- `categories.tsv`: kinds of differing forms, e.g. `ADJ_SHORT: only aot`, `N 2: only aot` (the second genitive);
- `groups.tsv`: pairs with the same differences, written as endings after the common stem, with the Wiktionary
  templates and examples;
- `pairs.tsv`: the differences of each pair;
- `only_wiki.tsv`, `only_aot.tsv`: unmatched lemmas, for a Wiktionary lemma the aot lemmas sharing its forms;
- `type_grammems.tsv`: differences of `$type_grm`.
