export interface Token {
  text: string;
  originalText: string;
  startChar: number;
  endChar: number;
  sentenceIndex: number;
  paragraphIndex: number;
}

export interface Passage {
  passageIndex: number;
  startChar: number;
  endChar: number;
  originalText: string;
  normalizedText: string;
  ngrams?: string[];
  tokens?: Token[];
}

export interface MatchRecord {
  id: string;
  queryDocId: string;
  targetDocId: string;
  targetDocName: string;
  targetSourceType: string;
  matchType: 'EXACT_MATCH' | 'NGRAM_MATCH' | 'JACCARD_MATCH' | 'SEMANTIC_MATCH';
  strategyName: string;
  score: number;
  queryPassage: Passage;
  targetPassage: Passage;
  matchedTokens: string[];
  reviewStatus: string;
}

export interface AcademicSource {
  title: string;
  authors: string[];
  publicationDate: string;
  doi: string;
  url: string;
  abstractText: string;
  provider: string;
  retrievalStatus: string;
  relevanceScore?: number;
}

export interface WritingPatternStats {
  totalWords: number;
  uniqueWords: number;
  typeTokenRatio: number;
  rootTTR: number;
  totalSentences: number;
  avgSentenceLength: number;
  sentenceLengthVariance: number;
  sentenceLengthStdDev: number;
  burstiness: number;
  characterEntropy: number;
  transitionPhraseFrequencies: Record<string, number>;
  repeatedPhrases: string[];
  interpretation: string;
}

export interface AIClassificationResult {
  serviceAvailable: boolean;
  modelName: string;
  label: string;
  aiProbability: number;
  humanProbability: number;
  confidence: number;
  reasoning: string;
  disclaimer: string;
  isPretrainedModel?: boolean;
  inferenceMode?: string;
}

export interface AnalysisSummary {
  submissionId: string;
  submissionFilename: string;
  timestamp: string;
  overallSimilarityScore: number;
  exactSimilarityScore: number;
  ngramSimilarityScore: number;
  jaccardSimilarityScore: number;
  semanticSimilarityScore: number;
  referencesCheckedCount: number;
  totalPassagesCount: number;
  matchedPassagesCount: number;
  matches: MatchRecord[];
  discoveredSources: AcademicSource[];
  writingStats: WritingPatternStats;
  aiResult: AIClassificationResult;
  errorsAndWarnings: string[];
  processingTimeMs: number;
}

export interface AnalysisOptions {
  enableExactMatch: boolean;
  enableNGram: boolean;
  enableJaccard: boolean;
  enableSemantic: boolean;
  enableAcademicSearch: boolean;
  enableWebSearch?: boolean;
  enableAIClassifier: boolean;
  enableWritingPatterns: boolean;
  ngramSize: number;
  exactThreshold: number;
  ngramThreshold: number;
  jaccardThreshold: number;
  semanticThreshold: number;
  maxConcurrentThreads: number;
}

export interface AnalysisProgress {
  submissionId: string;
  stage: string;
  percent: number;
  message: string;
  isCompleted: boolean;
  hasError: boolean;
  errorMessage?: string;
}

export interface ReferenceDoc {
  id: string;
  filename: string;
  wordCount: number;
  characterCount: number;
  fileSize: number;
  sentencesCount: number;
}

export interface ReportItem {
  id: string;
  filename: string;
  timestamp: string;
  overallSimilarity: number;
  aiLabel: string;
  jsonPath: string;
  htmlPath: string;
}

export interface SystemHealth {
  status: string;
  version: string;
  cppCore: string;
  referencesIndexed: number;
  academicProviderOnline: boolean;
  aiServiceOnline: boolean;
  aiModelName?: string;
  aiHfLoaded?: boolean;
  aiMode?: string;
}

export interface SelfTestCase {
  id: string;
  name: string;
  description: string;
  expected: string;
  passed: boolean;
  status: 'PASS' | 'FAIL' | 'FALLBACK (STATISTICAL MODE)' | 'OFFLINE' | string;
  overallScore?: number;
  exactScore?: number;
  ngramScore?: number;
  jaccardScore?: number;
  semanticScore?: number;
  matchesFound?: number;
  details?: string;
  error?: string;
  online?: boolean;
  modelName?: string;
  isPretrainedModel?: boolean;
  mode?: string;
}

export interface SelfTestReport {
  timestamp: number;
  elapsedMs: number;
  totalTests: number;
  passedCount: number;
  failedCount: number;
  overallStatus: 'PASS' | 'ATTENTION_REQUIRED';
  tests: SelfTestCase[];
  aiServiceAudit: {
    online: boolean;
    modelName: string;
    isPretrainedModel: boolean;
    mode: string;
  };
}
