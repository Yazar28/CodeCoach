import { useQuery } from '@tanstack/react-query'
import { useParams } from 'react-router-dom'
import { getSubmission, postAnalysis } from '../api/clients'
import { useEffect, useState } from 'react'
import type { AnalysisRes } from '../types'

export default function SubmissionPage() {
  const { id } = useParams()
  const [analysis, setAnalysis] = useState<AnalysisRes | null>(null)

  const { data: sub, isLoading, isFetching } = useQuery({
    queryKey:['submission', id],
    queryFn:()=>getSubmission(id!),
    enabled: !!id,
    refetchInterval: (q) => {
      const s = q.state.data as any
      return !s || s.status !== 'done' ? 800 : false
    }
  })

  useEffect(()=> {
    async function run(){
      if (sub && sub.status === 'done' && !analysis) {
        const a = await postAnalysis({ source:'// omitido en UI', results: sub.results || [], metrics: { timeMs: sub.timeMs, memoryKB: sub.memoryKB } })
        setAnalysis(a)
      }
    }
    run()
  }, [sub, analysis])

  if (isLoading || !sub) return <p>Cargando…</p>

  return (
    <div style={{display:'grid', gap:16}}>
      <h1 style={{fontSize:22}}>Ejecución #{id}</h1>
      <div>
        <div>Estado: <b>{sub.status}</b>{isFetching && sub.status!=='done' ? ' (actualizando…)':''}</div>
        {sub.status==='done' && (
          <div style={{display:'grid', gap:8}}>
            <div>Tiempo total: {sub.timeMs} ms · Memoria: {sub.memoryKB} KB</div>
            <h3>Resultados</h3>
            <ul>
              {sub.results?.map((r,i)=>(
                <li key={i}>
                  {r.name}: {r.pass ? '✔️ PASS' : '❌ FAIL'}
                  {typeof r.timeMs==='number' ? ` · ${r.timeMs} ms` : ''}
                  {typeof r.memoryKB==='number' ? ` · ${r.memoryKB} KB` : ''}
                </li>
              ))}
            </ul>
          </div>
        )}
      </div>

      {analysis && (
        <div style={{borderTop:'1px solid #eee', paddingTop:8}}>
          <h3>Feedback del Coach</h3>
          <ul>
            {analysis.hints.map((h,i)=> (<li key={i}>{h}</li>))}
          </ul>
          {analysis.probablePatterns?.length ? (
            <div style={{fontSize:12, color:'#666'}}>Patrones detectados: {analysis.probablePatterns.join(', ')}</div>
          ) : null}
          {analysis.complexityEstimate ? (
            <div style={{fontSize:12, color:'#666'}}>Complejidad estimada: {analysis.complexityEstimate}</div>
          ) : null}
        </div>
      )}
    </div>
  )
}
