import random
import os

PATTERNS = [
        "VIRUS",
        "EVILSCRIPT",
        "KEYLOGGER",
        "CRYPTOMINER"
    ]

with open("large_file.txt", "w") as f:
    size = 0
    while size < 1024 * 15:
        if random.random() < 0.03:
            pattern = random.choice(PATTERNS)
            f.write(pattern)
            size += len(pattern)
        else:
            f.write("0")
            size += 1

