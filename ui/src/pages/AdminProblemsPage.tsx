import { useState } from 'react'
import { createProblem } from '../api/clients'
import type { CreateProblemReq } from '../types'

export default function AdminProblemsPage() {
  const [id, setId] = useState('')
  const [title, setTitle] = useState('')
  const [difficulty, setDifficulty] = useState<'easy' | 'medium' | 'hard'>('easy')
  const [tags, setTags] = useState('array, hash-table')
  const [statement, setStatement] = useState('')
  const [examplesRaw, setExamplesRaw] = useState(
    `[
  { "in": "nums = [2,7,11,15], target = 9", "out": "[0,1]" }
]`
  )
  const [starterCode, setStarterCode] = useState(
`class Solution {
public:
    // tu código aquí
};`
  )
  const [testsRaw, setTestsRaw] = useState(
    `[
  { "in": { "nums": [2,7,11,15], "target": 9 }, "out": [0,1] }
]`
  )

  const [status, setStatus] = useState<string | null>(null)
  const [error, setError] = useState<string | null>(null)
  const [isSubmitting, setIsSubmitting] = useState(false)

  async function handleSubmit(e: React.FormEvent) {
    e.preventDefault()
    setStatus(null)
    setError(null)

    // Validaciones básicas
    if (!id.trim() || !title.trim()) {
      setError('id y title son obligatorios')
      return
    }

    try {
      const examples = JSON.parse(examplesRaw)
      const tests = JSON.parse(testsRaw)

      const payload: CreateProblemReq = {
        id: id.trim(),
        title: title.trim(),
        difficulty,
        tags: tags.split(',').map(t => t.trim()).filter(Boolean),
        statement,
        examples,
        starterCode,
        tests,
      }

      setIsSubmitting(true)
      const res = await createProblem(payload)
      setStatus(`✅ Problema creado con id: ${res.id}`)
    } catch (err: any) {
      console.error(err)
      setError(`Error: ${err.message ?? 'JSON inválido en examples/tests'}`)
    } finally {
      setIsSubmitting(false)
    }
  }

  return (
    <div style={{ display: 'grid', gap: 16 }}>
      <h1>Admin · Crear problema</h1>

      <form onSubmit={handleSubmit} style={{ display: 'grid', gap: 12, maxWidth: 800 }}>
        <div style={{ display: 'grid', gap: 4 }}>
          <label>ID (único, ej. two-sum, reverse-string)</label>
          <input
            value={id}
            onChange={e => setId(e.target.value)}
            style={{ padding: 6 }}
          />
        </div>

        <div style={{ display: 'grid', gap: 4 }}>
          <label>Título</label>
          <input
            value={title}
            onChange={e => setTitle(e.target.value)}
            style={{ padding: 6 }}
          />
        </div>

        <div style={{ display: 'grid', gap: 4 }}>
          <label>Dificultad</label>
          <select
            value={difficulty}
            onChange={e => setDifficulty(e.target.value as any)}
            style={{ padding: 6 }}
          >
            <option value="easy">easy</option>
            <option value="medium">medium</option>
            <option value="hard">hard</option>
          </select>
        </div>

        <div style={{ display: 'grid', gap: 4 }}>
          <label>Tags (separadas por coma)</label>
          <input
            value={tags}
            onChange={e => setTags(e.target.value)}
            style={{ padding: 6 }}
          />
        </div>

        <div style={{ display: 'grid', gap: 4 }}>
          <label>Enunciado (statement)</label>
          <textarea
            value={statement}
            onChange={e => setStatement(e.target.value)}
            rows={4}
            style={{ padding: 6 }}
          />
        </div>

        <div style={{ display: 'grid', gap: 4 }}>
          <label>Ejemplos (JSON)</label>
          <textarea
            value={examplesRaw}
            onChange={e => setExamplesRaw(e.target.value)}
            rows={6}
            style={{ fontFamily: 'monospace', padding: 6 }}
          />
        </div>

        <div style={{ display: 'grid', gap: 4 }}>
          <label>Starter code (C++)</label>
          <textarea
            value={starterCode}
            onChange={e => setStarterCode(e.target.value)}
            rows={8}
            style={{ fontFamily: 'monospace', padding: 6 }}
          />
        </div>

        <div style={{ display: 'grid', gap: 4 }}>
          <label>Tests (JSON)</label>
          <textarea
            value={testsRaw}
            onChange={e => setTestsRaw(e.target.value)}
            rows={6}
            style={{ fontFamily: 'monospace', padding: 6 }}
          />
        </div>

        <button type="submit" disabled={isSubmitting} style={{ padding: '8px 16px' }}>
          {isSubmitting ? 'Guardando…' : 'Guardar problema'}
        </button>
      </form>

      {status && (
        <div style={{ color: 'green' }}>{status}</div>
      )}
      {error && (
        <div style={{ color: 'crimson', whiteSpace: 'pre-wrap' }}>{error}</div>
      )}

      <p style={{ fontSize: 12, color: '#666' }}>
        Tip: después de crear un problema nuevo, puedes ir a la página principal, recargar y
        debería aparecer en la lista.
      </p>
    </div>
  )
}
