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
