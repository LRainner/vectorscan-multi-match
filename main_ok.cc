#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <string.h>

#include <hs/hs.h>

struct Match {
    unsigned int id;
    unsigned long long from;
    unsigned long long to;
    unsigned int flags;
};

static int onMatch(unsigned int id,
                   unsigned long long from,
                   unsigned long long to,
                   unsigned int flags,
                   void *ctx) {
    auto *matches = static_cast<std::vector<Match>*>(ctx);
    matches->push_back(Match{id, from, to, flags});
    return 0; // Return 0 to continue scanning
}

int main() {
    // Define multiple regex patterns (PCRE subset)
    std::vector<std::string> patterns = {
        "<%[\\s\\S]+%>",
        "<[^<>]+:scriptlet",
    };

    // Convert std::string to const char* array for hs_compile_multi
    std::vector<const char*> exprs;
    exprs.reserve(patterns.size());
    for (const auto &s : patterns) {
        exprs.push_back(s.c_str());
    }

    std::vector<unsigned int> flags(patterns.size(), 0);

    // Unique ID for each pattern
    std::vector<unsigned int> ids(patterns.size());
    for (unsigned int i = 0; i < ids.size(); ++i) ids[i] = i;

    hs_database_t *db = nullptr;
    hs_compile_error_t *compileErr = nullptr;

    // Compile database in BLOCK mode
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
        std::cerr << "Compilation failed: " << (compileErr ? compileErr->message : "unknown")
                  << " (pattern index=" << (compileErr ? compileErr->expression : -1) << ")\n";
        if (compileErr) {
            hs_free_compile_error(compileErr);
        }
        return 1;
    }
    hs_free_compile_error(compileErr);

    hs_scratch_t *scratch = nullptr;
    rc = hs_alloc_scratch(db, &scratch);
    if (rc != HS_SUCCESS) {
        std::cerr << "Failed to allocate scratch, error code: " << rc << "\n";
        hs_free_database(db);
        return 1;
    }

    // get scratch size
    size_t scratch_size = 0;
    hs_scratch_size(scratch, &scratch_size);
    std::cout << "scratch size: " << scratch_size << " bytes\n";

    // get database size
    size_t db_size = 0;
    hs_database_size(db, &db_size);
    std::cout << "database size: " << db_size << " bytes\n";

    // Read input text from stdin; use the default sample if empty

    std::string data = "<%out.print();int a%>";

    std::vector<Match> matches;
    rc = hs_scan(db, data.c_str(),data.size(), 0, scratch, onMatch, &matches);
    if (rc != HS_SUCCESS) {
        std::cerr << "Scan failed, error code: " << rc << "\n";
        hs_free_scratch(scratch);
        hs_free_database(db);
        return 1;
    }

    // Print results
    std::cout << "Total length: " << data.size() << ", matches: " << matches.size() << "\n";
    for (const auto &m : matches) {
        std::string pat = patterns[m.id];
        std::cout << std::setw(6) << m.id
                  << " [" << m.from << "," << m.to << ") "
                  << "pattern=\"" << pat << "\""
                  << " text=\"" << data.substr(static_cast<size_t>(m.from),
                                              static_cast<size_t>(m.to - m.from)) << "\"\n";
    }

    hs_free_scratch(scratch);
    hs_free_database(db);
    return 0;
}