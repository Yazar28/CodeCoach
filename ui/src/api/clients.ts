import axios from 'axios'
import type {
  Problem, ProblemSummary, PostSubmissionReq, PostSubmissionRes,
  SubmissionStatus, AnalysisReq, AnalysisRes
} from '../types'

// Cambia a false cuando conectes con tu backend real
const USE_MOCK = true

const PM_BASE   = import.meta.env.VITE_API_PM   || 'http://localhost:8081'
const EVAL_BASE = import.meta.env.VITE_API_EVAL || 'http://localhost:8082'
const ANA_BASE  = import.meta.env.VITE_API_ANALYZER || 'http://localhost:8083'

const pm = axios.create({ baseURL: PM_BASE })
const ev = axios.create({ baseURL: EVAL_BASE })
const an = axios.create({ baseURL: ANA_BASE })

// ── MOCK DATA ───────────────────────────────────────────────────────────────
const MOCK_PROBLEMS: Problem[] = [
  {
    id: 'two-sum',
    title: 'Two Sum',
    difficulty: 'easy',
    tags: ['array','hashmap'],
    statement: 'Dado un arreglo nums y un entero target, retorna índices i y j tales que nums[i] + nums[j] = target. Asume una única solución y no reutilices el mismo elemento.',
    examples: [
      { in:{ nums:[2,7,11,15], target:9 }, out:[0,1] },
      { in:{ nums:[3,2,4], target:6 }, out:[1,2] }
    ],
    constraints: { nMax: 1e5, valuesRange: [-1e9, 1e9] },
    tests: [
      { in:{ nums:[2,7,11,15], target:9 }, out:[0,1] },
      { in:{ nums:[3,2,4], target:6 }, out:[1,2] },
      { in:{ nums:[3,3], target:6 }, out:[0,1] }
    ]
  }
]
function delay<T>(data:T, ms=600){ return new Promise<T>(res=>setTimeout(()=>res(data), ms)) }

// Problem Manager
export async function listProblems(): Promise<ProblemSummary[]> {
  if (USE_MOCK) return delay(MOCK_PROBLEMS.map(p=>({ id:p.id, title:p.title, difficulty:p.difficulty, tags:p.tags })))
  const { data } = await pm.get('/problems'); return data
}
export async function getProblem(id: string): Promise<Problem> {
  if (USE_MOCK) {
    const p = MOCK_PROBLEMS.find(x=>x.id===id); if(!p) throw new Error('Problem not found')
    return delay(p)
  }
  const { data } = await pm.get(`/problems/${id}`); return data
}

// Evaluator
export async function postSubmission(req: PostSubmissionReq): Promise<PostSubmissionRes> {
  if (USE_MOCK) {
    const submissionId = Math.random().toString(36).slice(2)
    sessionStorage.setItem(`mock-sub-${submissionId}`, JSON.stringify({ started: Date.now(), req }))
    return delay({ submissionId }, 200)
  }
  const { data } = await ev.post('/submissions', req); return data
}
export async function getSubmission(id: string): Promise<SubmissionStatus> {
  if (USE_MOCK) {
    const meta = sessionStorage.getItem(`mock-sub-${id}`)
    if (!meta) return delay({ status:'error' as const })
    const started = JSON.parse(meta).started as number
    const elapsed = Date.now() - started
    if (elapsed < 800) return delay({ status:'queued' })
    if (elapsed < 1600) return delay({ status:'running' })
    return delay({
      status: 'done',
      results: [
        { name:'sample #1', pass:true, timeMs: 11, memoryKB: 980 },
        { name:'sample #2', pass:true, timeMs: 10, memoryKB: 1000 }
      ],
      timeMs: 21, memoryKB: 1980
    })
  }
  const { data } = await ev.get(`/submissions/${id}`); return data
}

// Analyzer (LLM)
export async function postAnalysis(body: AnalysisReq): Promise<AnalysisRes> {
  if (USE_MOCK) {
    const allPass = body.results.every(r=>r.pass)
    return delay({
      hints: allPass
        ? ['¡Bien! Considera usar una estructura O(1) para búsquedas y analiza la complejidad.']
        : ['Revisa índices y casos con duplicados.', 'Valida entradas vacías.'],
      probablePatterns: ['HashMap lookup'],
      complexityEstimate: 'O(n) tiempo, O(n) espacio'
    })
  }
  const { data } = await an.post('/analysis', body); return data
}
