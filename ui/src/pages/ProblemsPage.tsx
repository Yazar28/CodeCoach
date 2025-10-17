import { useQuery } from '@tanstack/react-query'
import { listProblems } from '../api/clients'
import { Link } from 'react-router-dom'

export default function ProblemsPage() {
  const { data, isLoading, error } = useQuery({ queryKey:['problems'], queryFn:listProblems })

  if (isLoading) return <p>Cargando…</p>
  if (error)     return <p>Error al cargar problemas</p>

  return (
    <div>
      <h1 style={{fontSize:24, marginBottom:8}}>Problemas</h1>
      <ul style={{display:'grid', gap:12}}>
        {data!.map(p=>(
          <li key={p.id} style={{border:'1px solid #eee', borderRadius:8, padding:12}}>
            <div style={{display:'flex', justifyContent:'space-between', gap:12}}>
              <div>
                <Link to={`/problems/${p.id}`} style={{fontWeight:600}}>{p.title}</Link>
                <div style={{fontSize:12, color:'#666'}}>Dificultad: {p.difficulty}</div>
              </div>
              <div style={{display:'flex', gap:6, flexWrap:'wrap'}}>
                {p.tags.map(t=>(
                  <span key={t} style={{fontSize:12, border:'1px solid #ddd', borderRadius:12, padding:'2px 8px'}}>{t}</span>
                ))}
              </div>
            </div>
          </li>
        ))}
      </ul>
    </div>
  )
}
