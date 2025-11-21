import { useParams, useNavigate } from 'react-router-dom'
import { useQuery, useMutation } from '@tanstack/react-query'
import { getProblem, submitSolution } from '../api/clients'
import { useState, useEffect } from 'react'
import CodeEditor from '../components/CodeEditor'

type SubmissionCreated = { submissionId: string }

export default function ProblemDetailPage() {
  const { id } = useParams()
  const nav = useNavigate()
  const [lang] = useState<'cpp'>('cpp')

  // Fallback por si NO viene starterCode del backend
  const getDefaultCode = (problemId?: string) => {
    switch (problemId) {
      case 'two-sum':
        return `#include <vector>
#include <unordered_map>
using namespace std;

class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        // tu código aquí
        return {};
    }
};`
      case 'reverse-string':
        return `#include <vector>
using namespace std;

class Solution {
public:
    void reverseString(vector<char>& s) {
        // tu código aquí
    }
};`
      default:
        return `#include <iostream>
using namespace std;

int main() {
    // tu código aquí
    return 0;
}`
    }
  }

  const [source, setSource] = useState<string>('')      // se llena luego
  const [initialized, setInitialized] = useState(false) // para no pisar cambios del usuario

  const { data: problem, isLoading, error } = useQuery({
    queryKey: ['problem', id],
    queryFn: () => getProblem(id!),
    enabled: !!id,
  })

  // Cuando llega el problema desde el PM, inicializamos el editor
  useEffect(() => {
    if (!initialized && problem) {
      const backendStarter = (problem as any).starterCode as string | undefined
      const initial = backendStarter ?? getDefaultCode(id)
      setSource(initial)
      setInitialized(true)
    }
  }, [initialized, problem, id])

  const submit = useMutation<SubmissionCreated, Error, void>({
    mutationFn: async () => submitSolution({ problemId: id!, lang, source }),
    onSuccess: (res) => {
      const url = `/submissions/${res.submissionId}?problemId=${id}`
      console.log('🚀 NAVEGANDO A:', url)
      nav(url)
    },
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
        <p style={{ whiteSpace: 'pre-wrap' }}>{(problem as any).statement}</p>
      </section>

      <section>
        <h3>Ejemplos</h3>
        <pre style={{ background: '#f6f6f6', padding: 12, borderRadius: 8 }}>
          {JSON.stringify((problem as any).examples, null, 2)}
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
