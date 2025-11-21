// ui/src/types.ts

// Dificultad de los problemas
export type Difficulty = 'easy' | 'medium' | 'hard'

// Resumen que se usa en la lista de problemas
export interface ProblemSummary {
  id: string
  title: string
  difficulty: Difficulty
  tags: string[]
}

// Entrada/salida de ejemplos y tests
export interface ExampleIO {
  in: any
  out: any
}

// Problema completo que viene del Problem Manager (PM)
export interface Problem extends ProblemSummary {
  statement: string
  examples: ExampleIO[]
  constraints?: Record<string, any>
  starterCode?: string          // código base para el editor
  tests: ExampleIO[]            // casos que luego usará el Evaluator
}

// ======= Evaluator / Submissions =======

// Lo que enviamos al Evaluator al hacer "Enviar"
export interface PostSubmissionReq {
  problemId: string
  // por ahora solo C++ está soportado
  lang: 'cpp'
  source: string
}

export interface PostSubmissionRes {
  submissionId: string
}

// Resultado de cada caso de prueba que devuelve el Evaluator
export interface EvalCaseResult {
  case: number          // 1, 2, ...
  pass: boolean
  stdout?: string
  timeMs?: number
}

// Estado completo de una ejecución (/submissions/:id)
export interface SubmissionStatus {
  status: 'queued' | 'running' | 'done'
  results?: EvalCaseResult[]
  timeMs?: number
  memoryKB?: number
  note?: string          // mensajes de error, compilación, etc.
}

// ======= Analyzer / LLM Coach =======

// Lo que le mandamos al Analyzer
export interface AnalysisReq {
  source: string               // código del usuario (o un resumen)
  results: SubmissionStatus    // resultado que viene del Evaluator
  problemId: string
}

// Lo que devuelve el Analyzer
export interface AnalysisRes {
  hints: string[]
  probablePatterns?: string[]
  complexityEstimate?: string
}

export interface CreateProblemReq {
  id: string
  title: string
  difficulty: Difficulty
  tags: string[]
  statement: string
  examples: ExampleIO[]
  starterCode?: string
  tests: ExampleIO[]
}