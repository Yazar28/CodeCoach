export type Difficulty = 'easy' | 'medium' | 'hard'

export interface ProblemSummary {
  id: string
  title: string
  difficulty: Difficulty
  tags: string[]
}

export interface ExampleIO { in: any; out: any }

export interface Problem extends ProblemSummary {
  statement: string
  examples: ExampleIO[]
  constraints?: Record<string, any>
  tests: ExampleIO[]           // (ocultos al usuario; los usa el evaluador)
}

export interface PostSubmissionReq { problemId: string; lang: 'cpp'|'python'|'java'; source: string }
export interface PostSubmissionRes { submissionId: string }

export interface TestResult { name: string; pass: boolean; timeMs?: number; memoryKB?: number; stderr?: string }
export interface SubmissionStatus {
  status: 'queued' | 'running' | 'done' | 'error'
  results?: TestResult[]
  timeMs?: number
  memoryKB?: number
  compileErrors?: string
}

export interface AnalysisReq { source: string; results: TestResult[]; metrics?: { timeMs?: number; memoryKB?: number } }
export interface AnalysisRes { hints: string[]; probablePatterns?: string[]; complexityEstimate?: string }
