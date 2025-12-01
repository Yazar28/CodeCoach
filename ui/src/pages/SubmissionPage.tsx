import { useParams, useNavigate } from 'react-router-dom'
import { useQuery } from '@tanstack/react-query'
import { getSubmission, analyzeSolution } from '../api/clients'
import { useEffect, useState } from 'react'

type AnalysisRes = {
  hints: string[];
  probablePatterns?: string[];
  complexityEstimate?: string;
};

export default function SubmissionPage() {
  const { id } = useParams()
  const nav = useNavigate()

  const [analysis, setAnalysis] = useState<AnalysisRes | null>(null)

  // Obtener problemId de la URL
  const urlParams = new URLSearchParams(window.location.search);
  const problemId = urlParams.get('problemId') || 'unknown';

  const { data: sub, isLoading, isFetching } = useQuery({
    queryKey: ['submission', id],
    queryFn: () => getSubmission(id!),
    enabled: !!id,
    refetchInterval: (q) => {
      const s = q.state.data as any
      return !s || s.status !== 'done' ? 2000 : false
    }
  })

  // Cuando el Evaluator termina (status === 'done'), disparamos el Analyzer (IA)
  useEffect(() => {
    async function run() {
      if (!sub) return
      if (sub.status !== 'done') return
      if (analysis) return   // ya tenemos feedback, no repetir

      try {
        const a = await analyzeSolution({
          source: '// código del usuario',
          results: sub,
          problemId: problemId
        })
        setAnalysis(a)
      } catch (err) {
        console.error('Error llamando al Analyzer:', err)
      }
    }
    run()
  }, [sub, analysis, problemId])

  if (isLoading || !sub) {
    return <p style={{ padding: 16 }}>Cargando…</p>
  }

  return (
    <div style={{ display: 'grid', gap: 16, padding: 16 }}>
      <h1 style={{ fontSize: 22 }}>Ejecución #{id}</h1>

      <div>
        <div>
          Estado: <b>{sub.status}</b>
          {isFetching && sub.status !== 'done' ? ' (actualizando…)' : ''}
        </div>

        {sub.status === 'done' && (
          <div style={{ display: 'grid', gap: 8 }}>
            <div>Tiempo total: {sub.timeMs} ms · Memoria: {sub.memoryKB} KB</div>

            <h3>Resultados por caso</h3>
            <ul>
              {sub.results?.map((r, i) => (
                <li key={i}>
                  Caso {typeof r.case === 'number' ? r.case : i + 1}: {r.pass ? '✔️ PASS' : '❌ FAIL'}
                  {typeof r.timeMs === 'number' ? ` · ${r.timeMs} ms` : ''}
                  {r.stdout ? ` · out: ${r.stdout}` : ''}
                </li>
              ))}
            </ul>
          </div>
        )}
      </div>

      <div style={{ borderTop: '1px solid #eee', paddingTop: 8 }}>
        <h3>Feedback del Coach</h3>

        {/* Mientras NO tengamos análisis, mostramos siempre este mensaje */}
        {!analysis && (
          <p style={{ fontSize: 14, color: '#666' }}>
            Preparando feedback del Coach (IA)…
          </p>
        )}

        {/* Cuando ya hay análisis, mostramos las pistas y la info extra */}
        {analysis && (
          <>
            <ul>
              {analysis.hints.map((h, i) => (
                <li key={i}>{h}</li>
              ))}
            </ul>
          </>
        )}
      </div>
    </div>
  )
}
