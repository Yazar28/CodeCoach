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
  
  const getDefaultCode = (problemId?: string) => {
    switch(problemId) {
      case 'two-sum':
        return `#include <vector>
#include <unordered_map>
using namespace std;

vector<int> twoSum(const vector<int>& nums, int target) {
  // ❌ ESTE CÓDIGO FALLARÁ GARANTIZADO
  // El array [-999, -999] nunca será la respuesta correcta
  return {-999, -999};
}`;
      
      case 'reverse-string':
        return `#include <vector>
using namespace std;

void reverseString(vector<char>& s) {
    // ❌ ESTE CÓDIGO FALLARÁ GARANTIZADO  
    // Cambia el array a algo completamente incorrecto
    for (int i = 0; i < s.size(); i++) {
        s[i] = 'X';  // Convierte todo en 'X' - nunca será correcto
    }
}`;
      
      default:
        return `#include <iostream>
using namespace std;

int main() {
    // tu código aquí
    return 0;
}`;
    }
  };

  const [source, setSource] = useState<string>(getDefaultCode(id));

  const { data: problem, isLoading, error } = useQuery({
    queryKey: ['problem', id],
    queryFn: () => getProblem(id!),
    enabled: !!id,
  })

  const submit = useMutation<SubmissionCreated, Error, void>({
    mutationFn: async () => submitSolution({ problemId: id!, lang, source }),
    onSuccess: (res) => {
      const url = `/submissions/${res.submissionId}?problemId=${id}`;
      console.log('🚀 NAVEGANDO A:', url);
      nav(url);
    },
  });

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
        <p style={{ whiteSpace: 'pre-wrap' }}>{problem.statement}</p>
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