import { useParams, useNavigate } from 'react-router-dom'
import { useQuery, useMutation } from '@tanstack/react-query'
import { getProblem, submitSolution } from '../api/clients'
import { useState } from 'react'
import CodeEditor from '../components/CodeEditor'

type SubmissionCreated = { submissionId: string }

export default function ProblemDetailPage() {
  const { id } = useParams()
  const nav = useNavigate()
  const [lang] = useState<'cpp'>('cpp')
  const [source, setSource] = useState<string>(
`#include <vector>
#include <unordered_map>
using namespace std;

// Implementa la función pedida por el problema:
vector<int> twoSum(const vector<int>& nums, int target) {
  // tu código aquí
  return {};
}`
);




  const { data: problem, isLoading, error } = useQuery({
    queryKey: ['problem', id],
    queryFn: () => getProblem(id!),
    enabled: !!id,
  })

  const submit = useMutation<SubmissionCreated, Error, void>({
    mutationFn: async () => submitSolution({ problemId: id!, lang, source }),
    onSuccess: (res) => nav(`/submissions/${res.submissionId}`),
  })

  if (isLoading) return <p style={{ padding: 16 }}>Cargando…</p>
  if (error || !problem) return <p style={{ padding: 16 }}>No se pudo cargar el problema.</p>

  return (
    <div style={{ display: 'grid', gap: 16, padding: 16 }}>
      <button onClick={() => nav(-1)} style={{ width: 90 }}>← Volver</button>

      <h1 style={{ fontSize: 24 }}>{problem.title}</h1>
      <div style={{ color: '#666' }}>
        Dificultad: <b>{problem.difficulty}</b> · {problem.tags.join(', ')}
      </div>

      <section>
        <h3>Enunciado</h3>
        <p>{problem.statement}</p>
      </section>

      <section>
        <h3>Ejemplos</h3>
        <pre style={{ background: '#f6f6f6', padding: 12, borderRadius: 8 }}>
{JSON.stringify(problem.examples, null, 2)}
        </pre>
      </section>

      <section>
        <h3>Editor</h3>
        <CodeEditor
          language="cpp"
          value={source}
          onChange={(v) => setSource(v ?? '')}
          height="300px"
        />
      </section>

      <div>
        <button
          onClick={() => submit.mutate()}
          disabled={submit.isPending}
          style={{ padding: '8px 16px' }}
        >
          {submit.isPending ? 'Enviando…' : 'Enviar'}
        </button>
      </div>
    </div>
  )
}
