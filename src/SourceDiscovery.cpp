#include "../include/SourceDiscovery.h"
#include "../include/AcademicSourceProvider.h"
#include "../include/TextProcessor.h"
#include "../include/JaccardSimilarity.h"
#include <algorithm>

namespace docutrace {

void SourceDiscovery::addProvider(std::shared_ptr<SourceProvider> provider) {
    if (provider) {
        providers_.push_back(provider);
    }
}

std::vector<AcademicSource> SourceDiscovery::discoverSources(const Document& queryDoc, size_t maxPerQuery) {
    std::vector<AcademicSource> allSources;
    if (providers_.empty()) return allSources;

    auto queries = AcademicSourceProvider::extractSalientQueries(queryDoc.getTokens(), 3);
    if (queries.empty()) {
        // Fallback: use first few non-stopword tokens
        std::string fallbackQ;
        size_t count = 0;
        for (const auto& t : queryDoc.getTokens()) {
            if (t.text.size() > 4) {
                fallbackQ += t.text + " ";
                if (++count >= 4) break;
            }
        }
        if (!fallbackQ.empty()) queries.push_back(fallbackQ);
    }

    for (const auto& q : queries) {
        for (const auto& prov : providers_) {
            auto found = prov->search(q, maxPerQuery);
            for (auto& s : found) {
                // Deduplicate by DOI or URL or Title
                bool exists = false;
                for (const auto& existing : allSources) {
                    if ((!s.doi.empty() && s.doi == existing.doi) ||
                        (!s.url.empty() && s.url == existing.url) ||
                        (s.title == existing.title)) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    allSources.push_back(s);
                }
            }
        }
    }

    return allSources;
}

std::vector<MatchRecord> SourceDiscovery::compareAgainstSources(
    const Document& queryDoc,
    const std::vector<AcademicSource>& sources,
    double threshold) {

    std::vector<MatchRecord> matches;
    TextProcessor proc(3);
    size_t matchCounter = 1;

    const auto& qPassages = queryDoc.getPassageWindows().empty() ? queryDoc.getSentences() : queryDoc.getPassageWindows();

    for (const auto& src : sources) {
        if (src.abstractText.empty()) continue;

        Document srcDoc("ext_" + std::to_string(matchCounter), src.title);
        srcDoc.setOriginalText(src.abstractText);
        proc.processDocument(srcDoc);

        for (const auto& qp : qPassages) {
            if (qp.tokens.size() < 4) continue;

            std::unordered_set<std::string> setQ;
            for (const auto& t : qp.tokens) setQ.insert(t.text);

            for (const auto& sp : srcDoc.getSentences()) {
                if (sp.tokens.size() < 4) continue;

                std::unordered_set<std::string> setS;
                for (const auto& t : sp.tokens) setS.insert(t.text);

                double jaccard = JaccardSimilarity::computeJaccard(setQ, setS);
                if (jaccard >= threshold) {
                    MatchRecord rec;
                    rec.id = "academic_match_" + std::to_string(matchCounter++);
                    rec.queryDocId = queryDoc.getId();
                    rec.targetDocId = src.doi.empty() ? src.title : src.doi;
                    rec.targetDocName = src.title + " (" + src.provider + ")";
                    rec.targetSourceType = "ACADEMIC";
                    rec.matchType = MatchType::JACCARD_MATCH;
                    rec.strategyName = "Academic Source Verification";
                    rec.score = jaccard;
                    rec.queryPassage = qp;
                    rec.targetPassage = sp;
                    rec.reviewStatus = (jaccard >= 0.70) ? "SUBSTANTIAL_OVERLAP" : "POTENTIAL_PARAPHRASE";

                    for (const auto& w : setQ) {
                        if (setS.find(w) != setS.end()) rec.matchedTokens.push_back(w);
                    }

                    matches.push_back(rec);
                }
            }
        }
    }

    return matches;
}

} // namespace docutrace
