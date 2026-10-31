import json

from constants import WRITE_PATHS
from src.generation import segment_stem
from src.segmentation import clean_segments
from src.utils.etc import basic_json_read

articles = basic_json_read(WRITE_PATHS["article"])

with open("tmp/processed/articles.jsonl", "w") as file:
    for article in articles:
        segmented: str = clean_segments(article["segments"])
        stems: dict = article["morpho"]["stems"]
        processed_stems = {}
        for name, stem in stems.items():
            # without morpheme segmentation the stems cannot be segmented either
            if stem and segmented:
                processed_stems[name] = segment_stem(stem, segmented)
        article["processed_stems"] = processed_stems
        file.write(json.dumps(article, ensure_ascii=False) + "\n")
