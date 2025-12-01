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
const AN_BASE = 'http://localhost:8083' // Analyzer (C++ que llama al LLM)

// Helper genérico para llamadas JSON
async function jsonFetch<T>(url: string, init?: RequestInit): Promise<T> {
  const res = await fetch(url, {
    headers: {
      'Content-Type': 'application/json',
      ...(init && init.headers ? init.headers : {}),
    },
    ...init,
  })

  if (!res.ok) {
    const text = await res.text()
    throw new Error(`Error HTTP ${res.status} al llamar ${url}: ${text}`)
  }

  if (res.status === 204) {
    return undefined as unknown as T
  }

  return (await res.json()) as T
}

// =====================
//  Problem Manager (PM)
// =====================

export async function listProblems(): Promise<ProblemSummary[]> {
  return jsonFetch<ProblemSummary[]>(`${PM_BASE}/problems`)
}

export async function getProblem(id: string): Promise<Problem> {
  return jsonFetch<Problem>(`${PM_BASE}/problems/${encodeURIComponent(id)}`)
}

export async function createProblem(body: CreateProblemReq): Promise<{ id: string }> {
  return jsonFetch<{ id: string }>(`${PM_BASE}/problems`, {
    method: 'POST',
    body: JSON.stringify(body),
  })
}

export async function deleteProblem(id: string): Promise<void> {
  await jsonFetch<unknown>(`${PM_BASE}/problems/${encodeURIComponent(id)}`, {
    method: 'DELETE',
  })
}

// =================
//  Evaluator (EV)
// =================

export async function submitSolution(body: PostSubmissionReq): Promise<PostSubmissionRes> {
  return jsonFetch<PostSubmissionRes>(`${EV_BASE}/submissions`, {
    method: 'POST',
    body: JSON.stringify(body),
  })
}

export async function getSubmission(id: string): Promise<SubmissionStatus> {
  return jsonFetch<SubmissionStatus>(`${EV_BASE}/submissions/${encodeURIComponent(id)}`)
}

// =================
//  Analyzer (AN)
// =================

export async function analyzeSolution(body: AnalysisReq): Promise<AnalysisRes> {
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
