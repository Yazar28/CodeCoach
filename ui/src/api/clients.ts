// ui/src/api/clients.ts
import type {
  ProblemSummary,
  Problem,
  PostSubmissionReq,
  PostSubmissionRes,
  SubmissionStatus,
  AnalysisReq,
  AnalysisRes,
  CreateProblemReq, 
} from '../types'


// URLs base de cada microservicio
const PM_BASE = 'http://localhost:8084' // Problem Manager (Python + Mongo)
const EV_BASE = 'http://localhost:8082' // Evaluator (C++)
const AN_BASE = 'http://localhost:8083' // Analyzer (C++)

// Helper genérico para hacer fetch y parsear JSON tipado
async function jsonFetch<T>(url: string, options?: RequestInit): Promise<T> {
  const res = await fetch(url, {
    ...options,
    headers: {
      'Content-Type': 'application/json',
      ...(options?.headers || {}),
    },
  })

  if (!res.ok) {
    const text = await res.text().catch(() => '')
    throw new Error(`Error HTTP ${res.status} en ${url}: ${text}`)
  }

  return (await res.json()) as T
}

// =========== PROBLEM MANAGER ===========

// Lista de problemas (se usa en ProblemsPage)
export async function listProblems(): Promise<ProblemSummary[]> {
  return jsonFetch<ProblemSummary[]>(`${PM_BASE}/problems`)
}

// Problema completo por id (se usa en ProblemDetailPage)
export async function getProblem(id: string): Promise<Problem> {
  return jsonFetch<Problem>(`${PM_BASE}/problems/${id}`)
}

// Crear un problema nuevo (modo admin)
export async function createProblem(
  body: CreateProblemReq
): Promise<{ id: string }> {
  return jsonFetch<{ id: string }>(`${PM_BASE}/problems`, {
    method: 'POST',
    body: JSON.stringify(body),
  })
}
// =========== EVALUATOR ===========

// Crear una nueva ejecución de código
export async function submitSolution(
  body: PostSubmissionReq
): Promise<PostSubmissionRes> {
  return jsonFetch<PostSubmissionRes>(`${EV_BASE}/submissions`, {
    method: 'POST',
    body: JSON.stringify(body),
  })
}

// Consultar estado de una ejecución
export async function getSubmission(id: string): Promise<SubmissionStatus> {
  return jsonFetch<SubmissionStatus>(`${EV_BASE}/submissions/${id}`)
}

// =========== ANALYZER / COACH ===========

// Enviar resultados + código al Analyzer
export async function analyzeSolution(
  body: AnalysisReq
): Promise<AnalysisRes> {
  return jsonFetch<AnalysisRes>(`${AN_BASE}/analysis`, {
    method: 'POST',
    body: JSON.stringify(body),
  })
}

// Re-export de tipos útiles para que puedas hacer:
//   import { listProblems, type ProblemSummary } from '../api/clients'
export type {
  ProblemSummary,
  Problem,
  SubmissionStatus,
  AnalysisRes,
}
