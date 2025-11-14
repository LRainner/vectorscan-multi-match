## Issue Title: scanning returns no matches with multi-pattern database; minimal set still matches

### Description

Compiling multiple expressions into a single database in HS_MODE_BLOCK and scanning a simple sample does not invoke the match callback, while older versions or a reduced expression set do produce matches. The behavior appears specific to multi-pattern compilation; a single expression still matches.

### Environment
- **VectorScan/Hyperscan Version**: lastest develop branch (87d8b357a98aad59e1a7d82795d6d895cbcf18ad)
    - This issue occurred after https://github.com/VectorCamp/vectorscan/commit/ca70a3d9beca61b58c6709fead60ec662482d36e
- **Operating System**: Linux Ubuntu 24.04 and Macos 26.1
- **Compiler**: g++ (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0 and Apple clang version 17.0.0 (clang-1700.4.4.1)

### Reproduction (key code only)

1) Patterns and test data
```cpp
// Problem set
std::vector<std::string> patterns = {
    "<%[\\s\\S]+%>",
    "<[^<>]+:(include|forward|invoke|doBody|getProperty|setProperty|useBean|plugin|element|attribute|body|fallback|params|param|output|root)",
    "[\\$#]{[\\s\\S]+\\([\\s\\S]*\\)[\\s\\S]*}"
};

// Minimal working set (for comparison)
// std::vector<std::string> patterns = {
//     "<%[\\s\\S]+%>",
//     "<[^<>]+:scriptlet>"
// };

std::string data = "<%out.print();int a%>";
```

2) Compile and scan (core API calls)
```cpp
// Prepare inputs
std::vector<const char*> exprs; for (auto &s : patterns) exprs.push_back(s.c_str());
std::vector<unsigned int> flags(patterns.size(), 0);
std::vector<unsigned int> ids(patterns.size()); for (unsigned i=0;i<ids.size();++i) ids[i]=i;

hs_database_t *db = nullptr; hs_compile_error_t *err = nullptr;
hs_error_t rc = hs_compile_multi(exprs.data(), flags.data(), ids.data(),
                                 patterns.size(), HS_MODE_BLOCK, nullptr, &db, &err);

hs_scratch_t *scratch = nullptr;
rc = hs_alloc_scratch(db, &scratch);

std::vector<Match> matches;
rc = hs_scan(db, data.c_str(), data.size(), 0, scratch, onMatch, &matches);
```

3) Match callback skeleton
```cpp
static int onMatch(unsigned int id, unsigned long long from, unsigned long long to,
                   unsigned int flags, void *ctx) {
    auto *out = static_cast<std::vector<Match>*>(ctx);
    out->push_back({id, from, to, flags});
    return 0;
}
```

### Demo code
https://github.com/LRainner/vectorscan-multi-match

### Expected Behavior

The test string `"<%out.print();int a%>"` should match the pattern `"<%[\\s\\S]+%>"` and return at least one match.