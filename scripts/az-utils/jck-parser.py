# Here’s a Python parser that takes a JCK fails.json file and cross-references it with KFL, JTX, batchExclude, and flaky test lists to categorize each failure as you requested.

# Assumptions:
# - `fails.json` is a JSON array or object with test names as keys or values.
# - KFL and JTX are plain text files, one test per line (ignoring comments and blank lines).
# - batchExcludeList.json is a JSON array with either `test` or `keywords` fields.
# - flakytestlists is a plain text file, one pattern per line.

# **How to use:**
# 1. Place this script in your workspace.
# 2. Adjust file paths at the top of `main()` if needed.
# 3. Run with:  
#    ```sh
#    python3 scripts/jck_failure_categorizer.py
#    ```
# 4. The output will be a table listing each failed test and its category (`kfl`, `jtx`, `batchExclude`, `flaky`, or `new`).

# Let me know if you need it adapted for your specific fails.json structure!

import json
import re

def load_list_from_file(filepath):
    with open(filepath) as f:
        return [line.strip().split()[0] for line in f if line.strip() and not line.startswith('#')]

def load_batch_excludes(filepath):
    with open(filepath) as f:
        data = json.load(f)
    tests = set()
    keywords = []
    for entry in data:
        if 'test' in entry:
            tests.add(entry['test'])
        if 'keywords' in entry:
            keywords.append(entry['keywords'])
    return tests, keywords

def load_flaky_patterns(filepath):
    with open(filepath) as f:
        return [line.strip() for line in f if line.strip() and not line.startswith('#')]

def categorize(test, kfl, jtx, batch_tests, batch_keywords, flaky_patterns):
    if any(test == k or test.startswith(k) for k in kfl):
        return 'kfl'
    if any(test == j or test.startswith(j) for j in jtx):
        return 'jtx'
    if test in batch_tests or any(kw in test for kw in batch_keywords):
        return 'batchExclude'
    if any(re.match(fnmatch_to_regex(pat), test) for pat in flaky_patterns):
        return 'flaky'
    return 'new'

def fnmatch_to_regex(pattern):
    # Convert simple glob to regex for matching
    return '^' + re.escape(pattern).replace(r'\*', '.*') + '.*$'

def main():
    # File paths (edit as needed)
    fails_json = 'fails.json'
    kfl_file = 'jck25.kfl'
    jtx_file = 'jck25.jtx'
    batch_exclude_file = 'batchExcludeList.json'
    flaky_file = 'flakytestlists'

    # Load data
    with open(fails_json) as f:
        fails = json.load(f)
    # Support both list and dict formats
    if isinstance(fails, dict):
        fail_tests = list(fails.keys())
    else:
        fail_tests = fails

    kfl = load_list_from_file(kfl_file)
    jtx = load_list_from_file(jtx_file)
    batch_tests, batch_keywords = load_batch_excludes(batch_exclude_file)
    flaky_patterns = load_flaky_patterns(flaky_file)

    # Categorize
    results = []
    for test in fail_tests:
        category = categorize(test, kfl, jtx, batch_tests, batch_keywords, flaky_patterns)
        results.append({'test': test, 'category': category})

    # Print results as a table
    print(f"{'Test Name':80} | Category")
    print('-'*100)
    for r in results:
        print(f"{r['test']:80} | {r['category']}")

if __name__ == '__main__':
    main()

