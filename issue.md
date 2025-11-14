## Issue Title: scanning returns no matches with multi-pattern database; minimal set still matches

### Description

Compiling multiple expressions into a single database in HS_MODE_BLOCK and scanning a simple sample does not invoke the match callback, while older versions or a reduced expression set do produce matches. The behavior appears specific to multi-pattern compilation; a single expression still matches.

### Environment
- **VectorScan/Hyperscan Version**: lastest develop branch (87d8b357a98aad59e1a7d82795d6d895cbcf18ad)
    - This issue occurred after https://github.com/VectorCamp/vectorscan/commit/ca70a3d9beca61b58c6709fead60ec662482d36e
- **Operating System**: Linux Ubuntu 24.04 and Macos 26.1
- **Compiler**: g++ (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0 and Apple clang version 17.0.0 (clang-1700.4.4.1)

### Minimal Reproducible Example

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <hs/hs.h>

struct Match {
    unsigned int id;
    unsigned long long from;
    unsigned long long to;
    unsigned int flags;
};

static int onMatch(unsigned int id, unsigned long long from,
                   unsigned long long to, unsigned int flags, void *ctx) {
    auto *matches = static_cast<std::vector<Match>*>(ctx);
    matches->push_back(Match{id, from, to, flags});
    return 0;
}

int main() {
    // These patterns compile successfully but fail to match
    std::vector<std::string> patterns = {
        "<%[\\s\\S]+%>",
        "<[^<>]+:(include|forward|invoke|doBody|getProperty|setProperty|useBean|plugin|element|attribute|body|fallback|params|param|output|root)",
        "[\\$#]{[\\s\\S]+\\([\\s\\S]*\\)[\\s\\S]*}",
    };

    std::vector<const char*> exprs;
    exprs.reserve(patterns.size());
    for (const auto &s : patterns) {
        exprs.push_back(s.c_str());
    }

    std::vector<unsigned int> flags(patterns.size(), 0);
    std::vector<unsigned int> ids(patterns.size());
    for (unsigned int i = 0; i < ids.size(); ++i) ids[i] = i;

    hs_database_t *db = nullptr;
    hs_compile_error_t *compileErr = nullptr;

    hs_error_t rc = hs_compile_multi(
        exprs.data(),
        flags.data(),
        ids.data(),
        patterns.size(),
        HS_MODE_BLOCK,
        nullptr,
        &db,
        &compileErr
    );

    if (rc != HS_SUCCESS) {
        std::cerr << "Compile failed: " << (compileErr ? compileErr->message : "unknown") << "\n";
        if (compileErr) hs_free_compile_error(compileErr);
        return 1;
    }
    hs_free_compile_error(compileErr);

    hs_scratch_t *scratch = nullptr;
    rc = hs_alloc_scratch(db, &scratch);
    if (rc != HS_SUCCESS) {
        std::cerr << "Failed to allocate scratch: " << rc << "\n";
        hs_free_database(db);
        return 1;
    }

    // Test data that should match the first pattern "<%[\\s\\S]+%>"
    std::string data = "<%out.print();int a%>";

    std::vector<Match> matches;
    rc = hs_scan(db, data.c_str(), data.size(), 0, scratch, onMatch, &matches);
    
    if (rc != HS_SUCCESS) {
        std::cerr << "Scan failed with error: " << rc << "\n";
    } else {
        std::cout << "Data length: " << data.size() << ", Matches found: " << matches.size() << "\n";
        for (const auto &m : matches) {
            std::cout << "ID: " << m.id << " [" << m.from << "," << m.to << ") - " 
                      << patterns[m.id] << "\n";
        }
    }

    hs_free_scratch(scratch);
    hs_free_database(db);
    return 0;
}
```

### Demo code
https://github.com/LRainner/vectorscan-multi-match

### Expected Behavior

The test string `"<%out.print();int a%>"` should match the pattern `"<%[\\s\\S]+%>"` and return at least one match.