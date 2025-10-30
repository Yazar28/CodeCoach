// ui/src/api/clients.ts
// Cliente de APIs + toggles de mock por servicio

import axios from "axios";

/** ========= Toggles por servicio ========= */
const USE_MOCK_PM   = false; // ← PM real (ya lo tenemos en C++)
const USE_MOCK_EVAL = false;  // ← por ahora mock
const USE_MOCK_ANA  = true;  // ← por ahora mock

/** ========= Axios por servicio ========= */
export const pm = axios.create({
  baseURL: import.meta.env.VITE_API_PM || "http://localhost:8081",
});

const evalApi = axios.create({
  baseURL: import.meta.env.VITE_API_EVAL || "http://localhost:8082",
});

const anaApi = axios.create({
  baseURL: import.meta.env.VITE_API_ANALYZER || "http://localhost:8083",
});

/** ========= Tipos mínimos ========= */
type Difficulty = "easy" | "medium" | "hard";

export interface ProblemSummary {
  id: string;
  title: string;
  difficulty: Difficulty;
  tags: string[];
}

export interface Example {
  in: any;  // para MVP mantenemos flexible
  out: any;
}

export interface ProblemDetail extends ProblemSummary {
  statement: string;
  examples: Example[];
}

export interface SubmissionPayload {
  problemId: string;
  lang: "cpp" | "python" | "java";
  source: string;
}

export interface SubmissionCreated {
  submissionId: string;
}

export type RunStatus = "queued" | "running" | "done" | "error";

export interface SubmissionResult {
  status: RunStatus;
  results?: Array<{
    case: number;
    pass: boolean;
    stdout?: string;
    stderr?: string;
    timeMs?: number;
  }>;
  timeMs?: number;
  memoryKB?: number;
  compileErrors?: string;
  note?: string; 
}

export interface AnalysisPayload {
  source: string;
  results?: SubmissionResult;
}

export interface AnalysisResult {
  hints: string[];
  probablePatterns?: string[];
  complexityEstimate?: string;
}

/** ========= Mocks ========= */
const MOCK_PROBLEMS: ProblemSummary[] = [
  { id: "two-sum", title: "Two Sum", difficulty: "easy", tags: ["array", "hashmap"] },
];

const MOCK_PROBLEM_DETAIL: Record<string, ProblemDetail> = {
  "two-sum": {
    id: "two-sum",
    title: "Two Sum",
    difficulty: "easy",
    tags: ["array", "hashmap"],
    statement:
      "Dado un arreglo nums y un entero target, retorna índices i y j tales que nums[i] + nums[j] = target. " +
      "Asume una única solución y no reutilices el mismo elemento.",
    examples: [
      { in: { nums: [2, 7, 11, 15], target: 9 }, out: [0, 1] },
      { in: { nums: [3, 2, 4], target: 6 }, out: [1, 2] },
    ],
  },
};

const MOCK_SUBMISSIONS: Record<string, SubmissionResult> = {};

function createMockSubmission(): SubmissionCreated {
  const id = `mock-${Math.random().toString(36).slice(2, 8)}`;
  // ciclo de vida simulado
  MOCK_SUBMISSIONS[id] = { status: "queued" };
  setTimeout(() => (MOCK_SUBMISSIONS[id] = { status: "running" }), 400);
  setTimeout(() => {
    MOCK_SUBMISSIONS[id] = {
      status: "done",
      timeMs: 12,
      memoryKB: 256,
      results: [
        { case: 1, pass: true, timeMs: 5, stdout: "[0,1]" },
        { case: 2, pass: true, timeMs: 7, stdout: "[1,2]" },
      ],
    };
  }, 1200);
  return { submissionId: id };
}

/** ========= APIs: Problem Manager ========= */

export async function listProblems(): Promise<ProblemSummary[]> {
  if (USE_MOCK_PM) {
    return Promise.resolve(MOCK_PROBLEMS);
  }
  const { data } = await pm.get<ProblemSummary[]>("/problems");
  return data;
}

export async function getProblem(id: string): Promise<ProblemDetail> {
  if (USE_MOCK_PM) {
    const p = MOCK_PROBLEM_DETAIL[id];
    if (!p) throw new Error("Problem not found (mock)");
    return Promise.resolve(p);
  }
  const { data } = await pm.get<ProblemDetail>(`/problems/${id}`);
  return data;
}

/** ========= APIs: Evaluator ========= */

export async function submitSolution(
  payload: SubmissionPayload
): Promise<SubmissionCreated> {
  if (USE_MOCK_EVAL) {
    return new Promise((r) => setTimeout(() => r(createMockSubmission()), 250));
  }
  const { data } = await evalApi.post<SubmissionCreated>("/submissions", payload);
  return data;
}

export async function getSubmission(id: string): Promise<SubmissionResult> {
  if (USE_MOCK_EVAL) {
    // si aún no existe, seguiremos marcando "queued"
    return new Promise((r) => setTimeout(() => r(MOCK_SUBMISSIONS[id] ?? { status: "queued" }), 250));
  }
  const { data } = await evalApi.get<SubmissionResult>(`/submissions/${id}`);
  return data;
}

/** ========= APIs: Analyzer ========= */

export async function analyzeSolution(
  payload: AnalysisPayload
): Promise<AnalysisResult> {
  if (USE_MOCK_ANA) {
    return new Promise((r) =>
      setTimeout(
        () =>
          r({
            hints: [
              "Piensa en un mapa de valor→índice para acelerar la búsqueda.",
              "Cuidado con reutilizar el mismo índice dos veces.",
              "La complejidad esperada es O(n) con espacio adicional O(n).",
            ],
            probablePatterns: ["hashmap", "two-pointers?"],
            complexityEstimate: "O(n)",
          }),
        300
      )
    );
  }
  const { data } = await anaApi.post<AnalysisResult>("/analysis", payload);
  return data;
}
