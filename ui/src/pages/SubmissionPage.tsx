// ui/src/pages/SubmissionPage.tsx
import { useQuery } from '@tanstack/react-query'
import { useParams, useNavigate } from 'react-router-dom'
import { getSubmission, analyzeSolution } from '../api/clients'
import { useEffect, useState } from 'react'

// Opcional: define el shape del análisis si no lo tienes en ../types
type AnalysisRes = {
  hints: string[];
  probablePatterns?: string[];
  complexityEstimate?: string;
};

export default function SubmissionPage() {
  const { id } = useParams()
  const nav = useNavigate()
  const [analysis, setAnalysis] = useState<AnalysisRes | null>(null)

  const { data: sub, isLoading, isFetching } = useQuery({
    queryKey: ['submission', id],
    queryFn: () => getSubmission(id!),
    enabled: !!id,
    refetchInterval: (q) => {
      const s = q.state.data as any
      return !s || s.status !== 'done' ? 800 : false
    }
  })

  useEffect(() => {
    async function run() {
      if (sub && sub.status === 'done' && !analysis) {
        // Enviamos el objeto completo de la submission (coincide con clients.ts)
        const a = await analyzeSolution({
          source: '// omitido en UI',
          results: sub
        })
        setAnalysis(a)
      }
    }
    run()
  }, [sub, analysis])

  if (isLoading || !sub) return <p style={{padding:16}}>Cargando…</p>

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
                  {/* r.name no existe; usamos el índice/case */}
                  Caso {typeof r.case === 'number' ? r.case : i + 1}: {r.pass ? '✔️ PASS' : '❌ FAIL'}
                  {typeof r.timeMs === 'number' ? ` · ${r.timeMs} ms` : ''}
                  {/* memoryKB no existe por-caso; si quieres mostrar stdout: */}
                  {r.stdout ? ` · out: ${r.stdout}` : ''}
                </li>
              ))}
            </ul>
          </div>
        )}
      </div>

      {analysis && (
        <div style={{ borderTop: '1px solid #eee', paddingTop: 8 }}>
          <h3>Feedback del Coach</h3>
          <ul>
            {analysis.hints.map((h, i) => (<li key={i}>{h}</li>))}
          </ul>
          {analysis.probablePatterns?.length ? (
            <div style={{ fontSize: 12, color: '#666' }}>
              Patrones detectados: {analysis.probablePatterns.join(', ')}
            </div>
          ) : null}
          {analysis.complexityEstimate ? (
            <div style={{ fontSize: 12, color: '#666' }}>
              Complejidad estimada: {analysis.complexityEstimate}
            </div>
          ) : null}
        </div>
      )}
    </div>
  )
}
