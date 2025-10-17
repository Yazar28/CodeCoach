import { useMutation, useQuery } from '@tanstack/react-query'
import { useParams, useNavigate } from 'react-router-dom'
import { getProblem, postSubmission } from '../api/clients'
import { useState } from 'react'
import CodeEditor from '../components/CodeEditor'

const DEFAULT_SNIPPETS: Record<'cpp'|'python'|'java', string> = {
  cpp: `#include <bits/stdc++.h>
using namespace std;
int main(){
  ios::sync_with_stdio(false);cin.tie(nullptr);
  // TODO: implementa tu solución
  return 0;
}
`,
  python: `# Escribe tu solución aquí
if __name__ == "__main__":
  pass
`,
  java: `import java.io.*; import java.util.*;
public class Main {
  public static void main(String[] args) throws Exception {
    // TODO: tu solución
  }
}
`
}

export default function ProblemDetailPage() {
  const { id } = useParams()
  const nav = useNavigate()
  const { data: problem, isLoading } = useQuery({ queryKey:['problem', id], queryFn:()=>getProblem(id!), enabled: !!id })
  const [lang, setLang] = useState<'cpp'|'python'|'java'>('cpp')
  const [source, setSource] = useState<string>(DEFAULT_SNIPPETS.cpp)

  const submit = useMutation({
    mutationFn: () => postSubmission({ problemId: id!, lang, source }),
    onSuccess: (res) => nav(`/submissions/${res.submissionId}`)
  })

  if (isLoading || !problem) return <p>Cargando…</p>

  return (
    <div style={{display:'grid', gap:16}}>
      <h1 style={{fontSize:22}}>{problem.title}</h1>

      <section>
        <h3>Enunciado</h3>
        <p style={{whiteSpace:'pre-wrap'}}>{problem.statement}</p>
      </section>

      <section>
        <h3>Ejemplos</h3>
        <ul>
          {problem.examples.map((ex,i)=>(
            <li key={i}><code>in</code>: {JSON.stringify(ex.in)} → <code>out</code>: {JSON.stringify(ex.out)}</li>
          ))}
        </ul>
      </section>

      <div style={{display:'flex', gap:12, alignItems:'center'}}>
        <label>Lenguaje: </label>
        <select value={lang} onChange={e=>{ const L = e.target.value as 'cpp'|'python'|'java'; setLang(L); setSource(DEFAULT_SNIPPETS[L]) }}>
          <option value="cpp">C++</option>
          <option value="python">Python</option>
          <option value="java">Java</option>
        </select>
      </div>

      <CodeEditor language={lang} value={source} onChange={(v)=>setSource(v ?? '')} />

      <button onClick={()=>submit.mutate()} disabled={submit.isPending}
        style={{padding:'8px 12px', borderRadius:8, border:'1px solid #ddd'}}>
        {submit.isPending ? 'Enviando…' : 'Enviar'}
      </button>
    </div>
  )
}
