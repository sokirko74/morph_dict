from timeit import default_timer as timer

# TODO parse template pages and pages for words that don't provide stems explicitly
from src.utils.html import process_title

TEST_WORDS = [
    "дар",
    "друг",
    "заживо",
    "доха",
    "муж",
    "ребёнок",
    "младенец",
    "лужа",
    "крыша",
    "звезда",
    "молоко"
]


start = timer()
for word in TEST_WORDS:
    print(word)
    print(process_title(word))

end = timer()

print("\n" + f"Time elapsed: {end - start:.4f}")
