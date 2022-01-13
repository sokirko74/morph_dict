import argparse

#adhoc script, do not support in future. Official support is only in C++


class TMrdFile:
    def __init__(self):
        self.flexia_models = list()
        self.accent_models = list()
        self.user_sessions = list()
        self.prefixes = list()
        self.lemmas = list()

    @staticmethod
    def read_section(inp):
        cnt = inp.readline().strip()
        assert cnt.isdigit()
        cnt = int(cnt)
        assert cnt > 0
        lines = list()
        for i in range(cnt):
            lines.append(inp.readline().strip())
        return lines

    @staticmethod
    def write_section(outp, lst):
        outp.write("{}\n".format(len(lst)))
        for l in lst:
            outp.write("{}\n".format(l))

    def read(self, filename):
        with open(filename) as inp:
            self.flexia_models = self.read_section(inp)
            self.accent_models = self.read_section(inp)
            self.user_sessions= self.read_section(inp)
            self.prefixes = self.read_section(inp)
            self.lemmas = self.read_section(inp)

    def write(self, filename):
        with open(filename, "w", newline='\n') as outp:
            self.write_section(outp, self.flexia_models)
            self.write_section(outp, self.accent_models)
            self.write_section(outp, self.user_sessions)
            self.write_section(outp, self.prefixes)
            self.write_section(outp, self.lemmas)


def read_mrd_selection_list(file_name):
    lemmas = dict()
    with open (file_name) as inp:
        for l in inp:
            stem, flex_model_id, accent_model_id, ancode, dummy = l.split()
            assert dummy == "-"
            assert ancode == "Уы"
            lemmas[stem] = flex_model_id
    return lemmas


def read_lemmas(file_name):
    lemmas = list()
    with open(file_name) as inp:
        for l in inp:
            lemmas.append(l.strip())
    return lemmas


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--action", dest='action', help="can be delete_by_file, create_for_misspell")
    parser.add_argument("--input-mrd-path", dest='in_mrd_path', help="for example C:/tmp/RML/Source/morph_dict/data/Russian/morphs.mrd")
    parser.add_argument("--output-mrd-path", dest='out_mrd_path',
                        help="for example C:/tmp/RML/Source/morph_dict/data/Russian/morphs.mrd.new")
    parser.add_argument("--word-list", dest='word_list')
    return parser.parse_args()


def delete_by_file(dict, lemmas_to_delete):
    new_lemmas = list()
    used = set()
    for l in dict.lemmas:
        stem, flex_model_id, accent_model_id, user_session, ancode, dummy = l.split()
        if lemmas_to_delete.get(stem, -1) == flex_model_id:
            used.add(stem)
            continue
        new_lemmas.append(l)
    assert len(new_lemmas) < len(dict.lemmas)
    dict.lemmas = new_lemmas
    for l in lemmas_to_delete:
        if l not in used:
            print("not deleted stem {}".format(l))


def create_for_misspell(dict, lemmas_to_delete):
    dict.flexia_models.append('%*яя')
    dict.accent_models.append('255')
    dict.user_sessions.append('alex;16:15, 09 November 2004;16:30, 09 November 2004')
    for l in lemmas_to_delete:
        entry = "{} 0 0 0 Ун - ".format(l)
        dict.lemmas.append(entry)


def main():
    args = parse_args()
    dict = TMrdFile()
    if args.action == "delete_by_file":
        dict.read_mrd_selection_list(args.in_mrd_path)
        lemmas_to_delete = read_list(args.word_list);
        delete_by_file(dict, lemmas_to_delete)
    elif args.action == "create_for_misspell":
        words = read_lemmas(args.word_list);
        create_for_misspell(dict, words)

    dict.write(args.out_mrd_path)


if __name__ == "__main__":
    main()


