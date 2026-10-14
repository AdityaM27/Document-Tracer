#ifndef DOCUTRACE_REPORTGENERATOR_H
#define DOCUTRACE_REPORTGENERATOR_H

#include <string>
#include "Models.h"
#include "Document.h"

namespace docutrace {

/**
 * @brief Generates comprehensive HTML and JSON analysis reports.
 * Demonstrates Unit IV: Formatted File I/O Streams (std::ofstream, std::stringstream).
 */
class ReportGenerator {
private:
    std::string outputDirectory_;

    std::string escapeHtml(const std::string& data) const;
    std::string generateHighlightedTextHtml(const Document& queryDoc, const std::vector<MatchRecord>& matches) const;

public:
    explicit ReportGenerator(const std::string& outputDir = "output");
    ~ReportGenerator() = default;

    /**
     * @brief Generates JSON report file and returns the output path.
     */
    std::string generateJsonReport(const AnalysisSummary& summary) const;

    /**
     * @brief Generates standalone interactive HTML report file and returns the output path.
     */
    std::string generateHtmlReport(const Document& queryDoc, const AnalysisSummary& summary) const;

    const std::string& getOutputDirectory() const noexcept { return outputDirectory_; }
    void setOutputDirectory(const std::string& dir) { outputDirectory_ = dir; }
};

} // namespace docutrace

#endif // DOCUTRACE_REPORTGENERATOR_H
